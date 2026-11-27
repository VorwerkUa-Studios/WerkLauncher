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

#include <QList>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>
#include <memory>

#include "minecraft/MinecraftInstance.h"
#include "ui/pages/BasePage.h"

class ModFolderModel;

class ModFolderPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QString pageTitle READ displayName CONSTANT)
    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)
    Q_PROPERTY(QVariantMap selectedMod READ selectedMod NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap actionStates READ actionStates NOTIFY actionStatesChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY itemsChanged)
    Q_PROPERTY(QString sortMode READ sortMode WRITE setSortMode NOTIFY itemsChanged)
    Q_PROPERTY(bool sortDescending READ sortDescending WRITE setSortDescending NOTIFY itemsChanged)

public:
    explicit ModFolderPage(
        BaseInstance *inst,
        std::shared_ptr<ModFolderModel> mods,
        QString id,
        QString iconName,
        QString displayName,
        QString helpPage = {},
        QWidget *parent = nullptr);
    ~ModFolderPage() override;

    void setFilter(const QString &filter) { m_fileSelectionFilter = filter; }
    void setShowConfigFolder(bool show);

    QString displayName() const override { return m_displayName; }
    QIcon icon() const override;
    QString id() const override { return m_id; }
    QString helpPage() const override { return m_helpName; }
    bool shouldDisplay() const override;
    void openedImpl() override;
    void closedImpl() override;

    QVariantList items() const;
    QVariantMap selectedMod() const;
    QVariantMap actionStates() const;
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);
    QString sortMode() const { return m_sortMode; }
    void setSortMode(const QString &mode);
    bool sortDescending() const { return m_sortDescending; }
    void setSortDescending(bool descending);

    Q_INVOKABLE void selectMod(const QString &modId, int modifiers);
    Q_INVOKABLE void setModEnabled(int sourceRow, bool enabled);
    Q_INVOKABLE void installUpdate(int sourceRow);
    Q_INVOKABLE void installDroppedFiles(const QVariantList &urls);
    Q_INVOKABLE void openModHomepage();
    Q_INVOKABLE void triggerAction(const QString &actionName);

signals:
    void itemsChanged();
    void selectionChanged();
    void actionStatesChanged();

private slots:
    void onRunningStatusChanged(bool running);

protected:
    BaseInstance *m_inst = nullptr;

private:
    QVariantMap modData(int sourceRow) const;
    QList<int> visibleRows() const;
    QList<int> selectedRows() const;
    int findModRow(const QString &modId) const;
    void reconcileSelection();
    void refreshAfterModelChange();
    void applyModStatus(const QList<int> &rows, int action);
    void addFiles();
    void openDownloader();
    void updateAll();
    void removeSelected();
    void enableSelected();
    void disableSelected();
    void viewFolder();
    void viewConfigs();

    std::shared_ptr<ModFolderModel> m_mods;
    QString m_iconName;
    QString m_id;
    QString m_displayName;
    QString m_helpName;
    QString m_fileSelectionFilter = QStringLiteral("%1 (*.zip *.jar)");
    QString m_filterText;
    QString m_sortMode = QStringLiteral("name");
    QString m_currentModId;
    QString m_selectionAnchorId;
    QSet<QString> m_selectedModIds;
    bool m_sortDescending = false;
    bool m_controlsEnabled = true;
    bool m_showConfigFolder = true;
    bool m_suppressModelUpdates = false;
};

class CoreModFolderPage : public ModFolderPage
{
public:
    explicit CoreModFolderPage(BaseInstance *inst, std::shared_ptr<ModFolderModel> mods, QString id,
                               QString iconName, QString displayName, QString helpPage = "",
                               QWidget *parent = nullptr);
    ~CoreModFolderPage() override = default;
    bool shouldDisplay() const override;
};
