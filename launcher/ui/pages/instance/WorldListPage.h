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

#pragma once

#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include "minecraft/MinecraftInstance.h"
#include "ui/pages/BasePage.h"
#include <Application.h>
#include <LoggedProcess.h>

class WorldList;

class WorldListPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QString pageTitle READ displayName CONSTANT)
    Q_PROPERTY(QVariantList worlds READ worlds NOTIFY worldsChanged)
    Q_PROPERTY(QVariantMap selectedWorld READ selectedWorld NOTIFY selectedWorldChanged)
    Q_PROPERTY(QVariantMap actionStates READ actionStates NOTIFY actionStatesChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY worldsChanged)
    Q_PROPERTY(QString sortMode READ sortMode WRITE setSortMode NOTIFY worldsChanged)
    Q_PROPERTY(bool sortDescending READ sortDescending WRITE setSortDescending NOTIFY worldsChanged)

public:
    explicit WorldListPage(InstancePtr inst, std::shared_ptr<WorldList> worlds,
                           QWidget *parent = nullptr);
    ~WorldListPage() override;

    QString displayName() const override { return tr("Worlds"); }
    QIcon icon() const override { return APPLICATION->getThemedIcon("worlds"); }
    QString id() const override { return QStringLiteral("worlds"); }
    QString helpPage() const override { return QStringLiteral("Worlds"); }
    bool shouldDisplay() const override;
    void openedImpl() override;
    void closedImpl() override;

    QVariantList worlds() const;
    QVariantMap selectedWorld() const;
    QVariantMap actionStates() const;
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);
    QString sortMode() const { return m_sortMode; }
    void setSortMode(const QString &mode);
    bool sortDescending() const { return m_sortDescending; }
    void setSortDescending(bool descending);

    Q_INVOKABLE void selectWorld(const QString &folderName);
    Q_INVOKABLE void triggerAction(const QString &actionName);

signals:
    void worldsChanged();
    void selectedWorldChanged();
    void actionStatesChanged();

private:
    QVariantMap worldData(int row) const;
    int findWorldRow(const QString &folderName) const;
    void refreshWorlds();
    bool quickPlaySupported() const;
    bool worldSafetyNagQuestion();
    void mceditError();
    void joinSelectedWorld(bool online);
    void addWorld();
    void copySeed();
    void copyWorld();
    void renameWorld();
    void removeWorld();
    void openDatapacks();
    void resetWorldIcon();
    void openMcedit();
    void refreshWorldList();
    void openWorldFolder();

private slots:
    void mceditState(LoggedProcess::State state);

protected:
    InstancePtr m_inst;

private:
    std::shared_ptr<WorldList> m_worlds;
    unique_qobject_ptr<LoggedProcess> m_mceditProcess;
    QString m_selectedWorldFolder;
    QString m_filterText;
    QString m_sortMode = QStringLiteral("name");
    bool m_sortDescending = false;
    bool m_mceditStarting = false;
};
