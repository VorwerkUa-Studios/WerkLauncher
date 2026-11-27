#pragma once

#include <QDialog>
#include <QString>
#include "notifications/NotificationChecker.h"

class NotificationDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QString notificationType READ notificationType CONSTANT)
    Q_PROPERTY(int closeCountdown READ closeCountdown NOTIFY countdownChanged)
    Q_PROPERTY(int dontShowAgainCountdown READ dontShowAgainCountdown NOTIFY countdownChanged)

public:
    enum ExitCode
    {
        Normal,
        DontShowAgain
    };

    explicit NotificationDialog(const NotificationChecker::NotificationEntry &entry, QWidget *parent = nullptr);
    ~NotificationDialog() override;

    QString message() const { return m_message; }
    QString notificationType() const { return m_type; }
    int closeCountdown() const { return m_closeTime; }
    int dontShowAgainCountdown() const { return m_dontShowAgainTime; }

    Q_INVOKABLE void dismiss();
    Q_INVOKABLE void dontShowAgain();

signals:
    void countdownChanged();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    QString m_message;
    QString m_type;
    int m_dontShowAgainTime = 10;
    int m_closeTime = 5;
};
