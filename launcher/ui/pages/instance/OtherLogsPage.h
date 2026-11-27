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

#include <QFont>
#include <QStringList>
#include <QWidget>

#include "ui/pages/BasePage.h"
#include <Application.h>
#include <pathmatcher/IPathMatcher.h>

class RecursiveFileSystemWatcher;

class OtherLogsPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY fileSelectionChanged)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(QFont textFont READ textFont NOTIFY contentChanged)

public:
    explicit OtherLogsPage(QString path, IPathMatcher::Ptr fileFilter, QWidget *parent = 0);
    ~OtherLogsPage() override;

    QString id() const override
    {
        return "logs";
    }
    QString displayName() const override
    {
        return tr("Other logs");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("log");
    }
    QString helpPage() const override
    {
        return "Minecraft-Logs";
    }
    void openedImpl() override;
    void closedImpl() override;

    QStringList files() const;
    QString currentFile() const { return m_currentFile; }
    QString content() const { return m_content; }
    QFont textFont() const { return m_textFont; }

    Q_INVOKABLE void selectFile(const QString &file);
    Q_INVOKABLE void reload();
    Q_INVOKABLE void uploadLog();
    Q_INVOKABLE void copyLog();
    Q_INVOKABLE void deleteLog();
    Q_INVOKABLE void cleanLogs();
    Q_INVOKABLE int findInContent(const QString &query, int start,
                                  bool reverse) const;

signals:
    void filesChanged();
    void fileSelectionChanged();
    void contentChanged();

private:
    void refreshFiles();
    QString absoluteFilePath(const QString &file) const;
    void setContent(const QString &content);

private:
    QString m_path;
    QString m_currentFile;
    QString m_content;
    QFont m_textFont;
    RecursiveFileSystemWatcher *m_watcher;
};
