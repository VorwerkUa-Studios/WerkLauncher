#pragma once

#include <QDialog>
#include <QString>

class AboutDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString launcherName READ launcherName CONSTANT)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
    Q_PROPERTY(QString platformString READ platformString CONSTANT)
    Q_PROPERTY(QString channelString READ channelString CONSTANT)
    Q_PROPERTY(QString creditsHtml READ creditsHtml CONSTANT)
    Q_PROPERTY(QString licenseHtml READ licenseHtml CONSTANT)
    Q_PROPERTY(QString urlString READ urlString CONSTANT)

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

    QString launcherName() const;
    QString versionString() const;
    QString platformString() const;
    QString channelString() const;
    QString creditsHtml() const;
    QString licenseHtml() const;
    QString urlString() const;

    Q_INVOKABLE void showAboutQt();
    Q_INVOKABLE void closeDialog();
};
