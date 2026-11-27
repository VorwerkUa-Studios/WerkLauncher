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

#include <memory>
#include <QDialog>

#include "java/JavaChecker.h"
#include "ui/pages/BasePage.h"
#include <Application.h>

class SettingsObject;

class MinecraftPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(bool launchMaximized READ launchMaximized WRITE setLaunchMaximized NOTIFY launchMaximizedChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY windowSizeChanged)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY windowSizeChanged)
    Q_PROPERTY(bool useNativeOpenAL READ useNativeOpenAL WRITE setUseNativeOpenAL NOTIFY settingsChanged)
    Q_PROPERTY(bool useNativeGLFW READ useNativeGLFW WRITE setUseNativeGLFW NOTIFY settingsChanged)
    Q_PROPERTY(bool showGameTime READ showGameTime WRITE setShowGameTime NOTIFY settingsChanged)
    Q_PROPERTY(bool showGlobalGameTime READ showGlobalGameTime WRITE setShowGlobalGameTime NOTIFY settingsChanged)
    Q_PROPERTY(bool recordGameTime READ recordGameTime WRITE setRecordGameTime NOTIFY settingsChanged)
    Q_PROPERTY(bool showGameTimeHours READ showGameTimeHours WRITE setShowGameTimeHours NOTIFY settingsChanged)

public:
    explicit MinecraftPage(QWidget *parent = 0);
    ~MinecraftPage();

    QString displayName() const override
    {
        return tr("Minecraft");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("minecraft");
    }
    QString id() const override
    {
        return "minecraft-settings";
    }
    QString helpPage() const override
    {
        return "Minecraft-settings";
    }
    bool apply() override;

    bool launchMaximized() const { return m_launchMaximized; }
    void setLaunchMaximized(bool value);
    int windowWidth() const { return m_windowWidth; }
    void setWindowWidth(int value);
    int windowHeight() const { return m_windowHeight; }
    void setWindowHeight(int value);
    bool useNativeOpenAL() const { return m_useNativeOpenAL; }
    void setUseNativeOpenAL(bool value);
    bool useNativeGLFW() const { return m_useNativeGLFW; }
    void setUseNativeGLFW(bool value);
    bool showGameTime() const { return m_showGameTime; }
    void setShowGameTime(bool value);
    bool showGlobalGameTime() const { return m_showGlobalGameTime; }
    void setShowGlobalGameTime(bool value);
    bool recordGameTime() const { return m_recordGameTime; }
    void setRecordGameTime(bool value);
    bool showGameTimeHours() const { return m_showGameTimeHours; }
    void setShowGameTimeHours(bool value);

signals:
    void launchMaximizedChanged();
    void windowSizeChanged();
    void settingsChanged();

private:
    void applySettings();
    void loadSettings();

private:
    bool m_launchMaximized = false;
    int m_windowWidth = 854;
    int m_windowHeight = 480;
    bool m_useNativeOpenAL = false;
    bool m_useNativeGLFW = false;
    bool m_showGameTime = true;
    bool m_showGlobalGameTime = true;
    bool m_recordGameTime = true;
    bool m_showGameTimeHours = false;
};
