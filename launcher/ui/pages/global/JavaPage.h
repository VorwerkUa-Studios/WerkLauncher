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

#include <QVariant>
#include <memory>
#include <QDialog>
#include "ui/pages/BasePage.h"
#include "JavaCommon.h"
#include <Application.h>
#include <QObjectPtr.h>

class SettingsObject;

class JavaPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(int minMemory READ minMemory WRITE setMinMemory NOTIFY minMemoryChanged)
    Q_PROPERTY(int maxMemory READ maxMemory WRITE setMaxMemory NOTIFY maxMemoryChanged)
    Q_PROPERTY(int permGen READ permGen WRITE setPermGen NOTIFY permGenChanged)
    Q_PROPERTY(int memoryMaximum READ memoryMaximum CONSTANT)
    Q_PROPERTY(QString javaPath READ javaPath WRITE setJavaPath NOTIFY javaPathChanged)
    Q_PROPERTY(QString jvmArgs READ jvmArgs WRITE setJvmArgs NOTIFY jvmArgsChanged)

public:
    explicit JavaPage(QWidget *parent = 0);
    ~JavaPage() override;

    QString displayName() const override
    {
        return tr("Java");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("java");
    }
    QString id() const override
    {
        return "java-settings";
    }
    QString helpPage() const override
    {
        return "Java-settings";
    }
    bool apply() override;

    int minMemory() const { return m_minMemory; }
    void setMinMemory(int value);
    int maxMemory() const { return m_maxMemory; }
    void setMaxMemory(int value);
    int permGen() const { return m_permGen; }
    void setPermGen(int value);
    int memoryMaximum() const { return m_memoryMaximum; }
    QString javaPath() const { return m_javaPath; }
    void setJavaPath(const QString &path);
    QString jvmArgs() const { return m_jvmArgs; }
    void setJvmArgs(const QString &args);

    Q_INVOKABLE void browseJava();
    Q_INVOKABLE void detectJava();
    Q_INVOKABLE void testJava();

signals:
    void minMemoryChanged();
    void maxMemoryChanged();
    void permGenChanged();
    void javaPathChanged();
    void jvmArgsChanged();

private:
    void applySettings();
    void loadSettings();

private:
    unique_qobject_ptr<JavaCommon::TestCheck> checker;
    int m_minMemory = 512;
    int m_maxMemory = 1024;
    int m_permGen = 64;
    int m_memoryMaximum = 65536;
    QString m_javaPath;
    QString m_jvmArgs;
};
