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

#include <QSet>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include "ui/pages/BasePage.h"
#include <Application.h>

class QFileSystemWatcher;

class ScreenshotsPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QString pageTitle READ displayName CONSTANT)
    Q_PROPERTY(QVariantList screenshots READ screenshots NOTIFY screenshotsChanged)
    Q_PROPERTY(QVariantMap actionStates READ actionStates NOTIFY actionStatesChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)

public:
    explicit ScreenshotsPage(QString path, QWidget *parent = nullptr);
    ~ScreenshotsPage() override;

    void openedImpl() override;
    void closedImpl() override;
    QString displayName() const override { return tr("Screenshots"); }
    QIcon icon() const override { return APPLICATION->getThemedIcon("screenshots"); }
    QString id() const override { return QStringLiteral("screenshots"); }
    QString helpPage() const override { return QStringLiteral("Screenshots-management"); }
    bool apply() override { return !m_uploadActive; }

    QVariantList screenshots() const;
    QVariantMap actionStates() const;
    int selectedCount() const { return m_selectedPaths.size(); }

    Q_INVOKABLE void selectScreenshot(const QString &path, int modifiers);
    Q_INVOKABLE void openScreenshot(const QString &path);
    Q_INVOKABLE void triggerAction(const QString &actionName);

signals:
    void screenshotsChanged();
    void selectionChanged();
    void actionStatesChanged();

private slots:
    void directoryChanged(const QString &path);

private:
    void refreshScreenshots();
    void uploadSelected();
    void copyImage();
    void copyFiles();
    void deleteSelected();
    void renameSelected();
    void openFolder();

    QString m_folder;
    QFileSystemWatcher *m_watcher = nullptr;
    QSet<QString> m_selectedPaths;
    QString m_currentPath;
    QString m_selectionAnchorPath;
    bool m_valid = false;
    bool m_uploadActive = false;
};
