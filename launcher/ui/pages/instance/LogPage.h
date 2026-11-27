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

#include <QAbstractItemModel>
#include <QFont>
#include <QWidget>

#include "BaseInstance.h"
#include "launch/LaunchTask.h"
#include "ui/pages/BasePage.h"
#include <Application.h>

class LogFormatProxyModel;

class LogPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *logModel READ logModel CONSTANT)
    Q_PROPERTY(bool hasLogModel READ hasLogModel NOTIFY controlsChanged)
    Q_PROPERTY(bool tracking READ tracking WRITE setTracking NOTIFY controlsChanged)
    Q_PROPERTY(bool wrapLines READ wrapLines WRITE setWrapLines NOTIFY controlsChanged)
    Q_PROPERTY(int foundRow READ foundRow NOTIFY foundRowChanged)

public:
    explicit LogPage(InstancePtr instance, QWidget *parent = 0);
    ~LogPage() override;
    virtual QString displayName() const override
    {
        return tr("Minecraft Log");
    }
    virtual QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("log");
    }
    virtual QString id() const override
    {
        return "console";
    }
    virtual bool apply() override;
    virtual QString helpPage() const override
    {
        return "Minecraft-Logs";
    }
    virtual bool shouldDisplay() const override;

    QAbstractItemModel *logModel() const;
    bool hasLogModel() const;
    bool tracking() const { return m_tracking; }
    void setTracking(bool tracking);
    bool wrapLines() const { return m_wrapLines; }
    void setWrapLines(bool wrap);
    int foundRow() const { return m_foundRow; }

    Q_INVOKABLE void copyLog();
    Q_INVOKABLE void uploadLog();
    Q_INVOKABLE void clearLog();
    Q_INVOKABLE int findNext(const QString &text, bool reverse);

signals:
    void controlsChanged();
    void foundRowChanged();

private slots:
    void onInstanceLaunchTaskChanged(shared_qobject_ptr<LaunchTask> proc);

private:
    void setInstanceLaunchTaskChanged(shared_qobject_ptr<LaunchTask> proc, bool initial);

private:
    InstancePtr m_instance;
    shared_qobject_ptr<LaunchTask> m_process;
    LogFormatProxyModel * m_proxy;
    shared_qobject_ptr <LogModel> m_model;
    bool m_tracking = true;
    bool m_wrapLines = true;
    int m_foundRow = -1;
};
