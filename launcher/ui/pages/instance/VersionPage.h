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

#pragma once

#include <QWidget>
#include <QVariantList>
#include <QVariantMap>

#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "ui/pages/BasePage.h"

class VersionPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QVariantList components READ components NOTIFY componentsChanged)
    Q_PROPERTY(QVariantMap selectedComponent READ selectedComponent NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap actionStates READ actionStates NOTIFY actionStatesChanged)
    Q_PROPERTY(int selectedComponentIndex READ selectedComponentIndex NOTIFY selectionChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY componentsChanged)

public:
    explicit VersionPage(MinecraftInstance *inst, QWidget *parent = nullptr);
    ~VersionPage() override;

    QString displayName() const override { return tr("Version"); }
    QIcon icon() const override;
    QString id() const override { return QStringLiteral("version"); }
    QString helpPage() const override { return QStringLiteral("Instance-Version"); }
    bool shouldDisplay() const override;

    QVariantList components() const;
    QVariantMap selectedComponent() const;
    QVariantMap actionStates() const;
    int selectedComponentIndex() const { return currentIdx; }
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);

    Q_INVOKABLE void selectComponent(int sourceRow);
    Q_INVOKABLE void setComponentEnabled(int sourceRow, bool enabled);
    Q_INVOKABLE void triggerAction(const QString &actionName);

signals:
    void componentsChanged();
    void selectionChanged();
    void actionStatesChanged();

private slots:
    void on_actionChange_version_triggered();
    void on_actionInstall_Forge_triggered();
    void on_actionInstall_NeoForge_triggered();
    void on_actionInstall_Fabric_triggered();
    void on_actionInstall_Quilt_triggered();
    void on_actionAdd_Empty_triggered();
    void on_actionInstall_LiteLoader_triggered();
    void on_actionReload_triggered();
    void on_actionRemove_triggered();
    void on_actionMove_up_triggered();
    void on_actionMove_down_triggered();
    void on_actionAdd_to_Minecraft_jar_triggered();
    void on_actionReplace_Minecraft_jar_triggered();
    void on_actionRevert_triggered();
    void on_actionEdit_triggered();
    void on_actionInstall_mods_triggered();
    void on_actionCustomize_triggered();
    void on_actionDownload_All_triggered();
    void on_actionMinecraftFolder_triggered();
    void on_actionLibrariesFolder_triggered();
    void updateVersionControls();
    void updateRunningStatus(bool running);
    void onGameUpdateError(QString error);

private:
    Component *current();
    int currentRow() const;
    QVariantMap componentData(int sourceRow) const;
    void updateButtons();
    void preselect(int row = 0);
    bool reloadPackProfile();

    std::shared_ptr<PackProfile> m_profile;
    MinecraftInstance *m_inst;
    int currentIdx = -1;
    bool controlsEnabled = false;
    bool supportsFabric = false;
    bool supportsLiteLoader = false;
    bool supportsNeoForge = false;
    QString m_filterText;
};
