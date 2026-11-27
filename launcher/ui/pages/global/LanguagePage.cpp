#include "LanguagePage.h"

#include <QLabel>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>

#include "Application.h"
#include "settings/SettingsObject.h"
#include "translations/TranslationsModel.h"

LanguagePage::LanguagePage(QWidget *parent)
    : QWidget(parent)
{
    loadSettings();

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("languagePageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The language settings page could not be loaded."), this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
    stack->addWidget(errorLabel);

    const auto showLoadError = [qmlWidget, stack, errorLabel]() {
        QStringList details;
        for (const auto &error : qmlWidget->errors()) {
            details.append(error.toString());
        }
        if (details.isEmpty()) {
            details.append(QObject::tr("No QML diagnostic was provided."));
        }
        qCritical() << "Language settings QML failed to load:"
                    << details.join('\n');
        errorLabel->setToolTip(details.join('\n'));
        stack->setCurrentWidget(errorLabel);
    };
    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget, showLoadError](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error ||
                    (status == QQuickWidget::Ready && !qmlWidget->rootObject())) {
                    showLoadError();
                }
            });
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/LanguagePage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

LanguagePage::~LanguagePage() = default;

bool LanguagePage::apply()
{
    applySettings();
    return true;
}

void LanguagePage::loadSettings()
{
    auto settings = APPLICATION->settings();
    m_selectedLanguage = settings->get("Language").toString();
    if (m_selectedLanguage.isEmpty()) {
        m_selectedLanguage = "en_US";
    }
}

void LanguagePage::applySettings()
{
    auto settings = APPLICATION->settings();
    settings->set("Language", m_selectedLanguage);
}

void LanguagePage::setSelectedLanguage(const QString &key)
{
    if (m_selectedLanguage != key) {
        m_selectedLanguage = key;
        auto translations = APPLICATION->translations();
        if (translations) {
            translations->selectLanguage(key);
            translations->updateLanguage(key);
        }
        emit selectedLanguageChanged();
    }
}
