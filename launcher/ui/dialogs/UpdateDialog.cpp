#include "UpdateDialog.h"

#include <QDebug>
#include <QLabel>
#include <QQuickWidget>
#include <QRegularExpression>
#include <QUrl>
#include <QVBoxLayout>

#include "Application.h"
#include "BuildConfig.h"
#include "HoeDown.h"
#include "Json.h"
#include "settings/SettingsObject.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

namespace {
QString reprocessCommits(QByteArray json) {
  try {
    QString result;
    auto document = Json::requireDocument(json);
    auto rootobject = Json::requireObject(document);
    auto commitarray = Json::requireArray(rootobject, "commits");
    result += "<table cellspacing=0 cellpadding=2 style='border-width: 1px; border-style: solid'>";
    for (int i = commitarray.size() - 1; i >= 0; i--) {
      const auto &commitval = commitarray[i];
      auto commitobj = Json::requireValueObject(commitval);
      auto parents_info = Json::ensureArray(commitobj, "parents");
      if (parents_info.size() > 1) {
        continue;
      }
      auto commit_url = Json::requireString(commitobj, "html_url");
      auto commit_info = Json::requireObject(commitobj, "commit");
      auto commit_message = Json::requireString(commit_info, "message");
      auto lines = commit_message.split('\n');
      QRegularExpression regexp("(?<prefix>(GH-(?<issuenr>[0-9]+))|(NOISSUE)|(SCRATCH))? *(?<rest>.*) *");
      auto match = regexp.match(lines.takeFirst(), 0, QRegularExpression::NormalMatch);
      auto issuenr = match.captured("issuenr");
      auto rest = match.captured("rest");

      result += "<tr>";
      result += "<td valign=top>";
      if (!issuenr.isEmpty()) {
        result += QString("<a href=\"https://github.com/MultiMC/Launcher/issues/%1\">GH-%1</a>").arg(issuenr);
      }
      result += "</td>";
      result += "<td>";
      result += QString("<a href=\"%1\">%2</a>").arg(commit_url, rest.toHtmlEscaped());
      result += "</td>";
      result += "</tr>";
    }
    result += "</table>";
    return result;
  } catch (const JSONValidationError &e) {
    qCritical() << "Failed to parse update changelog JSON:" << e.cause();
    return QString();
  }
}
} // namespace

UpdateDialog::UpdateDialog(bool hasUpdate, QWidget *parent)
    : QDialog(parent), m_hasUpdate(hasUpdate) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Update"));
  resize(560, 520);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Update"));
  mainLayout->addWidget(titleBar);

  m_changelogHtml = tr("<center><h3>Loading changelog...</h3></center>");

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("updateDialogQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "UpdateDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/UpdateDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  loadChangelog();
  GuiUtil::fitWindowToAvailableScreen(this, size());
}

UpdateDialog::~UpdateDialog() = default;

QString UpdateDialog::headerText() const {
  if (m_hasUpdate) {
    return tr("A new update for %1 is available!").arg(BuildConfig.LAUNCHER_DISPLAYNAME);
  }
  return tr("No update found. You are running the latest version.");
}

void UpdateDialog::loadChangelog() {
  dljob = new NetJob("Changelog", APPLICATION->network());
  QString url = QString("https://api.github.com/repos/MultiMC/Launcher/compare/%1...develop").arg(BuildConfig.GIT_COMMIT);
  m_changelogType = CHANGELOG_COMMITS;
  dljob->addNetAction(Net::Download::makeByteArray(QUrl(url), &changelogData));
  connect(dljob.get(), &NetJob::succeeded, this, &UpdateDialog::changelogLoaded);
  connect(dljob.get(), &NetJob::failed, this, &UpdateDialog::changelogFailed);
  dljob->start();
}

void UpdateDialog::changelogLoaded() {
  QString result = reprocessCommits(changelogData);
  if (result.isEmpty()) {
    result = tr("<center><h3>No commits found.</h3></center>");
  }
  m_changelogHtml = result;
  emit changelogHtmlChanged();
  dljob.reset();
}

void UpdateDialog::changelogFailed(QString reason) {
  m_changelogHtml = tr("<center><h3>Failed to load changelog: %1</h3></center>").arg(reason);
  emit changelogHtmlChanged();
  dljob.reset();
}

void UpdateDialog::updateNow() {
  done(UPDATE_NOW);
}

void UpdateDialog::updateLater() {
  done(UPDATE_LATER);
}

void UpdateDialog::closeEvent(QCloseEvent *event) {
  APPLICATION->settings()->set("UpdateDialogGeometry", saveGeometry().toBase64());
  QDialog::closeEvent(event);
}
