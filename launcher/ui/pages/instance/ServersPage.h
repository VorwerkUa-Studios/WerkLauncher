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

#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include "ui/pages/BasePage.h"
#include <Application.h>

class ServersModel;

class ServersPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QString pageTitle READ displayName CONSTANT)
    Q_PROPERTY(QVariantList servers READ servers NOTIFY serversChanged)
    Q_PROPERTY(QVariantMap selectedServer READ selectedServer NOTIFY selectedServerChanged)
    Q_PROPERTY(QVariantMap actionStates READ actionStates NOTIFY actionStatesChanged)

public:
    explicit ServersPage(InstancePtr inst, QWidget *parent = nullptr);
    ~ServersPage() override;

    void openedImpl() override;
    void closedImpl() override;
    QString displayName() const override { return tr("Servers"); }
    QIcon icon() const override { return APPLICATION->getThemedIcon("unknown_server"); }
    QString id() const override { return QStringLiteral("servers"); }
    QString helpPage() const override { return QStringLiteral("Servers-management"); }

    QVariantList servers() const;
    QVariantMap selectedServer() const;
    QVariantMap actionStates() const;

    Q_INVOKABLE void selectServer(int row);
    Q_INVOKABLE void setServerName(const QString &name);
    Q_INVOKABLE void setServerAddress(const QString &address);
    Q_INVOKABLE void setAcceptsTextures(int index);
    Q_INVOKABLE void triggerAction(const QString &actionName);

signals:
    void serversChanged();
    void selectedServerChanged();
    void actionStatesChanged();

private:
    void updateState();
    void addServer();
    void removeServer();
    void moveServerUp();
    void moveServerDown();
    void joinServer();

private slots:
    void rowsRemoved(const QModelIndex &parent, int first, int last);
    void onRunningStatusChanged(bool running);

private:
    int currentServer = -1;
    bool m_locked = true;
    ServersModel *m_model = nullptr;
    InstancePtr m_inst = nullptr;
};
