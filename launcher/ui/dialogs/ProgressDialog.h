#pragma once

#include <QDialog>
#include <memory>
#include <QString>

class Task;

class ProgressDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int progressValue READ progressValue NOTIFY progressChanged)
    Q_PROPERTY(int progressMaximum READ progressMaximum NOTIFY progressChanged)
    Q_PROPERTY(bool isIndeterminate READ isIndeterminate NOTIFY progressChanged)
    Q_PROPERTY(bool skipButtonVisible READ skipButtonVisible NOTIFY skipButtonChanged)
    Q_PROPERTY(QString skipButtonText READ skipButtonText NOTIFY skipButtonChanged)

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog() override;

    void updateSize();

    int execWithTask(Task *task);
    int execWithTask(std::unique_ptr<Task> &&task);
    int execWithTask(std::unique_ptr<Task> &task);

    void setSkipButton(bool present, QString label = QString());
    Task *getTask();

    QString statusText() const { return m_statusText; }
    int progressValue() const { return m_progressValue; }
    int progressMaximum() const { return m_progressMaximum; }
    bool isIndeterminate() const { return m_progressMaximum <= 0; }
    bool skipButtonVisible() const { return m_skipButtonVisible; }
    QString skipButtonText() const { return m_skipButtonLabel; }

    Q_INVOKABLE void abortTask();

public slots:
    void onTaskStarted();
    void onTaskFailed(QString failure);
    void onTaskSucceeded();

    void changeStatus(const QString &status);
    void changeProgress(qint64 current, qint64 total);

signals:
    void statusTextChanged();
    void progressChanged();
    void skipButtonChanged();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    bool handleImmediateResult(QDialog::DialogCode &result);

private:
    Task *task = nullptr;
    std::unique_ptr<Task> m_taskHolder;

    QString m_statusText;
    int m_progressValue = 0;
    int m_progressMaximum = 100;
    bool m_skipButtonVisible = false;
    QString m_skipButtonLabel;
};
