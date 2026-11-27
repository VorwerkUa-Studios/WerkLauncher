/* Copyright 2015-2023 MultiMC Contributors
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

#include "WorldListPage.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QQmlError>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QInputDialog>
#include <QSet>
#include <algorithm>

#include "Application.h"
#include "FileSystem.h"
#include "DesktopServices.h"
#include "minecraft/PackProfile.h"
#include "minecraft/VersionFilterData.h"
#include "minecraft/WorldList.h"
#include "tools/MCEditTool.h"
#include "ui/GuiUtil.h"

WorldListPage::WorldListPage(InstancePtr inst, std::shared_ptr<WorldList> worlds,
                             QWidget *parent)
    : QWidget(parent), m_inst(std::move(inst)), m_worlds(std::move(worlds))
{
    const auto publishChanges = [this]() { refreshWorlds(); };
    connect(m_worlds.get(), &QAbstractItemModel::rowsInserted, this, publishChanges);
    connect(m_worlds.get(), &QAbstractItemModel::rowsRemoved, this, publishChanges);
    connect(m_worlds.get(), &QAbstractItemModel::dataChanged, this, publishChanges);
    connect(m_worlds.get(), &QAbstractItemModel::modelReset, this, publishChanges);

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("worldsPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel = new QLabel(tr("The worlds page could not be loaded."), this);
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
        qCritical() << "Worlds page QML failed to load:" << details.join('\n');
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/WorldListPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready &&
         !qmlWidget->rootObject())) {
        showLoadError();
    }

    refreshWorlds();
}

WorldListPage::~WorldListPage()
{
    m_worlds->stopWatching();
}

bool WorldListPage::shouldDisplay() const
{
    return true;
}

void WorldListPage::openedImpl()
{
    m_worlds->startWatching();
}

void WorldListPage::closedImpl()
{
    m_worlds->stopWatching();
}

QVariantMap WorldListPage::worldData(int row) const
{
    if (row < 0 || row >= m_worlds->rowCount()) {
        return {};
    }
    const auto &world = m_worlds->allWorlds().at(row);
    const QString folder = world.folderName();
    const QString iconFile = world.iconFile();
    QVariantMap result;
    result.insert(QStringLiteral("row"), row);
    result.insert(QStringLiteral("folderName"), folder);
    result.insert(QStringLiteral("name"), world.name());
    result.insert(QStringLiteral("gameMode"), world.gameType().toTranslatedString());
    result.insert(QStringLiteral("lastPlayed"),
                  QLocale().toString(world.lastPlayed(), QLocale::ShortFormat));
    result.insert(QStringLiteral("seed"), QVariant::fromValue<qlonglong>(world.seed()));
    result.insert(QStringLiteral("iconUrl"),
                  iconFile.isEmpty() ? QString() : QUrl::fromLocalFile(iconFile).toString());
    result.insert(QStringLiteral("hasIcon"), !iconFile.isEmpty());
    result.insert(QStringLiteral("selected"), folder == m_selectedWorldFolder);
    return result;
}

QVariantList WorldListPage::worlds() const
{
    QVariantList result;
    QList<int> rows;
    const QString filter = m_filterText.trimmed();
    for (int row = 0; row < m_worlds->rowCount(); ++row) {
        const auto &world = m_worlds->allWorlds().at(row);
        const QString searchable = world.name() + QLatin1Char(' ') +
                                   world.folderName() + QLatin1Char(' ') +
                                   world.gameType().toTranslatedString();
        if (!filter.isEmpty() &&
            !searchable.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }
        rows.append(row);
    }
    std::stable_sort(rows.begin(), rows.end(), [this](int leftRow, int rightRow) {
        const auto &left = m_worlds->allWorlds().at(leftRow);
        const auto &right = m_worlds->allWorlds().at(rightRow);
        int comparison = 0;
        if (m_sortMode == QStringLiteral("gameMode")) {
            comparison = QString::localeAwareCompare(
                left.gameType().toTranslatedString(),
                right.gameType().toTranslatedString());
        } else if (m_sortMode == QStringLiteral("lastPlayed")) {
            if (left.lastPlayed() < right.lastPlayed()) comparison = -1;
            else if (left.lastPlayed() > right.lastPlayed()) comparison = 1;
        } else {
            comparison = QString::localeAwareCompare(left.name(), right.name());
        }
        return m_sortDescending ? comparison > 0 : comparison < 0;
    });
    for (const int row : rows) {
        result.append(worldData(row));
    }
    return result;
}

void WorldListPage::setFilterText(const QString &text)
{
    if (m_filterText == text) {
        return;
    }
    m_filterText = text;
    emit worldsChanged();
}

void WorldListPage::setSortMode(const QString &mode)
{
    static const QSet<QString> supportedModes{
        QStringLiteral("name"), QStringLiteral("gameMode"),
        QStringLiteral("lastPlayed")};
    if (!supportedModes.contains(mode)) {
        qWarning() << "Ignoring unsupported world-list sort mode" << mode;
        return;
    }
    if (m_sortMode == mode) {
        return;
    }
    m_sortMode = mode;
    emit worldsChanged();
}

void WorldListPage::setSortDescending(bool descending)
{
    if (m_sortDescending == descending) {
        return;
    }
    m_sortDescending = descending;
    emit worldsChanged();
}

int WorldListPage::findWorldRow(const QString &folderName) const
{
    if (folderName.isEmpty()) {
        return -1;
    }
    const auto &allWorlds = m_worlds->allWorlds();
    for (int row = 0; row < allWorlds.size(); ++row) {
        if (allWorlds.at(row).folderName() == folderName) {
            return row;
        }
    }
    return -1;
}

QVariantMap WorldListPage::selectedWorld() const
{
    return worldData(findWorldRow(m_selectedWorldFolder));
}

bool WorldListPage::quickPlaySupported() const
{
    const auto minecraftInstance = std::dynamic_pointer_cast<MinecraftInstance>(m_inst);
    if (!minecraftInstance) {
        return false;
    }
    const auto profile = minecraftInstance->getPackProfile();
    const auto minecraftComponent = profile->getComponent("net.minecraft");
    return minecraftComponent &&
           minecraftComponent->getReleaseDateTime() >=
               g_VersionFilterData.quickPlayBeginsDate;
}

QVariantMap WorldListPage::actionStates() const
{
    const int row = findWorldRow(m_selectedWorldFolder);
    const bool hasSelection = row >= 0;
    const bool hasIcon = hasSelection &&
                         !m_worlds->allWorlds().at(row).iconFile().isEmpty();
    const bool quickPlay = quickPlaySupported();
    QVariantMap result;
    result.insert(QStringLiteral("add"), true);
    result.insert(QStringLiteral("join"), hasSelection && quickPlay);
    result.insert(QStringLiteral("joinOffline"), hasSelection && quickPlay);
    result.insert(QStringLiteral("quickPlayAvailable"), quickPlay);
    result.insert(QStringLiteral("rename"), hasSelection);
    result.insert(QStringLiteral("copy"), hasSelection);
    result.insert(QStringLiteral("remove"), hasSelection);
    result.insert(QStringLiteral("mcedit"), hasSelection);
    result.insert(QStringLiteral("datapacks"), hasSelection);
    result.insert(QStringLiteral("resetIcon"), hasSelection && hasIcon);
    result.insert(QStringLiteral("copySeed"), hasSelection);
    result.insert(QStringLiteral("refresh"), true);
    result.insert(QStringLiteral("folder"), true);
    return result;
}

void WorldListPage::selectWorld(const QString &folderName)
{
    if (!folderName.isEmpty() && findWorldRow(folderName) < 0) {
        qWarning() << "Ignoring selection of an unknown world" << folderName;
        return;
    }
    if (m_selectedWorldFolder == folderName) {
        return;
    }
    m_selectedWorldFolder = folderName;
    emit worldsChanged();
    emit selectedWorldChanged();
    emit actionStatesChanged();
}

void WorldListPage::refreshWorlds()
{
    if (findWorldRow(m_selectedWorldFolder) < 0) {
        m_selectedWorldFolder.clear();
    }
    emit worldsChanged();
    emit selectedWorldChanged();
    emit actionStatesChanged();
}

void WorldListPage::triggerAction(const QString &actionName)
{
    static const QSet<QString> supportedActions{
        QStringLiteral("add"),        QStringLiteral("join"),
        QStringLiteral("joinOffline"),QStringLiteral("rename"),
        QStringLiteral("copy"),       QStringLiteral("remove"),
        QStringLiteral("mcedit"),     QStringLiteral("datapacks"),
        QStringLiteral("resetIcon"),  QStringLiteral("copySeed"),
        QStringLiteral("refresh"),    QStringLiteral("folder")};
    if (!supportedActions.contains(actionName)) {
        qWarning() << "Ignoring unknown worlds-page action" << actionName;
        return;
    }
    if (!actionStates().value(actionName).toBool()) {
        return;
    }
    if (actionName == QStringLiteral("add")) addWorld();
    else if (actionName == QStringLiteral("join")) joinSelectedWorld(true);
    else if (actionName == QStringLiteral("joinOffline")) joinSelectedWorld(false);
    else if (actionName == QStringLiteral("rename")) renameWorld();
    else if (actionName == QStringLiteral("copy")) copyWorld();
    else if (actionName == QStringLiteral("remove")) removeWorld();
    else if (actionName == QStringLiteral("mcedit")) openMcedit();
    else if (actionName == QStringLiteral("datapacks")) openDatapacks();
    else if (actionName == QStringLiteral("resetIcon")) resetWorldIcon();
    else if (actionName == QStringLiteral("copySeed")) copySeed();
    else if (actionName == QStringLiteral("refresh")) refreshWorldList();
    else if (actionName == QStringLiteral("folder")) openWorldFolder();
}

bool WorldListPage::worldSafetyNagQuestion()
{
    if (!m_inst->isRunning()) {
        return true;
    }
    return QMessageBox::question(
               this, tr("Copy World"),
               tr("Changing a world while Minecraft is running is potentially unsafe.\n"
                  "Do you wish to proceed?")) == QMessageBox::Yes;
}

void WorldListPage::addWorld()
{
    const auto files = GuiUtil::BrowseForFiles(
        displayName(), tr("Select a Minecraft world zip"),
        tr("Minecraft World Zip File (*.zip)"), QString(), parentWidget());
    if (files.isEmpty()) {
        return;
    }
    m_worlds->stopWatching();
    for (const auto &filename : files) {
        m_worlds->installWorld(QFileInfo(filename));
    }
    m_worlds->startWatching();
}

void WorldListPage::copySeed()
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0) {
        return;
    }
    QApplication::clipboard()->setText(QString::number(
        m_worlds->allWorlds().at(row).seed()));
}

void WorldListPage::copyWorld()
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0 || !worldSafetyNagQuestion()) {
        return;
    }
    auto &world = (*m_worlds)[row];
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("World name"), tr("Enter a new name for the copy."),
        QLineEdit::Normal, world.name(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        world.install(m_worlds->dir().absolutePath(), name.trimmed());
    }
}

void WorldListPage::renameWorld()
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0 || !worldSafetyNagQuestion()) {
        return;
    }
    auto &world = (*m_worlds)[row];
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("World name"), tr("Enter a new world name."),
        QLineEdit::Normal, world.name(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        world.rename(name.trimmed());
    }
}

void WorldListPage::removeWorld()
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0) {
        return;
    }
    const auto result = QMessageBox::question(
        this, tr("Are you sure?"),
        tr("This will remove the selected world permanently.\n"
           "The world will be gone forever (A LONG TIME).\n\n"
           "Do you want to continue?"));
    if (result != QMessageBox::Yes) {
        return;
    }
    m_worlds->stopWatching();
    m_worlds->deleteWorld(row);
    m_worlds->startWatching();
}

void WorldListPage::openDatapacks()
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0 || !worldSafetyNagQuestion()) {
        return;
    }
    const QString folder = m_worlds->dir().absoluteFilePath(
        m_worlds->allWorlds().at(row).folderName());
    DesktopServices::openDirectory(FS::PathCombine(folder, "datapacks"));
}

void WorldListPage::resetWorldIcon()
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row >= 0 && m_worlds->resetIcon(row)) {
        refreshWorlds();
    }
}

void WorldListPage::openMcedit()
{
    if (m_mceditStarting) {
        return;
    }
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0 || !worldSafetyNagQuestion()) {
        return;
    }

    auto mcedit = APPLICATION->mcedit();
    const QString mceditPath = mcedit->path();
    const QString fullPath = m_worlds->dir().absoluteFilePath(
        m_worlds->allWorlds().at(row).folderName());
    const QString program = mcedit->getProgramPath();
    if (program.isEmpty()) {
        QMessageBox::warning(
            parentWidget(), tr("No MCEdit found or set up!"),
            tr("You do not have MCEdit set up or it was moved.\n"
               "You can set it up in the global settings."));
        return;
    }

#ifdef Q_OS_WIN32
    if (!QProcess::startDetached(program, {fullPath}, mceditPath)) {
        mceditError();
    }
#else
    m_mceditProcess.reset(new LoggedProcess());
    m_mceditProcess->setDetachable(true);
    connect(m_mceditProcess.get(), &LoggedProcess::stateChanged, this,
            &WorldListPage::mceditState);
    m_mceditProcess->start(program, {fullPath});
    m_mceditProcess->setWorkingDirectory(mceditPath);
    m_mceditStarting = true;
#endif
}

void WorldListPage::mceditError()
{
    QMessageBox::warning(parentWidget(), tr("MCEdit failed to start!"),
                         tr("MCEdit failed to start.\n"
                            "It may be necessary to reinstall it."));
}

void WorldListPage::mceditState(LoggedProcess::State state)
{
    switch (state) {
    case LoggedProcess::NotRunning:
    case LoggedProcess::Starting:
        return;
    case LoggedProcess::FailedToStart:
    case LoggedProcess::Crashed:
    case LoggedProcess::Aborted:
        m_mceditStarting = false;
        mceditError();
        return;
    case LoggedProcess::Running:
    case LoggedProcess::Finished:
        m_mceditStarting = false;
        return;
    }
}

void WorldListPage::joinSelectedWorld(bool online)
{
    const int row = findWorldRow(m_selectedWorldFolder);
    if (row < 0) {
        return;
    }
    const auto &world = m_worlds->allWorlds().at(row);
    APPLICATION->launch(
        m_inst, online, nullptr,
        std::make_shared<QuickPlayTarget>(
            QuickPlayTarget::parseSingleplayer(world.folderName())));
}

void WorldListPage::refreshWorldList()
{
    m_worlds->update();
}

void WorldListPage::openWorldFolder()
{
    DesktopServices::openDirectory(m_worlds->dir().absolutePath());
}
