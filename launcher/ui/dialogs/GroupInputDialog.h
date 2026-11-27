#pragma once

#include <QDialog>
#include <QString>

class GroupInputDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString dialogTitle READ dialogTitle CONSTANT)
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QString textValue READ textValue WRITE setTextValue NOTIFY textValueChanged)
    Q_PROPERTY(bool canAccept READ canAccept NOTIFY canAcceptChanged)

public:
    explicit GroupInputDialog(const QString &title, const QString &label,
                              const QString &initialText = QString(),
                              QWidget *parent = nullptr);
    ~GroupInputDialog() override;

    QString dialogTitle() const { return m_title; }
    QString label() const { return m_label; }

    QString textValue() const { return m_textValue.trimmed(); }
    void setTextValue(const QString &text);

    bool canAccept() const { return !m_textValue.trimmed().isEmpty(); }

    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();

signals:
    void textValueChanged();
    void canAcceptChanged();

private:
    QString m_title;
    QString m_label;
    QString m_textValue;
};
