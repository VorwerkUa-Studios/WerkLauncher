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

#include "InstanceSettingsPage.h"

#include <QLabel>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QQmlError>
#include <QVariant>

#include "Application.h"
#include "BaseInstance.h"

InstanceSettingsPage::InstanceSettingsPage(BaseInstance *inst, QWidget *parent)
    : QWidget(parent), m_instance(inst), m_settings(inst->settings()) {
    m_windowOverride = m_settings->get("OverrideWindow").toBool();
    m_launchMaximized = m_settings->get("LaunchMaximized").toBool();
    m_windowWidth = qBound(1, m_settings->get("MinecraftWinWidth").toInt(), 65536);
    m_windowHeight = qBound(1, m_settings->get("MinecraftWinHeight").toInt(), 65536);
    m_consoleOverride = m_settings->get("OverrideConsole").toBool();
    m_showConsole = m_settings->get("ShowConsole").toBool();
    m_autoCloseConsole = m_settings->get("AutoCloseConsole").toBool();
    m_showConsoleOnError = m_settings->get("ShowConsoleOnError").toBool();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *stack = new QStackedLayout;
    layout->addLayout(stack);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("instanceSettingsQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("settingsController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel = new QLabel(this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
    errorLabel->setText(tr("The instance settings page could not be loaded."));
    stack->addWidget(errorLabel);

    const auto showLoadError = [qmlWidget, stack, errorLabel]() {
        QStringList details;
        for (const auto &error : qmlWidget->errors()) {
            details.append(error.toString());
        }
        if (details.isEmpty()) {
            details.append(QObject::tr("No QML diagnostic was provided."));
        }
        qCritical() << "Instance settings QML failed to load:"
                    << details.join('\n');
        errorLabel->setToolTip(details.join('\n'));
        stack->setCurrentWidget(errorLabel);
    };

    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget, showLoadError](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error ||
                    (status == QQuickWidget::Ready &&
                     !qmlWidget->rootObject())) {
                    showLoadError();
                }
            });
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/InstanceSettings.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready &&
         !qmlWidget->rootObject())) {
        showLoadError();
    }
}

QString InstanceSettingsPage::displayName() const {
    return tr("Settings");
}

QIcon InstanceSettingsPage::icon() const {
    return APPLICATION->getThemedIcon("instance-settings");
}

QString InstanceSettingsPage::id() const {
    return QStringLiteral("settings");
}

bool InstanceSettingsPage::apply() {
    SettingsObject::Lock lock(m_settings);

    m_settings->set("OverrideWindow", m_windowOverride);
    if (m_windowOverride) {
        m_settings->set("LaunchMaximized", m_launchMaximized);
        m_settings->set("MinecraftWinWidth", m_windowWidth);
        m_settings->set("MinecraftWinHeight", m_windowHeight);
    } else {
        m_settings->reset("LaunchMaximized");
        m_settings->reset("MinecraftWinWidth");
        m_settings->reset("MinecraftWinHeight");
    }

    m_settings->set("OverrideConsole", m_consoleOverride);
    if (m_consoleOverride) {
        m_settings->set("ShowConsole", m_showConsole);
        m_settings->set("AutoCloseConsole", m_autoCloseConsole);
        m_settings->set("ShowConsoleOnError", m_showConsoleOnError);
    } else {
        m_settings->reset("ShowConsole");
        m_settings->reset("AutoCloseConsole");
        m_settings->reset("ShowConsoleOnError");
    }

    return true;
}

bool InstanceSettingsPage::shouldDisplay() const {
    return m_instance && !m_instance->isRunning();
}

void InstanceSettingsPage::setWindowOverride(bool value) {
    if (m_windowOverride == value) return;
    m_windowOverride = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setLaunchMaximized(bool value) {
    if (m_launchMaximized == value) return;
    m_launchMaximized = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setWindowWidth(int value) {
    value = qBound(1, value, 65536);
    if (m_windowWidth == value) return;
    m_windowWidth = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setWindowHeight(int value) {
    value = qBound(1, value, 65536);
    if (m_windowHeight == value) return;
    m_windowHeight = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setConsoleOverride(bool value) {
    if (m_consoleOverride == value) return;
    m_consoleOverride = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setShowConsole(bool value) {
    if (m_showConsole == value) return;
    m_showConsole = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setAutoCloseConsole(bool value) {
    if (m_autoCloseConsole == value) return;
    m_autoCloseConsole = value;
    emit settingsChanged();
}

void InstanceSettingsPage::setShowConsoleOnError(bool value) {
    if (m_showConsoleOnError == value) return;
    m_showConsoleOnError = value;
    emit settingsChanged();
}
