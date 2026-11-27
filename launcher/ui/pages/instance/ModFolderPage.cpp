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

#include "ModFolderPage.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QQmlError>
#include <QQuickWidget>
#include <QRegularExpression>
#include <QStackedLayout>
#include <QTimer>
#include <QUrl>
#include <algorithm>
#include <utility>

#include "Application.h"
#include "DesktopServices.h"
#include "Version.h"
#include "minecraft/PackProfile.h"
#include "minecraft/mod/Mod.h"
#include "minecraft/mod/ModFolderModel.h"
#include "minecraft/VersionFilterData.h"
#include "ui/GuiUtil.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ModDownloaderDialog.h"
#include "ui/dialogs/ProgressDialog.h"
#include "ui/pages/modplatform/unified/UnifiedModData.h"
#include "minecraft/mod/ModUpdateInstallTask.h"

namespace {
QString sortName(const Mod &mod)
{
    QString name = mod.name().isEmpty() ? mod.mmc_id() : mod.name();
    static const QRegularExpression articlePrefix(
        QStringLiteral("^(the|teh) +"), QRegularExpression::CaseInsensitiveOption);
    name.remove(articlePrefix);
    return name.trimmed();
}

QString modVersionText(const Mod &mod)
{
    if (mod.type() == Mod::MOD_FOLDER) {
        return QObject::tr("Folder");
    }
    if (mod.type() == Mod::MOD_SINGLEFILE) {
        return QObject::tr("File");
    }
    return mod.version();
}
}

ModFolderPage::ModFolderPage(BaseInstance *inst,
                             std::shared_ptr<ModFolderModel> mods, QString id,
                             QString iconName, QString displayName,
                             QString helpPage, QWidget *parent)
    : QWidget(parent),
      m_inst(inst),
      m_mods(std::move(mods)),
      m_iconName(std::move(iconName)),
      m_id(std::move(id)),
      m_displayName(std::move(displayName)),
      m_helpName(std::move(helpPage))
{
    m_controlsEnabled = m_inst && !m_inst->isRunning();
    m_mods->setInstance(m_inst);

    const auto publishChanges = [this]() { refreshAfterModelChange(); };
    connect(m_mods.get(), &QAbstractItemModel::rowsInserted, this, publishChanges);
    connect(m_mods.get(), &QAbstractItemModel::rowsRemoved, this, publishChanges);
    connect(m_mods.get(), &QAbstractItemModel::dataChanged, this, publishChanges);
    connect(m_mods.get(), &QAbstractItemModel::modelReset, this, publishChanges);
    connect(m_mods.get(), &QAbstractItemModel::layoutChanged, this, publishChanges);
    if (m_inst) {
        connect(m_inst, &BaseInstance::runningStatusChanged, this,
                &ModFolderPage::onRunningStatusChanged);
    }

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(m_id + QStringLiteral("PageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel = new QLabel(tr("The mods and packs page could not be loaded."), this);
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
        qCritical() << "Mods and packs page QML failed to load:"
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/ModFolderPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready &&
         !qmlWidget->rootObject())) {
        showLoadError();
    }
}

ModFolderPage::~ModFolderPage()
{
    m_mods->stopWatching();
}

QIcon ModFolderPage::icon() const
{
    return APPLICATION->getThemedIcon(m_iconName);
}

bool ModFolderPage::shouldDisplay() const
{
    return true;
}

void ModFolderPage::setShowConfigFolder(bool show)
{
    if (m_showConfigFolder == show) {
        return;
    }
    m_showConfigFolder = show;
    emit actionStatesChanged();
}

void ModFolderPage::openedImpl()
{
    m_mods->startWatching();
    QTimer::singleShot(2000, this, [this]() { m_mods->checkModUpdates(); });
}

void ModFolderPage::closedImpl()
{
    m_mods->stopWatching();
}

QVariantMap ModFolderPage::modData(int sourceRow) const
{
    if (sourceRow < 0 || sourceRow >= m_mods->rowCount(QModelIndex())) {
        return {};
    }

    const auto &mod = m_mods->at(static_cast<size_t>(sourceRow));
    const QString modId = mod.mmc_id();
    QVariantMap result;
    result.insert(QStringLiteral("sourceRow"), sourceRow);
    result.insert(QStringLiteral("id"), modId);
    result.insert(QStringLiteral("name"),
                  mod.name().isEmpty() ? modId : mod.name());
    result.insert(QStringLiteral("version"), modVersionText(mod));
    result.insert(QStringLiteral("authors"), mod.authors().join(QStringLiteral(", ")));
    result.insert(QStringLiteral("description"), mod.description());
    result.insert(QStringLiteral("homepage"), mod.homeurl());
    result.insert(QStringLiteral("enabled"), mod.enabled());
    result.insert(QStringLiteral("selected"), m_selectedModIds.contains(modId));
    result.insert(QStringLiteral("hasUpdate"), mod.hasUpdate());
    result.insert(QStringLiteral("latestVersion"), mod.latestVersion());
    result.insert(QStringLiteral("fileName"), mod.filename().fileName());
    result.insert(QStringLiteral("modified"),
                  QLocale().toString(mod.dateTimeChanged(), QLocale::ShortFormat));
    return result;
}

QList<int> ModFolderPage::visibleRows() const
{
    QList<int> rows;
    const QString filter = m_filterText.trimmed();
    for (int row = 0; row < m_mods->rowCount(QModelIndex()); ++row) {
        const auto &mod = m_mods->at(static_cast<size_t>(row));
        if (!filter.isEmpty()) {
            QStringList searchable{mod.name(), mod.description()};
            searchable.append(mod.authors());
            bool matches = false;
            for (const auto &field : searchable) {
                if (field.contains(filter, Qt::CaseInsensitive)) {
                    matches = true;
                    break;
                }
            }
            if (!matches) {
                continue;
            }
        }
        rows.append(row);
    }

    std::stable_sort(rows.begin(), rows.end(), [this](int leftRow, int rightRow) {
        const auto &left = m_mods->at(static_cast<size_t>(leftRow));
        const auto &right = m_mods->at(static_cast<size_t>(rightRow));
        if (left.hasUpdate() != right.hasUpdate()) {
            return left.hasUpdate();
        }

        int comparison = 0;
        if (m_sortMode == QStringLiteral("version")) {
            const Version leftVersion(left.version());
            const Version rightVersion(right.version());
            if (leftVersion < rightVersion) {
                comparison = -1;
            } else if (leftVersion > rightVersion) {
                comparison = 1;
            }
        } else if (m_sortMode == QStringLiteral("modified")) {
            if (left.dateTimeChanged() < right.dateTimeChanged()) {
                comparison = -1;
            } else if (left.dateTimeChanged() > right.dateTimeChanged()) {
                comparison = 1;
            }
        } else if (m_sortMode == QStringLiteral("enabled")) {
            if (left.enabled() != right.enabled()) {
                comparison = left.enabled() ? -1 : 1;
            }
        } else {
            const int nameComparison =
                QString::localeAwareCompare(sortName(left), sortName(right));
            comparison = (nameComparison > 0) - (nameComparison < 0);
        }

        if (comparison == 0) {
            const int nameComparison =
                QString::localeAwareCompare(sortName(left), sortName(right));
            comparison = (nameComparison > 0) - (nameComparison < 0);
        }
        return m_sortDescending ? comparison > 0 : comparison < 0;
    });
    return rows;
}

QVariantList ModFolderPage::items() const
{
    QVariantList result;
    for (const int row : visibleRows()) {
        result.append(modData(row));
    }
    return result;
}

QVariantMap ModFolderPage::selectedMod() const
{
    const int row = findModRow(m_currentModId);
    return modData(row);
}

QVariantMap ModFolderPage::actionStates() const
{
    const bool hasSelection = !m_selectedModIds.isEmpty();
    QVariantMap result;
    result.insert(QStringLiteral("add"), m_controlsEnabled);
    result.insert(QStringLiteral("toggle"), m_controlsEnabled);
    result.insert(QStringLiteral("download"), true);
    result.insert(QStringLiteral("updateAll"), true);
    result.insert(QStringLiteral("remove"), m_controlsEnabled && hasSelection);
    result.insert(QStringLiteral("enable"), m_controlsEnabled && hasSelection);
    result.insert(QStringLiteral("disable"), m_controlsEnabled && hasSelection);
    result.insert(QStringLiteral("configs"), m_showConfigFolder);
    result.insert(QStringLiteral("folder"), true);
    return result;
}

void ModFolderPage::setFilterText(const QString &text)
{
    if (m_filterText == text) {
        return;
    }
    m_filterText = text;
    emit itemsChanged();
}

void ModFolderPage::setSortMode(const QString &mode)
{
    static const QSet<QString> supportedModes{
        QStringLiteral("name"), QStringLiteral("version"),
        QStringLiteral("modified"), QStringLiteral("enabled")};
    if (!supportedModes.contains(mode)) {
        qWarning() << "Ignoring unsupported mod-list sort mode" << mode;
        return;
    }
    if (m_sortMode == mode) {
        return;
    }
    m_sortMode = mode;
    emit itemsChanged();
}

void ModFolderPage::setSortDescending(bool descending)
{
    if (m_sortDescending == descending) {
        return;
    }
    m_sortDescending = descending;
    emit itemsChanged();
}

int ModFolderPage::findModRow(const QString &modId) const
{
    if (modId.isEmpty()) {
        return -1;
    }
    for (int row = 0; row < m_mods->rowCount(QModelIndex()); ++row) {
        if (m_mods->at(static_cast<size_t>(row)).mmc_id() == modId) {
            return row;
        }
    }
    return -1;
}

QList<int> ModFolderPage::selectedRows() const
{
    QList<int> rows;
    for (const auto &modId : m_selectedModIds) {
        const int row = findModRow(modId);
        if (row >= 0) {
            rows.append(row);
        }
    }
    std::sort(rows.begin(), rows.end());
    return rows;
}

void ModFolderPage::selectMod(const QString &modId, int modifiers)
{
    const int selectedRow = findModRow(modId);
    if (selectedRow < 0) {
        qWarning() << "Ignoring selection of an unknown mod" << modId;
        return;
    }

    const bool controlHeld = modifiers & Qt::ControlModifier;
    const bool shiftHeld = modifiers & Qt::ShiftModifier;
    const QList<int> rows = visibleRows();
    const int anchorRow = findModRow(m_selectionAnchorId);
    const int visibleAnchor = rows.indexOf(anchorRow);
    const int visibleCurrent = rows.indexOf(selectedRow);

    if (shiftHeld && visibleAnchor >= 0 && visibleCurrent >= 0) {
        if (!controlHeld) {
            m_selectedModIds.clear();
        }
        const int first = std::min(visibleAnchor, visibleCurrent);
        const int last = std::max(visibleAnchor, visibleCurrent);
        for (int i = first; i <= last; ++i) {
            m_selectedModIds.insert(
                m_mods->at(static_cast<size_t>(rows.at(i))).mmc_id());
        }
    } else if (controlHeld) {
        if (m_selectedModIds.contains(modId)) {
            m_selectedModIds.remove(modId);
        } else {
            m_selectedModIds.insert(modId);
        }
        m_selectionAnchorId = modId;
    } else {
        m_selectedModIds = {modId};
        m_selectionAnchorId = modId;
    }

    m_currentModId = modId;
    emit itemsChanged();
    emit selectionChanged();
    emit actionStatesChanged();
}

void ModFolderPage::reconcileSelection()
{
    for (auto iter = m_selectedModIds.begin(); iter != m_selectedModIds.end();) {
        if (findModRow(*iter) < 0) {
            iter = m_selectedModIds.erase(iter);
        } else {
            ++iter;
        }
    }
    if (findModRow(m_currentModId) < 0) {
        m_currentModId = m_selectedModIds.isEmpty()
                             ? QString()
                             : *m_selectedModIds.constBegin();
    }
    if (findModRow(m_selectionAnchorId) < 0) {
        m_selectionAnchorId = m_currentModId;
    }
}

void ModFolderPage::refreshAfterModelChange()
{
    if (m_suppressModelUpdates) {
        return;
    }
    reconcileSelection();
    emit itemsChanged();
    emit selectionChanged();
    emit actionStatesChanged();
}

void ModFolderPage::applyModStatus(const QList<int> &rows, int action)
{
    if (!m_controlsEnabled || rows.isEmpty()) {
        return;
    }

    const QList<int> previousSelection = selectedRows();
    const int previousCurrentRow = findModRow(m_currentModId);
    const int previousAnchorRow = findModRow(m_selectionAnchorId);
    QModelIndexList indexes;
    for (const int row : rows) {
        if (row >= 0 && row < m_mods->rowCount(QModelIndex())) {
            indexes.append(m_mods->index(row, ModFolderModel::ActiveColumn));
        }
    }

    m_suppressModelUpdates = true;
    m_mods->setModStatus(
        indexes, static_cast<ModFolderModel::ModStatusAction>(action));
    m_suppressModelUpdates = false;

    m_selectedModIds.clear();
    for (const int row : previousSelection) {
        if (row >= 0 && row < m_mods->rowCount(QModelIndex())) {
            m_selectedModIds.insert(
                m_mods->at(static_cast<size_t>(row)).mmc_id());
        }
    }
    if (previousCurrentRow >= 0 &&
        previousCurrentRow < m_mods->rowCount(QModelIndex())) {
        m_currentModId =
            m_mods->at(static_cast<size_t>(previousCurrentRow)).mmc_id();
    }
    if (previousAnchorRow >= 0 &&
        previousAnchorRow < m_mods->rowCount(QModelIndex())) {
        m_selectionAnchorId =
            m_mods->at(static_cast<size_t>(previousAnchorRow)).mmc_id();
    }
    refreshAfterModelChange();
}

void ModFolderPage::setModEnabled(int sourceRow, bool enabled)
{
    if (sourceRow < 0 || sourceRow >= m_mods->rowCount(QModelIndex()) ||
        !m_controlsEnabled) {
        return;
    }
    const QString modId =
        m_mods->at(static_cast<size_t>(sourceRow)).mmc_id();
    if (!m_selectedModIds.contains(modId)) {
        selectMod(modId, Qt::NoModifier);
    }
    const auto action = enabled ? ModFolderModel::Enable : ModFolderModel::Disable;
    applyModStatus({sourceRow}, action);
}

void ModFolderPage::installUpdate(int sourceRow)
{
    if (sourceRow < 0 || sourceRow >= m_mods->rowCount(QModelIndex())) {
        qWarning() << "Ignoring update request for invalid mod row" << sourceRow;
        return;
    }
    const Mod mod = m_mods->at(static_cast<size_t>(sourceRow));
    if (!mod.hasUpdate()) {
        qWarning() << "Ignoring update request for mod without an update"
                   << mod.mmc_id();
        return;
    }
    QString itemType = tr("елемент");
    QString itemTypeCap = tr("Елемент");
    if (m_id == QStringLiteral("mods")) {
        itemType = tr("мод");
        itemTypeCap = tr("Мод");
    } else if (m_id == QStringLiteral("resourcepacks") ||
               m_id == QStringLiteral("texturepacks")) {
        itemType = tr("ресурспак");
        itemTypeCap = tr("Ресурспак");
    } else if (m_id == QStringLiteral("shaderpacks")) {
        itemType = tr("шейдер");
        itemTypeCap = tr("Шейдер");
    }

    if (mod.updateUrl().isEmpty()) {
        CustomMessageBox::information(
            this, tr("Оновлення відсутнє"),
            tr("На жаль, посилання на завантаження відсутнє для '%1'.")
                .arg(mod.name()));
        return;
    }

    auto *task = new ModUpdateInstallTask(m_inst, m_mods.get(), {mod}, this);
    ProgressDialog progressDialog(this);
    if (progressDialog.execWithTask(task) == QDialog::Accepted) {
        CustomMessageBox::information(
            this, tr("Оновлення завершено"),
            tr("%1 '%2' успішно оновлено!").arg(itemTypeCap, mod.name()));
    } else {
        CustomMessageBox::information(
            this, tr("Помилка"),
            tr("Не вдалося оновити %1 '%2':\n%3")
                .arg(itemType, mod.name(), task->failReason()));
    }
}

void ModFolderPage::installDroppedFiles(const QVariantList &urls)
{
    if (!m_controlsEnabled) {
        return;
    }
    for (const auto &value : urls) {
        QUrl url = value.toUrl();
        if (!url.isValid() && !value.toString().isEmpty()) {
            url = QUrl::fromUserInput(value.toString());
        }
        if (!url.isLocalFile()) {
            qWarning() << "Ignoring non-local file dropped onto mod page" << url;
            continue;
        }
        m_mods->installMod(url.toLocalFile());
    }
}

void ModFolderPage::openModHomepage()
{
    const QString homepage = selectedMod().value(QStringLiteral("homepage")).toString();
    const QUrl url(homepage);
    if ((url.scheme() != QStringLiteral("http") &&
         url.scheme() != QStringLiteral("https")) ||
        url.host().isEmpty()) {
        qWarning() << "Ignoring invalid mod homepage URL" << homepage;
        return;
    }
    if (!QDesktopServices::openUrl(url)) {
        qWarning() << "Failed to open mod homepage" << url;
    }
}

void ModFolderPage::triggerAction(const QString &actionName)
{
    static const QSet<QString> supportedActions{
        QStringLiteral("add"),      QStringLiteral("download"),
        QStringLiteral("updateAll"), QStringLiteral("remove"),
        QStringLiteral("enable"),   QStringLiteral("disable"),
        QStringLiteral("configs"),  QStringLiteral("folder")};
    if (!supportedActions.contains(actionName)) {
        qWarning() << "Ignoring unknown mods-page action" << actionName;
        return;
    }
    const QVariantMap states = actionStates();
    if (!states.value(actionName).toBool()) {
        return;
    }

    if (actionName == QStringLiteral("add")) addFiles();
    else if (actionName == QStringLiteral("download")) openDownloader();
    else if (actionName == QStringLiteral("updateAll")) updateAll();
    else if (actionName == QStringLiteral("remove")) removeSelected();
    else if (actionName == QStringLiteral("enable")) enableSelected();
    else if (actionName == QStringLiteral("disable")) disableSelected();
    else if (actionName == QStringLiteral("configs")) viewConfigs();
    else if (actionName == QStringLiteral("folder")) viewFolder();
}

void ModFolderPage::onRunningStatusChanged(bool running)
{
    const bool controlsEnabled = !running;
    if (m_controlsEnabled == controlsEnabled) {
        return;
    }
    m_controlsEnabled = controlsEnabled;
    emit actionStatesChanged();
    emit itemsChanged();
}

void ModFolderPage::addFiles()
{
    if (!m_controlsEnabled) {
        return;
    }
    const auto files = GuiUtil::BrowseForFiles(
        m_helpName,
        tr("Select %1", "Select whatever type of files the page contains. "
                        "Example: 'Loader Mods'")
            .arg(m_displayName),
        m_fileSelectionFilter.arg(m_displayName),
        APPLICATION->settings()->get("CentralModsDir").toString(),
        this->parentWidget());
    for (const auto &filename : files) {
        m_mods->installMod(filename);
    }
}

void ModFolderPage::openDownloader()
{
    Unified::ProjectType projectType = Unified::ProjectType::Mod;
    if (m_id == QStringLiteral("resourcepacks") ||
        m_id == QStringLiteral("texturepacks")) {
        projectType = Unified::ProjectType::ResourcePack;
    } else if (m_id == QStringLiteral("shaderpacks")) {
        projectType = Unified::ProjectType::ShaderPack;
    }
    ModDownloaderDialog dialog(m_inst, projectType, this);
    dialog.exec();
}

void ModFolderPage::updateAll()
{
    QList<Mod> modsToUpdate;
    for (int row = 0; row < m_mods->rowCount(QModelIndex()); ++row) {
        const auto mod = m_mods->at(static_cast<size_t>(row));
        if (mod.hasUpdate() && !mod.updateUrl().isEmpty()) {
            modsToUpdate.append(mod);
        }
    }

    if (modsToUpdate.isEmpty()) {
        CustomMessageBox::information(
            this, tr("Оновлення відсутні"),
            tr("Немає доступних оновлень або посилання на завантаження відсутні."));
        return;
    }

    QString itemTypePlural = tr("елемент(ів)");
    if (m_id == QStringLiteral("mods")) {
        itemTypePlural = tr("мод(ів)");
    } else if (m_id == QStringLiteral("resourcepacks") ||
               m_id == QStringLiteral("texturepacks")) {
        itemTypePlural = tr("ресурспак(ів)");
    } else if (m_id == QStringLiteral("shaderpacks")) {
        itemTypePlural = tr("шейдер(ів)");
    }

    auto *task = new ModUpdateInstallTask(m_inst, m_mods.get(), modsToUpdate, this);
    ProgressDialog progressDialog(this);
    if (progressDialog.execWithTask(task) == QDialog::Accepted) {
        CustomMessageBox::information(
            this, tr("Оновлення завершено"),
            tr("Успішно оновлено %1 %2!")
                .arg(QString::number(modsToUpdate.size()), itemTypePlural));
    } else {
        CustomMessageBox::information(
            this, tr("Помилка"),
            tr("Під час оновлення виникла помилка:\n%1")
                .arg(task->failReason()));
    }
}

void ModFolderPage::removeSelected()
{
    if (!m_controlsEnabled) {
        return;
    }
    QModelIndexList indexes;
    for (const int row : selectedRows()) {
        indexes.append(m_mods->index(row, ModFolderModel::ActiveColumn));
    }
    m_mods->deleteMods(indexes);
}

void ModFolderPage::enableSelected()
{
    applyModStatus(selectedRows(), ModFolderModel::Enable);
}

void ModFolderPage::disableSelected()
{
    applyModStatus(selectedRows(), ModFolderModel::Disable);
}

void ModFolderPage::viewFolder()
{
    DesktopServices::openDirectory(m_mods->dir().absolutePath());
}

void ModFolderPage::viewConfigs()
{
    if (m_showConfigFolder) {
        DesktopServices::openDirectory(m_inst->instanceConfigFolder());
    }
}

CoreModFolderPage::CoreModFolderPage(BaseInstance *inst,
                                     std::shared_ptr<ModFolderModel> mods,
                                     QString id, QString iconName,
                                     QString displayName, QString helpPage,
                                     QWidget *parent)
    : ModFolderPage(inst, std::move(mods), std::move(id), std::move(iconName),
                    std::move(displayName), std::move(helpPage), parent)
{
}

bool CoreModFolderPage::shouldDisplay() const
{
    auto *instance = dynamic_cast<MinecraftInstance *>(m_inst);
    if (!instance) {
        return true;
    }
    auto profile = instance->getPackProfile();
    if (!profile || !profile->getComponent("net.minecraftforge") ||
        !profile->getComponent("net.minecraft")) {
        return false;
    }
    return profile->getComponent("net.minecraft")->getReleaseDateTime() <
           g_VersionFilterData.legacyCutoffDate;
}
