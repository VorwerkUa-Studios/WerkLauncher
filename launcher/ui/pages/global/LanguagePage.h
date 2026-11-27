#pragma once

#include <QWidget>
#include "ui/pages/BasePage.h"
#include <Application.h>

class LanguagePage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(QString selectedLanguage READ selectedLanguage WRITE setSelectedLanguage NOTIFY selectedLanguageChanged)

public:
    explicit LanguagePage(QWidget *parent = nullptr);
    ~LanguagePage() override;

    QString displayName() const override
    {
        return tr("Language");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("language");
    }
    QString id() const override
    {
        return "language-settings";
    }
    QString helpPage() const override
    {
        return "Language-settings";
    }
    bool apply() override;

    QString selectedLanguage() const { return m_selectedLanguage; }
    void setSelectedLanguage(const QString &key);

signals:
    void selectedLanguageChanged();

private:
    void loadSettings();
    void applySettings();

private:
    QString m_selectedLanguage;
};
