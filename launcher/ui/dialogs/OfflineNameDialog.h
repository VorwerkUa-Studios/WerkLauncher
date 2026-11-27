#pragma once

#include <QDialog>
#include <QString>

class OfflineNameDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString textValue READ textValue WRITE setTextValue NOTIFY textValueChanged)
    Q_PROPERTY(bool canAccept READ canAccept NOTIFY canAcceptChanged)

public:
    enum OfflineNameDialogCode {
        Rejected = QDialog::Rejected,
        Accepted = QDialog::Accepted,
        OnlineRequested,
    };

    explicit OfflineNameDialog(QString current_name, QWidget *parent = nullptr);
    ~OfflineNameDialog() override;

    QString textValue() const { return m_textValue.trimmed(); }
    void setTextValue(const QString &text);

    bool canAccept() const { return !m_textValue.trimmed().isEmpty(); }

    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();
    Q_INVOKABLE void retryOnline();

signals:
    void textValueChanged();
    void canAcceptChanged();

private:
    QString m_textValue;
};
