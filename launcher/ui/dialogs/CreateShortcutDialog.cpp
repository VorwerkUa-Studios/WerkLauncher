#ifdef _WIN32
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <windows.h>
#endif

#include "CreateShortcutDialog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QQuickWidget>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

#include "Application.h"
#include "BuildConfig.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "minecraft/VersionFilterData.h"
#include "minecraft/WorldList.h"
#include "minecraft/auth/AccountList.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

CreateShortcutDialog::CreateShortcutDialog(QWidget *parent, InstancePtr instance)
    : QDialog(parent), m_instance(instance) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Create Shortcut"));
  resize(520, 500);

  auto accounts = APPLICATION->accounts();
  for (int i = 0; i < accounts->count(); i++) {
    auto entry = accounts->at(i);
    if (entry.isAccount && entry.account) {
      m_availableProfiles.append(entry.account->profileName());
    }
  }

  if (accounts->defaultAccount()) {
    m_selectedProfile = accounts->defaultAccount()->profileName();
  } else if (!m_availableProfiles.isEmpty()) {
    m_selectedProfile = m_availableProfiles.first();
  }

  auto mcInstance = std::dynamic_pointer_cast<MinecraftInstance>(instance);
  if (mcInstance) {
    mcInstance->getPackProfile()->reload(Net::Mode::Online);
    auto comp = mcInstance->getPackProfile()->getComponent("net.minecraft");
    if (comp && comp->getReleaseDateTime() >= g_VersionFilterData.quickPlayBeginsDate) {
      m_instanceSupportsQuickPlay = true;
      mcInstance->worldList()->update();
      for (const auto &world : mcInstance->worldList()->allWorlds()) {
        m_singleplayerWorlds.append(world.folderName());
      }
      if (!m_singleplayerWorlds.isEmpty()) {
        m_selectedSingleplayerWorld = m_singleplayerWorlds.first();
      }
    }
  }

  QString linkExtension = "lnk";
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
  linkExtension = "desktop";
#elif defined(Q_OS_MAC)
  linkExtension = "command";
#endif

  m_shortcutPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" +
                   m_instance->name() + " - " + BuildConfig.LAUNCHER_DISPLAYNAME + "." + linkExtension;

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Create Shortcut"));
  mainLayout->addWidget(titleBar);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("createShortcutDialogQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "CreateShortcutDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/CreateShortcutDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  GuiUtil::fitWindowToAvailableScreen(this, size());
}

CreateShortcutDialog::~CreateShortcutDialog() = default;

bool CreateShortcutDialog::createScriptEnabled() const {
#if defined(Q_OS_UNIX) && !defined(Q_OS_LINUX)
  return false;
#else
  return true;
#endif
}

bool CreateShortcutDialog::canCreate() const {
  if (m_shortcutPath.trimmed().isEmpty()) {
    return false;
  }
  if (m_joinWorld) {
    if (m_joinServerMode && m_serverAddress.trimmed().isEmpty()) {
      return false;
    }
    if (!m_joinServerMode && m_selectedSingleplayerWorld.trimmed().isEmpty()) {
      return false;
    }
  }
  if (m_useProfile && m_selectedProfile.trimmed().isEmpty()) {
    return false;
  }
  if (m_launchOffline && m_setOfflineUsername && m_offlineUsername.trimmed().isEmpty()) {
    return false;
  }
  return true;
}

void CreateShortcutDialog::setShortcutPath(const QString &val) {
  if (m_shortcutPath != val) {
    m_shortcutPath = val;
    emit shortcutPathChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setJoinWorld(bool val) {
  if (m_joinWorld != val) {
    m_joinWorld = val;
    emit joinWorldChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setJoinServerMode(bool val) {
  if (m_joinServerMode != val) {
    m_joinServerMode = val;
    emit joinServerModeChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setServerAddress(const QString &val) {
  if (m_serverAddress != val) {
    m_serverAddress = val;
    emit serverAddressChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setSelectedSingleplayerWorld(const QString &val) {
  if (m_selectedSingleplayerWorld != val) {
    m_selectedSingleplayerWorld = val;
    emit selectedSingleplayerWorldChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setUseProfile(bool val) {
  if (m_useProfile != val) {
    m_useProfile = val;
    emit useProfileChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setSelectedProfile(const QString &val) {
  if (m_selectedProfile != val) {
    m_selectedProfile = val;
    emit selectedProfileChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setLaunchOffline(bool val) {
  if (m_launchOffline != val) {
    m_launchOffline = val;
    emit launchOfflineChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setSetOfflineUsername(bool val) {
  if (m_setOfflineUsername != val) {
    m_setOfflineUsername = val;
    emit setOfflineUsernameChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setOfflineUsername(const QString &val) {
  if (m_offlineUsername != val) {
    m_offlineUsername = val;
    emit offlineUsernameChanged();
    emit canCreateChanged();
  }
}

void CreateShortcutDialog::setCreateScript(bool val) {
  if (m_createScript != val) {
    m_createScript = val;
    emit createScriptChanged();
  }
}

void CreateShortcutDialog::browseShortcutPath() {
  QString linkExtension;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
  linkExtension = m_createScript ? "sh" : "desktop";
#elif defined(Q_OS_MAC)
  linkExtension = "command";
#elif defined(Q_OS_WIN)
  linkExtension = m_createScript ? "bat" : "lnk";
#endif

  QFileDialog fileDialog(
      this, tr("Select shortcut path"),
      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
  fileDialog.setDefaultSuffix(linkExtension);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile(m_instance->name() + " - " +
                        BuildConfig.LAUNCHER_DISPLAYNAME + "." + linkExtension);
  if (fileDialog.exec()) {
    setShortcutPath(fileDialog.selectedFiles().at(0));
  }
}

QString CreateShortcutDialog::getLaunchCommand(bool escapeQuotesTwice) {
  return "\"" +
         QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
             .replace('"', escapeQuotesTwice ? "\\\\\"" : "\\\"") +
         "\"" + getLaunchArgs(escapeQuotesTwice);
}

QString CreateShortcutDialog::getLaunchArgs(bool escapeQuotesTwice) {
  return " -d \"" +
         QDir::toNativeSeparators(QDir::currentPath())
             .replace('"', escapeQuotesTwice ? "\\\\\"" : "\\\"") +
         "\"" + " -l \"" + m_instance->id() + "\"" +
         (m_joinWorld && m_joinServerMode ? " -s \"" + m_serverAddress + "\"" : "") +
         (m_joinWorld && !m_joinServerMode ? " -w \"" + m_selectedSingleplayerWorld + "\"" : "") +
         (m_useProfile ? " -a \"" + m_selectedProfile + "\"" : "") +
         (m_launchOffline ? " -o" : "") +
         (m_launchOffline && m_setOfflineUsername ? " -n \"" + m_offlineUsername + "\"" : "");
}

void CreateShortcutDialog::createShortcut() {
#ifdef Q_OS_WIN
  if (m_createScript) {
    QString shortcutText = "@ECHO OFF\r\n"
                           "CD \"" +
                           QDir::toNativeSeparators(QDir::currentPath()) +
                           "\"\r\n"
                           "START /B \"\" " +
                           getLaunchCommand() + "\r\n";
    QFile shortcutFile(m_shortcutPath);
    if (shortcutFile.open(QIODevice::WriteOnly)) {
      QTextStream stream(&shortcutFile);
      stream << shortcutText;
      shortcutFile.close();
    }
  } else {
    if (!QFileInfo::exists(QDir::currentPath() + "/icons/shortcut-icon.ico")) {
      QPixmap iconPixmap = QIcon(":/logo.svg").pixmap(64, 64);
      iconPixmap.save(QDir::currentPath() + "/icons/shortcut-icon.ico");
    }

    createWindowsLink(
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()).toStdWString().c_str(),
        QDir::toNativeSeparators(QDir::currentPath()).toStdWString().c_str(),
        getLaunchArgs().toStdWString().c_str(),
        m_shortcutPath.toStdWString().c_str(),
        (m_instance->name() + " - " + BuildConfig.LAUNCHER_DISPLAYNAME).toStdWString().c_str(),
        QDir::toNativeSeparators(QDir::currentPath() + "/icons/shortcut-icon.ico").toStdWString().c_str());
  }
#else
  QString shortcutText;
  if (m_createScript) {
    shortcutText = "#!/bin/sh\n"
                   "cd \"" + QDir::currentPath().replace('"', "\\\"") + "\"\n" +
                   getLaunchCommand() + " &\n";
  } else {
    if (!QFileInfo::exists(QDir::currentPath() + "/icons/shortcut-icon.png")) {
      QPixmap iconPixmap = QIcon(":/logo.svg").pixmap(64, 64);
      iconPixmap.save(QDir::currentPath() + "/icons/shortcut-icon.png");
    }
    shortcutText = "[Desktop Entry]\nType=Application\nName=" +
                   m_instance->name() + " - " + BuildConfig.LAUNCHER_DISPLAYNAME + "\n" +
                   "Exec=" + getLaunchCommand(true) + "\n" +
                   "Path=" + QDir::currentPath() + "\nIcon=" +
                   QDir::currentPath() + "/icons/shortcut-icon.png\n";
  }
  QFile shortcutFile(m_shortcutPath);
  if (shortcutFile.open(QIODevice::WriteOnly)) {
    QTextStream stream(&shortcutFile);
    stream << shortcutText;
    shortcutFile.setPermissions(QFile::ReadOwner | QFile::ReadGroup |
                                QFile::ReadOther | QFile::WriteOwner |
                                QFile::ExeOwner | QFile::ExeGroup);
    shortcutFile.close();
  }
#endif
}

#ifdef Q_OS_WIN
void CreateShortcutDialog::createWindowsLink(LPCWSTR target, LPCWSTR workingDir,
                                             LPCWSTR args, LPCWSTR filename,
                                             LPCWSTR desc, LPCWSTR iconPath) {
  HRESULT result;
  IShellLink *link;

  CoInitialize(nullptr);
  result = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID *)&link);
  if (SUCCEEDED(result)) {
    IPersistFile *file;

    link->SetPath(target);
    link->SetWorkingDirectory(workingDir);
    link->SetArguments(args);
    link->SetDescription(desc);
    link->SetIconLocation(iconPath, 0);

    result = link->QueryInterface(IID_IPersistFile, (LPVOID *)&file);

    if (SUCCEEDED(result)) {
      file->Save(filename, TRUE);
      file->Release();
    }
    link->Release();
  }
  CoUninitialize();
}
#endif

void CreateShortcutDialog::createShortcutAction() {
  if (canCreate()) {
    createShortcut();
    accept();
  }
}

void CreateShortcutDialog::cancelAction() {
  reject();
}
