/* Copyright 2013-2023 MultiMC Contributors
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

#include "ui/pages/BasePage.h"
#include "Application.h"

class BaseInstance;

class InstanceSettingsPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(bool windowOverride READ windowOverride WRITE setWindowOverride NOTIFY settingsChanged)
    Q_PROPERTY(bool launchMaximized READ launchMaximized WRITE setLaunchMaximized NOTIFY settingsChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY settingsChanged)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY settingsChanged)
    Q_PROPERTY(bool consoleOverride READ consoleOverride WRITE setConsoleOverride NOTIFY settingsChanged)
    Q_PROPERTY(bool showConsole READ showConsole WRITE setShowConsole NOTIFY settingsChanged)
    Q_PROPERTY(bool autoCloseConsole READ autoCloseConsole WRITE setAutoCloseConsole NOTIFY settingsChanged)
    Q_PROPERTY(bool showConsoleOnError READ showConsoleOnError WRITE setShowConsoleOnError NOTIFY settingsChanged)

public:
    explicit InstanceSettingsPage(BaseInstance *inst, QWidget *parent = nullptr);
    ~InstanceSettingsPage() override = default;

    QString displayName() const override;
    QIcon icon() const override;
    QString id() const override;
    bool apply() override;
    bool shouldDisplay() const override;

    bool windowOverride() const { return m_windowOverride; }
    void setWindowOverride(bool value);
    bool launchMaximized() const { return m_launchMaximized; }
    void setLaunchMaximized(bool value);
    int windowWidth() const { return m_windowWidth; }
    void setWindowWidth(int value);
    int windowHeight() const { return m_windowHeight; }
    void setWindowHeight(int value);
    bool consoleOverride() const { return m_consoleOverride; }
    void setConsoleOverride(bool value);
    bool showConsole() const { return m_showConsole; }
    void setShowConsole(bool value);
    bool autoCloseConsole() const { return m_autoCloseConsole; }
    void setAutoCloseConsole(bool value);
    bool showConsoleOnError() const { return m_showConsoleOnError; }
    void setShowConsoleOnError(bool value);

signals:
    void settingsChanged();

private:
    BaseInstance *m_instance;
    SettingsObjectPtr m_settings;
    bool m_windowOverride = false;
    bool m_launchMaximized = false;
    int m_windowWidth = 854;
    int m_windowHeight = 480;
    bool m_consoleOverride = false;
    bool m_showConsole = false;
    bool m_autoCloseConsole = false;
    bool m_showConsoleOnError = false;
};
