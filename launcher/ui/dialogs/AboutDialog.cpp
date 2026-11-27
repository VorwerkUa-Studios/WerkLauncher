#include "AboutDialog.h"
#include "Application.h"
#include "BuildConfig.h"
#include "HoeDown.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QQuickWidget>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

namespace {
QString getCreditsHtml(QStringList patrons) {
  QString output;
  QTextStream stream(&output);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec("UTF-8");
#endif
  stream << "<center>\n";
  stream << "<h3>" << QObject::tr("WerkLauncher Maintainers", "About Credits") << "</h3>\n";
  stream << "<p>Vorwerkua Studio</p>\n";
  stream << "<h3>" << QObject::tr("Original MultiMC Authors", "About Credits") << "</h3>\n";
  stream << "<p>Andrew Okin &lt;<a href='mailto:forkk@forkk.net'>forkk@forkk.net</a>&gt;</p>\n";
  stream << "<p>Petr Mr&aacute;zek &lt;<a href='mailto:peterix@gmail.com'>peterix@gmail.com</a>&gt;</p>\n";

  if (!patrons.isEmpty()) {
    stream << "<h3>" << QObject::tr("Patrons", "About Credits") << "</h3>\n";
    for (const QString &patron : patrons) {
      stream << "<p>" << patron << "</p>\n";
    }
  }

  stream << "</center>\n";
  return output;
}

QString getLicenseHtml() {
  HoeDown hoedown;
  QFile dataFile(":/documents/COPYING.md");
  (void)dataFile.open(QIODevice::ReadOnly);
  QString output = hoedown.process(dataFile.readAll());
  return output;
}
} // namespace

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("About %1").arg(BuildConfig.LAUNCHER_NAME));
  resize(540, 580);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("About %1").arg(BuildConfig.LAUNCHER_NAME));
  mainLayout->addWidget(titleBar);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("aboutDialogQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "AboutDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/AboutDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  GuiUtil::fitWindowToAvailableScreen(this, size());
}

AboutDialog::~AboutDialog() = default;

QString AboutDialog::launcherName() const {
  return BuildConfig.LAUNCHER_NAME;
}

QString AboutDialog::versionString() const {
  return BuildConfig.printableVersionString();
}

QString AboutDialog::platformString() const {
  return BuildConfig.BUILD_PLATFORM;
}

QString AboutDialog::channelString() const {
  return BuildConfig.VERSION_CHANNEL;
}

QString AboutDialog::creditsHtml() const {
  return getCreditsHtml(QStringList());
}

QString AboutDialog::licenseHtml() const {
  return getLicenseHtml();
}

QString AboutDialog::urlString() const {
  return BuildConfig.LAUNCHER_GIT;
}

void AboutDialog::showAboutQt() {
  QApplication::aboutQt();
}

void AboutDialog::closeDialog() {
  close();
}
