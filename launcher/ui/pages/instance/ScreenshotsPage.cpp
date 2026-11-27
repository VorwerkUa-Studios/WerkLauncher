/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ScreenshotsPage.h"

#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QQmlError>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <algorithm>

#include "Application.h"
#include "DesktopServices.h"
#include "FileSystem.h"
#include "net/NetJob.h"
#include "screenshots/ImgurAlbumCreation.h"
#include "screenshots/ImgurUpload.h"
#include "tasks/SequentialTask.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ProgressDialog.h"

ScreenshotsPage::ScreenshotsPage(QString path, QWidget *parent)
    : QWidget(parent), m_folder(std::move(path))
{
    m_valid = FS::ensureFolderPathExists(m_folder);
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
            &ScreenshotsPage::directoryChanged);

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("screenshotsPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The screenshots page could not be loaded."), this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
    stack->addWidget(errorLabel);

    const auto showLoadError = [qmlWidget, stack, errorLabel]() {
        QStringList details;
        for (const auto &error : qmlWidget->errors()) {
            details.append(error.toString());
        }
        if (details.isEmpty()) {
            details.append(QObject::tr("No QML diagnostic was provided."));
        }
        qCritical() << "Screenshots page QML failed to load:"
                    << details.join('\n');
        errorLabel->setToolTip(details.join('\n'));
        stack->setCurrentWidget(errorLabel);
    };
    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget, showLoadError](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error ||
                    (status == QQuickWidget::Ready &&
                     !qmlWidget->rootObject())) {
                    showLoadError();
                }
            });
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/ScreenshotsPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready &&
         !qmlWidget->rootObject())) {
        showLoadError();
    }

    refreshScreenshots();
}

ScreenshotsPage::~ScreenshotsPage()
{
    if (m_watcher->directories().contains(QDir(m_folder).absolutePath())) {
        m_watcher->removePath(QDir(m_folder).absolutePath());
    }
}

void ScreenshotsPage::openedImpl()
{
    if (!m_valid) {
        m_valid = FS::ensureFolderPathExists(m_folder);
    }
    if (!m_valid) {
        qWarning() << "Screenshot folder is not accessible:" << m_folder;
        return;
    }
    const QString path = QDir(m_folder).absolutePath();
    if (!m_watcher->directories().contains(path) && !m_watcher->addPath(path)) {
        qWarning() << "Failed to watch screenshot folder:" << path;
    }
    refreshScreenshots();
}

void ScreenshotsPage::closedImpl()
{
    const QString path = QDir(m_folder).absolutePath();
    if (m_watcher->directories().contains(path)) {
        m_watcher->removePath(path);
    }
}

QVariantList ScreenshotsPage::screenshots() const
{
    QVariantList result;
    QDir directory(m_folder);
    const auto files = directory.entryInfoList(
        {QStringLiteral("*.png")}, QDir::Files | QDir::Readable | QDir::Writable,
        QDir::Name | QDir::IgnoreCase);
    for (const auto &info : files) {
        QVariantMap item;
        const QString path = info.absoluteFilePath();
        item.insert(QStringLiteral("path"), path);
        item.insert(QStringLiteral("name"), info.completeBaseName());
        item.insert(QStringLiteral("imageUrl"), QUrl::fromLocalFile(path).toString());
        item.insert(QStringLiteral("modified"),
                    info.lastModified().toMSecsSinceEpoch());
        item.insert(QStringLiteral("size"), info.size());
        item.insert(QStringLiteral("selected"), m_selectedPaths.contains(path));
        result.append(item);
    }
    return result;
}

QVariantMap ScreenshotsPage::actionStates() const
{
    const bool hasSelection = !m_selectedPaths.isEmpty();
    QVariantMap result;
    result.insert(QStringLiteral("upload"), hasSelection && !m_uploadActive);
    result.insert(QStringLiteral("copyImage"), hasSelection);
    result.insert(QStringLiteral("copyFiles"), hasSelection);
    result.insert(QStringLiteral("delete"), hasSelection && !m_uploadActive);
    result.insert(QStringLiteral("rename"), hasSelection && !m_uploadActive);
    result.insert(QStringLiteral("folder"), true);
    return result;
}

void ScreenshotsPage::refreshScreenshots()
{
    QSet<QString> existing;
    for (const auto &item : screenshots()) {
        existing.insert(item.toMap().value(QStringLiteral("path")).toString());
    }
    for (auto iter = m_selectedPaths.begin(); iter != m_selectedPaths.end();) {
        if (!existing.contains(*iter)) {
            iter = m_selectedPaths.erase(iter);
        } else {
            ++iter;
        }
    }
    if (!existing.contains(m_currentPath)) {
        m_currentPath = m_selectedPaths.isEmpty()
                            ? QString()
                            : *m_selectedPaths.constBegin();
    }
    if (!existing.contains(m_selectionAnchorPath)) {
        m_selectionAnchorPath = m_currentPath;
    }
    emit screenshotsChanged();
    emit selectionChanged();
    emit actionStatesChanged();
}

void ScreenshotsPage::directoryChanged(const QString &)
{
    refreshScreenshots();
}

void ScreenshotsPage::selectScreenshot(const QString &path, int modifiers)
{
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    if (!QFileInfo::exists(absolutePath) ||
        QFileInfo(absolutePath).absolutePath() != QDir(m_folder).absolutePath()) {
        qWarning() << "Ignoring selection of a screenshot outside its folder"
                   << path;
        return;
    }

    QStringList visiblePaths;
    for (const auto &item : screenshots()) {
        visiblePaths.append(item.toMap().value(QStringLiteral("path")).toString());
    }
    const bool controlHeld = modifiers & Qt::ControlModifier;
    const bool shiftHeld = modifiers & Qt::ShiftModifier;
    const int anchor = visiblePaths.indexOf(m_selectionAnchorPath);
    const int current = visiblePaths.indexOf(absolutePath);
    if (shiftHeld && anchor >= 0 && current >= 0) {
        if (!controlHeld) {
            m_selectedPaths.clear();
        }
        const int first = std::min(anchor, current);
        const int last = std::max(anchor, current);
        for (int i = first; i <= last; ++i) {
            m_selectedPaths.insert(visiblePaths.at(i));
        }
    } else if (controlHeld) {
        if (m_selectedPaths.contains(absolutePath)) {
            m_selectedPaths.remove(absolutePath);
        } else {
            m_selectedPaths.insert(absolutePath);
        }
        m_selectionAnchorPath = absolutePath;
    } else {
        m_selectedPaths = {absolutePath};
        m_selectionAnchorPath = absolutePath;
    }
    m_currentPath = absolutePath;
    emit screenshotsChanged();
    emit selectionChanged();
    emit actionStatesChanged();
}

void ScreenshotsPage::openScreenshot(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || info.absolutePath() != QDir(m_folder).absolutePath()) {
        qWarning() << "Ignoring request to open an invalid screenshot" << path;
        return;
    }
    DesktopServices::openFile(info.absoluteFilePath());
}

void ScreenshotsPage::triggerAction(const QString &actionName)
{
    static const QSet<QString> supportedActions{
        QStringLiteral("upload"), QStringLiteral("copyImage"),
        QStringLiteral("copyFiles"), QStringLiteral("delete"),
        QStringLiteral("rename"), QStringLiteral("folder")};
    if (!supportedActions.contains(actionName)) {
        qWarning() << "Ignoring unknown screenshots-page action" << actionName;
        return;
    }
    if (!actionStates().value(actionName).toBool()) {
        return;
    }
    if (actionName == QStringLiteral("upload")) uploadSelected();
    else if (actionName == QStringLiteral("copyImage")) copyImage();
    else if (actionName == QStringLiteral("copyFiles")) copyFiles();
    else if (actionName == QStringLiteral("delete")) deleteSelected();
    else if (actionName == QStringLiteral("rename")) renameSelected();
    else if (actionName == QStringLiteral("folder")) openFolder();
}

void ScreenshotsPage::uploadSelected()
{
    QStringList paths = m_selectedPaths.values();
    std::sort(paths.begin(), paths.end());
    if (paths.isEmpty()) {
        return;
    }
    const QString uploadText = paths.size() == 1
        ? tr("Upload screenshot to imgur.com?")
        : tr("Upload %1 screenshots to imgur.com?").arg(paths.size());
    const auto response = CustomMessageBox::selectable(
        this, tr("Upload?"), uploadText, QMessageBox::Question,
        QMessageBox::Yes | QMessageBox::No)->exec();
    if (response != QMessageBox::Yes) {
        return;
    }

    auto copyLinkToClipboard = [](const QString &link) {
        QApplication::clipboard()->setText(link);
    };

    if (paths.size() == 1) {
        auto screenshot = std::make_shared<ScreenShot>(QFileInfo(paths.first()));
        auto job = NetJob::Ptr(
            new NetJob("Screenshot Upload", APPLICATION->network()));
        job->addNetAction(ImgurUpload::make(screenshot));

        m_uploadActive = true;
        emit actionStatesChanged();
        ProgressDialog dialog(this);
        const auto result = dialog.execWithTask(job.get());
        m_uploadActive = false;
        emit actionStatesChanged();
        if (result != QDialog::Accepted) {
            CustomMessageBox::selectable(
                this, tr("Failed to upload screenshots!"), tr("Unknown error"),
                QMessageBox::Warning)->exec();
            return;
        }

        const QString link = screenshot->m_url;
        copyLinkToClipboard(link);
        CustomMessageBox::selectable(
            this, tr("Upload finished"),
            tr("The <a href=\"%1\">link to the uploaded screenshot</a> has "
               "been placed in your clipboard.").arg(link),
            QMessageBox::Information)->exec();
        return;
    }

    QList<ScreenShot::Ptr> uploaded;
    auto job = NetJob::Ptr(
        new NetJob("Screenshot Upload", APPLICATION->network()));
    for (const auto &path : paths) {
        auto screenshot = std::make_shared<ScreenShot>(QFileInfo(path));
        uploaded.append(screenshot);
        job->addNetAction(ImgurUpload::make(screenshot));
    }
    SequentialTask task;
    auto albumJob =
        NetJob::Ptr(new NetJob("Imgur Album Creation", APPLICATION->network()));
    auto album = ImgurAlbumCreation::make(uploaded);
    albumJob->addNetAction(album);
    task.addTask(job);
    task.addTask(albumJob);

    m_uploadActive = true;
    emit actionStatesChanged();
    ProgressDialog progress(this);
    const auto result = progress.execWithTask(&task);
    m_uploadActive = false;
    emit actionStatesChanged();
    if (result != QDialog::Accepted) {
        CustomMessageBox::selectable(
            this, tr("Failed to upload screenshots!"), tr("Unknown error"),
            QMessageBox::Warning)->exec();
        return;
    }
    const QString link = QStringLiteral("https://imgur.com/a/%1").arg(album->id());
    copyLinkToClipboard(link);
    CustomMessageBox::selectable(
        this, tr("Upload finished"),
        tr("The <a href=\"%1\">link to the uploaded album</a> has been placed "
           "in your clipboard.").arg(link),
        QMessageBox::Information)->exec();
}

void ScreenshotsPage::copyImage()
{
    if (m_selectedPaths.isEmpty()) {
        return;
    }
    QString path = m_currentPath;
    if (!m_selectedPaths.contains(path)) {
        QStringList paths = m_selectedPaths.values();
        std::sort(paths.begin(), paths.end());
        path = paths.first();
    }
    QImage image(path);
    if (image.isNull()) {
        qWarning() << "Could not read screenshot image for clipboard" << path;
        return;
    }
    QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
}

void ScreenshotsPage::copyFiles()
{
    if (m_selectedPaths.isEmpty()) {
        return;
    }
    QStringList paths = m_selectedPaths.values();
    std::sort(paths.begin(), paths.end());
    QList<QUrl> urls;
    for (const auto &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    auto *mimeData = new QMimeData();
    mimeData->setUrls(urls);
    QApplication::clipboard()->setMimeData(mimeData);
}

void ScreenshotsPage::deleteSelected()
{
    if (m_selectedPaths.isEmpty()) {
        return;
    }
    const auto answer = CustomMessageBox::selectable(
        this, tr("Are you sure?"),
        tr("This will delete all selected screenshots."),
        QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No)->exec();
    if (answer != QMessageBox::Yes) {
        return;
    }
    for (const auto &path : m_selectedPaths) {
        if (!QFile::remove(path)) {
            qWarning() << "Failed to delete screenshot" << path;
        }
    }
    refreshScreenshots();
}

void ScreenshotsPage::renameSelected()
{
    if (m_selectedPaths.isEmpty()) {
        return;
    }
    QString path = m_currentPath;
    if (!m_selectedPaths.contains(path)) {
        QStringList paths = m_selectedPaths.values();
        std::sort(paths.begin(), paths.end());
        path = paths.first();
    }
    const QFileInfo info(path);
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("Rename screenshot"), tr("Enter a new screenshot name."),
        QLineEdit::Normal, info.completeBaseName(), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    QString baseName = QFileInfo(name.trimmed()).fileName();
    if (baseName != name.trimmed() || baseName == QStringLiteral(".") ||
        baseName == QStringLiteral("..")) {
        QMessageBox::warning(this, tr("Invalid name"),
                             tr("Enter a file name without a folder path."));
        return;
    }
    if (baseName.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)) {
        baseName.chop(4);
    }
    if (baseName.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid name"),
                             tr("Enter a valid screenshot name."));
        return;
    }
    const QString target = QDir(m_folder).filePath(baseName + QStringLiteral(".png"));
    if (target == path) {
        return;
    }
    if (QFileInfo::exists(target) || !QFile::rename(path, target)) {
        QMessageBox::warning(this, tr("Rename failed"),
                             tr("The screenshot could not be renamed."));
        return;
    }
    m_selectedPaths.remove(path);
    m_selectedPaths.insert(target);
    m_currentPath = target;
    m_selectionAnchorPath = target;
    refreshScreenshots();
}

void ScreenshotsPage::openFolder()
{
    DesktopServices::openDirectory(m_folder);
}
