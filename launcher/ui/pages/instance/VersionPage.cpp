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

#include "VersionPage.h"

#include <QFileInfo>
#include <QHash>
#include <QLabel>
#include <QMessageBox>
#include <QQmlError>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>

#include "Application.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/VersionSelectDialog.h"
#include "ui/dialogs/NewComponentDialog.h"
#include "ui/dialogs/ProgressDialog.h"
#include "ui/GuiUtil.h"
#include "minecraft/PackProfile.h"
#include "minecraft/auth/AccountList.h"
#include "minecraft/VersionFilterData.h"
#include "icons/IconList.h"
#include "Exception.h"
#include "Version.h"
#include "DesktopServices.h"
#include "meta/Index.h"
#include "meta/VersionList.h"

QIcon VersionPage::icon() const
{
    return APPLICATION->icons()->getIcon(m_inst->iconKey());
}
bool VersionPage::shouldDisplay() const
{
    return true;
}

VersionPage::VersionPage(MinecraftInstance *inst, QWidget *parent)
   : QWidget(parent), m_inst(inst)
{
    m_profile = m_inst->getPackProfile();
    reloadPackProfile();

    auto publishProfileChange = [this]() {
       if (currentIdx >= m_profile->rowCount()) {
           currentIdx = m_profile->rowCount() - 1;
       }
        emit componentsChanged();
        emit selectionChanged();
        updateButtons();
    };
    connect(m_profile.get(), &QAbstractItemModel::rowsInserted, this,
            [publishProfileChange]() { publishProfileChange(); });
    connect(m_profile.get(), &QAbstractItemModel::rowsRemoved, this,
            [publishProfileChange]() { publishProfileChange(); });
    connect(m_profile.get(), &QAbstractItemModel::rowsMoved, this,
            [publishProfileChange]() { publishProfileChange(); });
    connect(m_profile.get(), &QAbstractItemModel::dataChanged, this,
            [publishProfileChange]() { publishProfileChange(); });
    connect(m_profile.get(), &QAbstractItemModel::modelReset, this,
            [publishProfileChange]() { publishProfileChange(); });
    connect(m_profile.get(), &QAbstractItemModel::layoutChanged, this,
            [publishProfileChange]() { publishProfileChange(); });
    connect(m_profile.get(), &PackProfile::minecraftChanged, this, &VersionPage::updateVersionControls);

    controlsEnabled = !m_inst->isRunning();
    updateVersionControls();
    preselect(0);
    connect(m_inst, &BaseInstance::runningStatusChanged, this, &VersionPage::updateRunningStatus);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("versionPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("versionController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The version page could not be loaded."), this);
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
        qCritical() << "Version page QML failed to load:"
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/VersionPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready &&
         !qmlWidget->rootObject())) {
        showLoadError();
    }
}

VersionPage::~VersionPage() = default;

QVariantMap VersionPage::componentData(int sourceRow) const
{
    auto *component = m_profile->getComponent(sourceRow);
    if (!component) {
        return {};
    }

    const auto severity = component->getProblemSeverity();
    QString severityName;
    QString summary;
    if (severity == ProblemSeverity::Warning) {
        severityName = QStringLiteral("warning");
        summary = tr("%1 possibly has issues.").arg(component->getName());
    } else if (severity == ProblemSeverity::Error) {
        severityName = QStringLiteral("error");
        summary = tr("%1 has issues!").arg(component->getName());
    }

    QStringList problems;
    for (const auto &problem : component->getProblems()) {
        QString prefix;
        if (problem.m_severity == ProblemSeverity::Error) {
            prefix = tr("Error: ");
        } else if (problem.m_severity == ProblemSeverity::Warning) {
            prefix = tr("Warning: ");
        }
        problems.append(prefix + problem.m_description);
    }

    QVariantMap result;
    result.insert(QStringLiteral("sourceRow"), sourceRow);
    result.insert(QStringLiteral("id"), component->getID());
    result.insert(QStringLiteral("name"), component->getName());
    result.insert(QStringLiteral("version"),
                  component->isCustom()
                      ? tr("%1 (Custom)").arg(component->getVersion())
                      : component->getVersion());
    result.insert(QStringLiteral("enabled"), component->isEnabled());
    result.insert(QStringLiteral("canBeDisabled"), component->canBeDisabled());
    result.insert(QStringLiteral("severity"), severityName);
    result.insert(QStringLiteral("summary"), summary);
    result.insert(QStringLiteral("problems"), problems.join('\n'));
    return result;
}

QVariantList VersionPage::components() const
{
    QVariantList result;
    if (!m_profile) {
        return result;
    }

    const QString filter = m_filterText.trimmed();
    for (int row = 0; row < m_profile->rowCount(); ++row) {
        auto item = componentData(row);
        const QString searchable = item.value(QStringLiteral("name")).toString()
                                   + QLatin1Char(' ')
                                   + item.value(QStringLiteral("version")).toString()
                                   + QLatin1Char(' ')
                                   + item.value(QStringLiteral("id")).toString();
        if (!filter.isEmpty() &&
            !searchable.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }
        result.append(item);
    }
    return result;
}

QVariantMap VersionPage::selectedComponent() const
{
    return componentData(currentIdx);
}

QVariantMap VersionPage::actionStates() const
{
    auto *component = m_profile->getComponent(currentIdx);
    const bool selected = component != nullptr;
    const bool editable = controlsEnabled && selected;
    QVariantMap result;
    result.insert(QStringLiteral("changeVersion"),
                  editable && component->isVersionChangeable());
    result.insert(QStringLiteral("moveUp"), editable && component->isMoveable());
    result.insert(QStringLiteral("moveDown"), editable && component->isMoveable());
    result.insert(QStringLiteral("remove"), editable && component->isRemovable());
    result.insert(QStringLiteral("customize"), editable && component->isCustomizable());
    result.insert(QStringLiteral("edit"), editable && component->isCustom());
    result.insert(QStringLiteral("revert"), editable && component->isRevertible());
    result.insert(QStringLiteral("installForge"), controlsEnabled);
    result.insert(QStringLiteral("installNeoForge"),
                  controlsEnabled && supportsNeoForge);
    result.insert(QStringLiteral("installFabric"),
                  controlsEnabled && supportsFabric);
    result.insert(QStringLiteral("installQuilt"),
                  controlsEnabled && supportsFabric);
    result.insert(QStringLiteral("installLiteLoader"),
                  controlsEnabled && supportsLiteLoader);
    result.insert(QStringLiteral("installMods"), controlsEnabled);
    result.insert(QStringLiteral("addJarMods"), controlsEnabled);
    result.insert(QStringLiteral("replaceMinecraftJar"), controlsEnabled);
    result.insert(QStringLiteral("addEmpty"), controlsEnabled);
    result.insert(QStringLiteral("openMinecraftFolder"), true);
    result.insert(QStringLiteral("openLibrariesFolder"), true);
    result.insert(QStringLiteral("reload"), controlsEnabled);
    result.insert(QStringLiteral("downloadAll"), controlsEnabled);
    return result;
}

void VersionPage::setFilterText(const QString &text)
{
    if (m_filterText == text) {
        return;
    }
    m_filterText = text;
    emit componentsChanged();
}

void VersionPage::selectComponent(int sourceRow)
{
    if (sourceRow < 0 || sourceRow >= m_profile->rowCount()) {
        qWarning() << "Ignoring invalid component selection" << sourceRow;
        return;
    }
    currentIdx = sourceRow;
    emit selectionChanged();
    updateButtons();
}

void VersionPage::setComponentEnabled(int sourceRow, bool enabled)
{
    if (!controlsEnabled) {
        return;
    }
    auto *component = m_profile->getComponent(sourceRow);
    if (!component) {
        qWarning() << "Ignoring invalid component enabled-state change"
                   << sourceRow;
        return;
    }
    if (!component->canBeDisabled()) {
        return;
    }
    component->setEnabled(enabled);
    emit componentsChanged();
}

void VersionPage::triggerAction(const QString &actionName)
{
    static const QHash<QString, QString> actionAvailability = {
        {QStringLiteral("actionChange_version"), QStringLiteral("changeVersion")},
        {QStringLiteral("actionMove_up"), QStringLiteral("moveUp")},
        {QStringLiteral("actionMove_down"), QStringLiteral("moveDown")},
        {QStringLiteral("actionRemove"), QStringLiteral("remove")},
        {QStringLiteral("actionCustomize"), QStringLiteral("customize")},
        {QStringLiteral("actionEdit"), QStringLiteral("edit")},
        {QStringLiteral("actionRevert"), QStringLiteral("revert")},
        {QStringLiteral("actionInstall_Forge"), QStringLiteral("installForge")},
        {QStringLiteral("actionInstall_NeoForge"), QStringLiteral("installNeoForge")},
        {QStringLiteral("actionInstall_Fabric"), QStringLiteral("installFabric")},
        {QStringLiteral("actionInstall_Quilt"), QStringLiteral("installQuilt")},
        {QStringLiteral("actionInstall_LiteLoader"), QStringLiteral("installLiteLoader")},
        {QStringLiteral("actionInstall_mods"), QStringLiteral("installMods")},
        {QStringLiteral("actionAdd_to_Minecraft_jar"), QStringLiteral("addJarMods")},
        {QStringLiteral("actionReplace_Minecraft_jar"), QStringLiteral("replaceMinecraftJar")},
        {QStringLiteral("actionAdd_Empty"), QStringLiteral("addEmpty")},
        {QStringLiteral("actionMinecraftFolder"), QStringLiteral("openMinecraftFolder")},
        {QStringLiteral("actionLibrariesFolder"), QStringLiteral("openLibrariesFolder")},
        {QStringLiteral("actionReload"), QStringLiteral("reload")},
        {QStringLiteral("actionDownload_All"), QStringLiteral("downloadAll")},
    };
    const auto availability = actionAvailability.constFind(actionName);
    if (availability == actionAvailability.constEnd()) {
        qWarning() << "Ignoring unknown version-page action" << actionName;
        return;
    }
    if (!actionStates().value(*availability).toBool()) {
        return;
    }

    if (actionName == QStringLiteral("actionChange_version")) on_actionChange_version_triggered();
    else if (actionName == QStringLiteral("actionMove_up")) on_actionMove_up_triggered();
    else if (actionName == QStringLiteral("actionMove_down")) on_actionMove_down_triggered();
    else if (actionName == QStringLiteral("actionRemove")) on_actionRemove_triggered();
    else if (actionName == QStringLiteral("actionCustomize")) on_actionCustomize_triggered();
    else if (actionName == QStringLiteral("actionEdit")) on_actionEdit_triggered();
    else if (actionName == QStringLiteral("actionRevert")) on_actionRevert_triggered();
    else if (actionName == QStringLiteral("actionInstall_Forge")) on_actionInstall_Forge_triggered();
    else if (actionName == QStringLiteral("actionInstall_NeoForge")) on_actionInstall_NeoForge_triggered();
    else if (actionName == QStringLiteral("actionInstall_Fabric")) on_actionInstall_Fabric_triggered();
    else if (actionName == QStringLiteral("actionInstall_Quilt")) on_actionInstall_Quilt_triggered();
    else if (actionName == QStringLiteral("actionInstall_LiteLoader")) on_actionInstall_LiteLoader_triggered();
    else if (actionName == QStringLiteral("actionInstall_mods")) on_actionInstall_mods_triggered();
    else if (actionName == QStringLiteral("actionAdd_to_Minecraft_jar")) on_actionAdd_to_Minecraft_jar_triggered();
    else if (actionName == QStringLiteral("actionReplace_Minecraft_jar")) on_actionReplace_Minecraft_jar_triggered();
    else if (actionName == QStringLiteral("actionAdd_Empty")) on_actionAdd_Empty_triggered();
    else if (actionName == QStringLiteral("actionMinecraftFolder")) on_actionMinecraftFolder_triggered();
    else if (actionName == QStringLiteral("actionLibrariesFolder")) on_actionLibrariesFolder_triggered();
    else if (actionName == QStringLiteral("actionReload")) on_actionReload_triggered();
    else if (actionName == QStringLiteral("actionDownload_All")) on_actionDownload_All_triggered();
}

void VersionPage::updateRunningStatus(bool running)
{
    if(controlsEnabled == running) {
        controlsEnabled = !running;
        updateVersionControls();
    }
}

void VersionPage::updateVersionControls()
{
    supportsFabric = false;
    supportsLiteLoader = false;
    supportsNeoForge = false;

    auto component = m_profile->getComponent("net.minecraft");
    if(component)
    {
        // FIXME: This is better than the broken stuff we had before, but it would probably be better to handle this in meta somehow
        auto minecraftReleaseDate = m_profile->getComponent("net.minecraft")->getReleaseDateTime();
        supportsFabric = minecraftReleaseDate >= g_VersionFilterData.fabricBeginsDate;
        supportsLiteLoader = minecraftReleaseDate <= g_VersionFilterData.liteLoaderEndsDate;
        supportsNeoForge = minecraftReleaseDate >= g_VersionFilterData.neoForgeBeginsDate;
    }

    updateButtons();
}

void VersionPage::updateButtons()
{
    emit actionStatesChanged();
}

bool VersionPage::reloadPackProfile()
{
    try
    {
        m_profile->reload(Net::Mode::Online);
        return true;
    }
    catch (const Exception &e)
    {
        QMessageBox::critical(this, tr("Error"), e.cause());
        return false;
    }
    catch (...)
    {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Couldn't load the instance profile."));
        return false;
    }
}

void VersionPage::on_actionReload_triggered()
{
    reloadPackProfile();
    m_container->refreshContainer();
}
void VersionPage::on_actionRemove_triggered()
{
    const int row = currentRow();
    if (row >= 0)
    {
        // FIXME: use actual model, not reloading.
        if (!m_profile->remove(row))
        {
            QMessageBox::critical(this, tr("Error"), tr("Couldn't remove file"));
        }
    }
    updateButtons();
    reloadPackProfile();
    m_container->refreshContainer();
}

void VersionPage::on_actionInstall_mods_triggered()
{
    if(m_container)
    {
        m_container->selectPage("mods");
    }
}

void VersionPage::on_actionAdd_to_Minecraft_jar_triggered()
{
    auto list = GuiUtil::BrowseForFiles("jarmod", tr("Select jar mods"), tr("Minecraft.jar mods (*.zip *.jar)"), QString(), this->parentWidget());
    if(!list.empty())
    {
        m_profile->installJarMods(list);
    }
    updateButtons();
}

void VersionPage::on_actionReplace_Minecraft_jar_triggered()
{
    auto jarPath = GuiUtil::BrowseForFile("jar", tr("Select jar"), tr("Minecraft.jar replacement (*.jar)"), QString(), this->parentWidget());
    if(!jarPath.isEmpty())
    {
        m_profile->installCustomJar(jarPath);
    }
    updateButtons();
}

void VersionPage::on_actionMove_up_triggered()
{
    const int row = currentRow();
    auto *selectedComponent = current();
    if (row < 0 || !selectedComponent) {
        return;
    }

    try
    {
        m_profile->move(row, PackProfile::MoveUp);
    }
    catch (const Exception &e)
    {
        QMessageBox::critical(this, tr("Error"), e.cause());
    }

    int selectedRow = row;
    for (int candidateRow = 0; candidateRow < m_profile->rowCount(); ++candidateRow) {
        if (m_profile->getComponent(candidateRow) == selectedComponent) {
            selectedRow = candidateRow;
            break;
        }
    }
    preselect(selectedRow);
}

void VersionPage::on_actionMove_down_triggered()
{
    const int row = currentRow();
    auto *selectedComponent = current();
    if (row < 0 || !selectedComponent) {
        return;
    }

    try
    {
        m_profile->move(row, PackProfile::MoveDown);
    }
    catch (const Exception &e)
    {
        QMessageBox::critical(this, tr("Error"), e.cause());
    }

    int selectedRow = row;
    for (int candidateRow = 0; candidateRow < m_profile->rowCount(); ++candidateRow) {
        if (m_profile->getComponent(candidateRow) == selectedComponent) {
            selectedRow = candidateRow;
            break;
        }
    }
    preselect(selectedRow);
}

void VersionPage::on_actionChange_version_triggered()
{
    auto versionRow = currentRow();
    if(versionRow == -1)
    {
        return;
    }
    auto patch = m_profile->getComponent(versionRow);
    auto name = patch->getName();
    auto list = patch->getVersionList();
    if(!list)
    {
        return;
    }
    auto uid = list->uid();
    // FIXME: this is a horrible HACK. Get version filtering information from the actual metadata...
    if(uid == "net.minecraftforge")
    {
        on_actionInstall_Forge_triggered();
        return;
    }
    else if (uid == "net.neoforged")
    {
        on_actionInstall_NeoForge_triggered();
        return;
    }
    else if (uid == "com.mumfrey.liteloader")
    {
        on_actionInstall_LiteLoader_triggered();
        return;
    }
    VersionSelectDialog vselect(list.get(), tr("Change %1 version").arg(name), this);
    if (uid == "net.fabricmc.intermediary" || uid == "org.quiltmc.hashed")
    {
        vselect.setEmptyString(tr("No intermediary mappings versions are currently available."));
        vselect.setEmptyErrorString(tr("Couldn't load or download the intermediary mappings version lists!"));
        vselect.setExactFilter(BaseVersionList::ParentVersionRole, m_profile->getComponentVersion("net.minecraft"));
    }
    auto currentVersion = patch->getVersion();
    if(!currentVersion.isEmpty())
    {
        vselect.setCurrentVersion(currentVersion);
    }
    if (!vselect.exec() || !vselect.selectedVersion())
        return;

    qDebug() << "Change" << uid << "to" << vselect.selectedVersion()->descriptor();
    bool important = false;
    if(uid == "net.minecraft")
    {
        important = true;
    }
    m_profile->setComponentVersion(uid, vselect.selectedVersion()->descriptor(), important);
    m_profile->resolve(Net::Mode::Online);
    m_container->refreshContainer();
}

void VersionPage::on_actionDownload_All_triggered()
{
    if (!APPLICATION->accounts()->anyAccountIsValid())
    {
        CustomMessageBox::selectable(
            this, tr("Error"),
            tr("MultiMC cannot download Minecraft or update instances unless you have at least "
               "one account added.\nPlease add your Mojang or Minecraft account."),
            QMessageBox::Warning)->show();
        return;
    }

    auto updateTask = m_inst->createUpdateTask(Net::Mode::Online);
    if (!updateTask)
    {
        return;
    }
    ProgressDialog tDialog(this);
    connect(updateTask.get(), SIGNAL(failed(QString)), SLOT(onGameUpdateError(QString)));
    // FIXME: unused return value
    tDialog.execWithTask(updateTask.get());
    updateButtons();
    m_container->refreshContainer();
}

void VersionPage::on_actionInstall_Forge_triggered()
{
    auto vlist = APPLICATION->metadataIndex()->get("net.minecraftforge");
    if(!vlist)
    {
        return;
    }
    VersionSelectDialog vselect(vlist.get(), tr("Select Forge version"), this);
    vselect.setExactFilter(BaseVersionList::ParentVersionRole, m_profile->getComponentVersion("net.minecraft"));
    vselect.setEmptyString(tr("No Forge versions are currently available for Minecraft ") + m_profile->getComponentVersion("net.minecraft"));
    vselect.setEmptyErrorString(tr("Couldn't load or download the Forge version lists!"));

    auto currentVersion = m_profile->getComponentVersion("net.minecraftforge");
    if(!currentVersion.isEmpty())
    {
        vselect.setCurrentVersion(currentVersion);
    }

    if (vselect.exec() && vselect.selectedVersion())
    {
        auto vsn = vselect.selectedVersion();
        m_profile->setComponentVersion("net.minecraftforge", vsn->descriptor());
        m_profile->resolve(Net::Mode::Online);
        // m_profile->installVersion();
        preselect(m_profile->rowCount(QModelIndex())-1);
        m_container->refreshContainer();
    }
}

void VersionPage::on_actionInstall_NeoForge_triggered()
{
    auto vlist = APPLICATION->metadataIndex()->get("net.neoforged");
    if(!vlist)
    {
        return;
    }
    VersionSelectDialog vselect(vlist.get(), tr("Select NeoForge version"), this);
    vselect.setExactFilter(BaseVersionList::ParentVersionRole, m_profile->getComponentVersion("net.minecraft"));
    vselect.setEmptyString(tr("No NeoForge versions are currently available for Minecraft ") + m_profile->getComponentVersion("net.minecraft"));
    vselect.setEmptyErrorString(tr("Couldn't load or download the NeoForge version lists!"));

    auto currentVersion = m_profile->getComponentVersion("net.neoforged");
    if(!currentVersion.isEmpty())
    {
        vselect.setCurrentVersion(currentVersion);
    }

    if (vselect.exec() && vselect.selectedVersion())
    {
        auto vsn = vselect.selectedVersion();
        m_profile->setComponentVersion("net.neoforged", vsn->descriptor());
        m_profile->resolve(Net::Mode::Online);
        // m_profile->installVersion();
        preselect(m_profile->rowCount(QModelIndex())-1);
        m_container->refreshContainer();
    }
}

void VersionPage::on_actionInstall_Fabric_triggered()
{
    auto vlist = APPLICATION->metadataIndex()->get("net.fabricmc.fabric-loader");
    if(!vlist)
    {
        return;
    }
    VersionSelectDialog vselect(vlist.get(), tr("Select Fabric Loader version"), this);
    vselect.setEmptyString(tr("No Fabric Loader versions are currently available."));
    vselect.setEmptyErrorString(tr("Couldn't load or download the Fabric Loader version lists!"));

    auto currentVersion = m_profile->getComponentVersion("net.fabricmc.fabric-loader");
    if(!currentVersion.isEmpty())
    {
        vselect.setCurrentVersion(currentVersion);
    }

    if (vselect.exec() && vselect.selectedVersion())
    {
        auto vsn = vselect.selectedVersion();
        m_profile->setComponentVersion("net.fabricmc.fabric-loader", vsn->descriptor());
        m_profile->resolve(Net::Mode::Online);
        preselect(m_profile->rowCount(QModelIndex())-1);
        m_container->refreshContainer();
    }
}

void VersionPage::on_actionInstall_Quilt_triggered()
{
    auto vlist = APPLICATION->metadataIndex()->get("org.quiltmc.quilt-loader");
    if(!vlist)
    {
        return;
    }
    VersionSelectDialog vselect(vlist.get(), tr("Select Quilt Loader version"), this);
    vselect.setEmptyString(tr("No Quilt Loader versions are currently available."));
    vselect.setEmptyErrorString(tr("Couldn't load or download the Quilt Loader version lists!"));

    auto currentVersion = m_profile->getComponentVersion("org.quiltmc.quilt-loader");
    if(!currentVersion.isEmpty())
    {
        vselect.setCurrentVersion(currentVersion);
    }

    if (vselect.exec() && vselect.selectedVersion())
    {
        auto vsn = vselect.selectedVersion();
        m_profile->setComponentVersion("org.quiltmc.quilt-loader", vsn->descriptor());
        m_profile->resolve(Net::Mode::Online);
        preselect(m_profile->rowCount(QModelIndex())-1);
        m_container->refreshContainer();
    }
}

void VersionPage::on_actionAdd_Empty_triggered()
{
    NewComponentDialog compdialog(QString(), QString(), this);
    QStringList blacklist;
    for(int i = 0; i < m_profile->rowCount(); i++)
    {
        auto comp = m_profile->getComponent(i);
        blacklist.push_back(comp->getID());
    }
    compdialog.setBlacklist(blacklist);
    if (compdialog.exec())
    {
        qDebug() << "name:" << compdialog.name();
        qDebug() << "uid:" << compdialog.uid();
        m_profile->installEmpty(compdialog.uid(), compdialog.name());
    }
}

void VersionPage::on_actionInstall_LiteLoader_triggered()
{
    auto vlist = APPLICATION->metadataIndex()->get("com.mumfrey.liteloader");
    if(!vlist)
    {
        return;
    }
    VersionSelectDialog vselect(vlist.get(), tr("Select LiteLoader version"), this);
    vselect.setExactFilter(BaseVersionList::ParentVersionRole, m_profile->getComponentVersion("net.minecraft"));
    vselect.setEmptyString(tr("No LiteLoader versions are currently available for Minecraft ") + m_profile->getComponentVersion("net.minecraft"));
    vselect.setEmptyErrorString(tr("Couldn't load or download the LiteLoader version lists!"));

    auto currentVersion = m_profile->getComponentVersion("com.mumfrey.liteloader");
    if(!currentVersion.isEmpty())
    {
        vselect.setCurrentVersion(currentVersion);
    }

    if (vselect.exec() && vselect.selectedVersion())
    {
        auto vsn = vselect.selectedVersion();
        m_profile->setComponentVersion("com.mumfrey.liteloader", vsn->descriptor());
        m_profile->resolve(Net::Mode::Online);
        // m_profile->installVersion(vselect.selectedVersion());
        preselect(m_profile->rowCount(QModelIndex())-1);
        m_container->refreshContainer();
    }
}

void VersionPage::on_actionLibrariesFolder_triggered()
{
    DesktopServices::openDirectory(m_inst->getLocalLibraryPath());
}

void VersionPage::on_actionMinecraftFolder_triggered()
{
    DesktopServices::openDirectory(m_inst->gameRoot());
}

void VersionPage::preselect(int row)
{
    if (row < 0) {
        row = 0;
    }
    if (row >= m_profile->rowCount()) {
        row = m_profile->rowCount() - 1;
    }
    currentIdx = row;
    emit selectionChanged();
    updateButtons();
}

void VersionPage::onGameUpdateError(QString error)
{
    CustomMessageBox::selectable(this, tr("Error updating instance"), error, QMessageBox::Warning)->show();
}

Component * VersionPage::current()
{
    auto row = currentRow();
    if(row < 0)
    {
        return nullptr;
    }
    return m_profile->getComponent(row);
}

int VersionPage::currentRow() const { return currentIdx; }

void VersionPage::on_actionCustomize_triggered()
{
    auto version = currentRow();
    if(version == -1)
    {
        return;
    }
    auto patch = m_profile->getComponent(version);
    if(!patch->getVersionFile())
    {
        // TODO: wait for the update task to finish here...
        return;
    }
    if(!m_profile->customize(version))
    {
        // TODO: some error box here
    }
    updateButtons();
    preselect(currentIdx);
}

void VersionPage::on_actionEdit_triggered()
{
    auto version = current();
    if(!version)
    {
        return;
    }
    auto filename = version->getFilename();
    if(!QFileInfo::exists(filename))
    {
        qWarning() << "file" << filename << "can't be opened for editing, doesn't exist!";
        return;
    }
    APPLICATION->openJsonEditor(filename);
}

void VersionPage::on_actionRevert_triggered()
{
    auto version = currentRow();
    if(version == -1)
    {
        return;
    }
    if(!m_profile->revertToBase(version))
    {
        // TODO: some error box here
    }
    updateButtons();
    preselect(currentIdx);
    m_container->refreshContainer();
}
