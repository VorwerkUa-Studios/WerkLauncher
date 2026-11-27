#pragma once

#include <QDialog>
#include <QString>
#include "net/NetJob.h"

enum UpdateAction
{
    UPDATE_LATER = QDialog::Rejected,
    UPDATE_NOW = QDialog::Accepted,
};

enum ChangelogType
{
    CHANGELOG_MARKDOWN,
    CHANGELOG_COMMITS
};

class UpdateDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool hasUpdate READ hasUpdate CONSTANT)
    Q_PROPERTY(QString headerText READ headerText CONSTANT)
    Q_PROPERTY(QString changelogHtml READ changelogHtml NOTIFY changelogHtmlChanged)

public:
    explicit UpdateDialog(bool hasUpdate = true, QWidget *parent = nullptr);
    ~UpdateDialog() override;

    bool hasUpdate() const { return m_hasUpdate; }
    QString headerText() const;
    QString changelogHtml() const { return m_changelogHtml; }

    Q_INVOKABLE void updateNow();
    Q_INVOKABLE void updateLater();

signals:
    void changelogHtmlChanged();

public slots:
    void loadChangelog();
    void changelogLoaded();
    void changelogFailed(QString reason);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool m_hasUpdate = true;
    QString m_changelogHtml;
    ChangelogType m_changelogType = CHANGELOG_COMMITS;
    NetJob::Ptr dljob;
    QByteArray changelogData;
};
