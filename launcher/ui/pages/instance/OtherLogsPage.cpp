#include "OtherLogsPage.h"

#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>

#include "Application.h"
#include "GZip.h"
#include "RecursiveFileSystemWatcher.h"
#include "settings/Setting.h"
#include "ui/GuiUtil.h"
#include "ui/dialogs/CustomMessageBox.h"

OtherLogsPage::OtherLogsPage(QString path, IPathMatcher::Ptr fileFilter,
                             QWidget *parent)
    : QWidget(parent),
      m_path(std::move(path)),
      m_watcher(new RecursiveFileSystemWatcher(this))
{
    m_watcher->setMatcher(std::move(fileFilter));
    m_watcher->setRootDir(QDir::current().absoluteFilePath(m_path));
    connect(m_watcher, &RecursiveFileSystemWatcher::filesChanged, this,
            &OtherLogsPage::refreshFiles);
    refreshFiles();

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("otherLogsPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The other logs page could not be loaded."), this);
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
        qCritical() << "Other logs page QML failed to load:"
                    << details.join('\n');
        errorLabel->setToolTip(details.join('\n'));
        stack->setCurrentWidget(errorLabel);
    };
    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget, showLoadError](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error ||
                    (status == QQuickWidget::Ready && !qmlWidget->rootObject())) {
                    showLoadError();
                }
            });
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/OtherLogsPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

OtherLogsPage::~OtherLogsPage() = default;

void OtherLogsPage::openedImpl()
{
    m_watcher->enable();
}

void OtherLogsPage::closedImpl()
{
    m_watcher->disable();
}

QStringList OtherLogsPage::files() const
{
    return m_watcher->files();
}

QString OtherLogsPage::absoluteFilePath(const QString &file) const
{
    return m_watcher->rootDir().absoluteFilePath(file);
}

void OtherLogsPage::setContent(const QString &content)
{
    if (m_content != content) {
        m_content = content;
    }
    emit contentChanged();
}

void OtherLogsPage::refreshFiles()
{
    const auto currentFiles = m_watcher->files();
    emit filesChanged();
    if (!m_currentFile.isEmpty() && !currentFiles.contains(m_currentFile)) {
        m_currentFile.clear();
        emit fileSelectionChanged();
        setContent(QString());
    }
}

void OtherLogsPage::selectFile(const QString &file)
{
    if (file.isEmpty()) {
        if (!m_currentFile.isEmpty()) {
            m_currentFile.clear();
            emit fileSelectionChanged();
            setContent(QString());
        }
        return;
    }
    if (!m_watcher->files().contains(file)) {
        qWarning() << "Ignoring selection of an unknown log file" << file;
        return;
    }
    if (m_currentFile != file) {
        m_currentFile = file;
        emit fileSelectionChanged();
    }
    reload();
}

void OtherLogsPage::reload()
{
    if (m_currentFile.isEmpty() ||
        !m_watcher->files().contains(m_currentFile)) {
        return;
    }
    const QString path = absoluteFilePath(m_currentFile);
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Unable to open %1 for reading: %2")
                .arg(m_currentFile, file.errorString()));
        return;
    }

    const QString fontFamily =
        APPLICATION->settings()->get("ConsoleFont").toString();
    bool conversionOk = false;
    int fontSize =
        APPLICATION->settings()->get("ConsoleFontSize").toInt(&conversionOk);
    if (!conversionOk) {
        fontSize = 11;
    }
    m_textFont = QFont(fontFamily, fontSize);

    if (file.size() > 12ll * 1024ll * 1024ll) {
        setContent(tr("The file (%1) is too big. You may want to open it in a "
                      "viewer optimized for large files.")
                       .arg(path));
        return;
    }

    QString loadedContent;
    if (path.endsWith(QStringLiteral(".gz"))) {
        QByteArray uncompressed;
        const QByteArray compressed = file.readAll();
        if (file.error() != QFileDevice::NoError) {
            QMessageBox::critical(
                this, tr("Error"),
                tr("Unable to read %1: %2")
                    .arg(m_currentFile, file.errorString()));
            return;
        }
        if (!GZip::unzip(compressed, uncompressed)) {
            setContent(tr("The file (%1) is not readable.").arg(path));
            return;
        }
        loadedContent = QString::fromUtf8(uncompressed);
    } else {
        const QByteArray bytes = file.readAll();
        if (file.error() != QFileDevice::NoError) {
            QMessageBox::critical(
                this, tr("Error"),
                tr("Unable to read %1: %2")
                    .arg(m_currentFile, file.errorString()));
            return;
        }
        loadedContent = QString::fromUtf8(bytes);
    }
    if (loadedContent.size() >= 50000000ll) {
        setContent(tr("The file (%1) is too big. You may want to open it in a "
                      "viewer optimized for large files.")
                       .arg(path));
        return;
    }
    setContent(loadedContent);
}

void OtherLogsPage::uploadLog()
{
    if (m_currentFile.isEmpty()) {
        return;
    }
    const auto response = CustomMessageBox::selectable(
                              this, tr("Log upload"),
                              tr("Are you sure you want to upload this log file?"),
                              QMessageBox::Warning,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
                              ->exec();
    if (response == QMessageBox::Yes) {
        GuiUtil::uploadPaste(m_content, this);
    }
}

void OtherLogsPage::copyLog()
{
    if (!m_currentFile.isEmpty()) {
        GuiUtil::setClipboardText(m_content);
    }
}

void OtherLogsPage::deleteLog()
{
    if (m_currentFile.isEmpty()) {
        return;
    }
    if (QMessageBox::question(
            this, tr("Delete"),
            tr("Do you really want to delete %1?").arg(m_currentFile),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) !=
        QMessageBox::Yes) {
        return;
    }

    QFile file(absoluteFilePath(m_currentFile));
    if (!file.remove()) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Unable to delete %1: %2")
                .arg(m_currentFile, file.errorString()));
        return;
    }
    m_currentFile.clear();
    emit fileSelectionChanged();
    setContent(QString());
    m_watcher->setRootDir(m_watcher->rootDir());
}

void OtherLogsPage::cleanLogs()
{
    const auto toDelete = m_watcher->files();
    if (toDelete.isEmpty()) {
        return;
    }

    QMessageBox messageBox(this);
    messageBox.setWindowTitle(tr("Clean up"));
    if (toDelete.size() > 5) {
        messageBox.setText(tr("Do you really want to delete all log files?"));
        messageBox.setDetailedText(toDelete.join('\n'));
    } else {
        messageBox.setText(
            tr("Do you really want to delete these files?\n%1")
                .arg(toDelete.join('\n')));
    }
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Cancel);
    messageBox.setTextInteractionFlags(Qt::TextBrowserInteraction);
    messageBox.setIcon(QMessageBox::Question);
    if (messageBox.exec() != QMessageBox::Ok) {
        return;
    }

    QStringList failed;
    for (const auto &item : toDelete) {
        QFile file(absoluteFilePath(item));
        if (!file.remove()) {
            failed.push_back(item);
        }
    }
    if (!failed.isEmpty()) {
        QMessageBox errorBox(this);
        errorBox.setWindowTitle(tr("Error"));
        if (failed.size() > 5) {
            errorBox.setText(tr("Couldn't delete some files!"));
            errorBox.setDetailedText(failed.join('\n'));
        } else {
            errorBox.setText(
                tr("Couldn't delete some files:\n%1").arg(failed.join('\n')));
        }
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.setTextInteractionFlags(Qt::TextBrowserInteraction);
        errorBox.exec();
    }
    if (!m_currentFile.isEmpty() && toDelete.contains(m_currentFile) &&
        !QFileInfo::exists(absoluteFilePath(m_currentFile))) {
        m_currentFile.clear();
        emit fileSelectionChanged();
        setContent(QString());
    }
    m_watcher->setRootDir(m_watcher->rootDir());
}

int OtherLogsPage::findInContent(const QString &query, int start,
                                 bool reverse) const
{
    if (query.isEmpty()) {
        return -1;
    }
    int found = reverse
                    ? m_content.lastIndexOf(query, start, Qt::CaseInsensitive)
                    : m_content.indexOf(query, start, Qt::CaseInsensitive);
    if (found < 0) {
        found = reverse
                    ? m_content.lastIndexOf(query, -1, Qt::CaseInsensitive)
                    : m_content.indexOf(query, 0, Qt::CaseInsensitive);
    }
    return found;
}
