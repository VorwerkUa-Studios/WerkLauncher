/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <memory>

#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QTimer>

#include <QStackedWidget>
#include <QPushButton>
#include <QPixmap>
#include <QVariant>

#include "BaseInstance.h"
#include "minecraft/auth/MinecraftAccount.h"
#include "net/NetJob.h"
#include "updater/GoUpdate.h"

class LaunchController;
class NewsChecker;
class NotificationChecker;
class QToolButton;
class InstanceProxyModel;
class LabeledToolButton;
class QLabel;
class MinecraftLauncher;
class BaseProfilerFactory;
class InstanceView;
class KonamiCode;
class InstanceTask;
class CustomTitleBar;
class SidePanel;
class QLineEdit;
class QComboBox;
class QSystemTrayIcon;

class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_PROPERTY(QString accentColor READ getAccentColor NOTIFY accentColorChanged)
  Q_PROPERTY(QString themeName READ getThemeName NOTIFY accentColorChanged)
  Q_PROPERTY(QString accountIconUrl READ getAccountIconUrl NOTIFY accountIconChanged)
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged)

  class Ui;

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  Q_INVOKABLE QString getThemedIconUrl(const QString &name) const;
  Q_INVOKABLE bool hasStatisticsAction() const;
  Q_INVOKABLE bool hasDiscordAction() const;
  Q_INVOKABLE QString getAboutMenuLabel() const;
  QString getAccentColor() const;
  QString getThemeName() const;
  QString getAccountIconUrl() const { return m_accountIconUrl; }
  bool canGoBack() const;

  InstanceProxyModel *proxymodel = nullptr;

  bool eventFilter(QObject *obj, QEvent *ev) override;
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void handleTrayIconActivation(QSystemTrayIcon::ActivationReason reason);

protected:
  bool nativeEvent(const QByteArray &eventType, void *message,
                   qintptr *result) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

public slots:
  void checkInstancePathForProblems();

  void updatesAllowedChanged(bool allowed);

  void droppedURLs(QList<QUrl> urls);

signals:
  void isClosing();
  void isShowingSettingsChanged(bool isShowing);
  void accentColorChanged();
  void accountIconChanged();
  void navigationChanged();

protected:
  QMenu *createPopupMenu() override;

public slots:
  void onCatToggled(bool);

  void on_actionAbout_triggered();

  void on_actionAddInstance_triggered();

  void on_actionREDDIT_triggered();

  void on_actionDISCORD_triggered();

  void on_actionTELEGRAM_triggered();

  void on_actionCopyInstance_triggered();

  void on_actionChangeInstGroup_triggered();

  void on_actionChangeInstIcon_triggered();
  void on_changeIconButton_clicked(bool) {
    on_actionChangeInstIcon_triggered();
  }

  void on_actionViewInstanceFolder_triggered();

  void on_actionViewCentralModsFolder_triggered();

  void on_actionConfig_Folder_triggered();

  void on_actionViewSelectedInstFolder_triggered();

  void on_actionViewSelectedMCFolder_triggered();

  void on_actionViewSelectedModsFolder_triggered();

  void refreshInstances();



  void checkForUpdates();

  void on_actionSettings_triggered();

  void on_actionInstanceSettings_triggered();

  void on_actionManageAccounts_triggered();

  void on_actionDonatello_triggered();
  void on_actionStatistics_triggered();

  void on_actionPatreon_triggered();

  void on_actionMoreNews_triggered();

  void newsButtonClicked();

  void on_actionLaunchInstance_triggered();

  void on_actionLaunchInstanceOffline_triggered();

  void on_actionDeleteInstance_triggered();

  void deleteGroup();

  void on_actionExportInstance_triggered();

  void on_actionRenameInstance_triggered();
  void on_renameButton_clicked(bool) { on_actionRenameInstance_triggered(); }

  void on_actionEditInstance_triggered();

  void on_actionEditInstNotes_triggered();

  void on_actionMods_triggered();

  void on_actionWorlds_triggered();

  void on_actionScreenshots_triggered();

  void on_actionCreateShortcut_triggered();

  void taskEnd();

  /**
   * called when an icon is changed in the icon model.
   */
  void iconUpdated(QString);

  Q_INVOKABLE QString getWallpaper() const;
  Q_INVOKABLE QVariantList getInstanceGroups() const;
  Q_INVOKABLE int getSortModeIndex() const;
  Q_INVOKABLE void searchTextChanged(const QString &text);
  Q_INVOKABLE void sortModeChanged(int index);
  Q_INVOKABLE void setGroupCollapsed(const QString &group, bool collapsed);
  Q_INVOKABLE QVariantList getAccountMenuItems() const;
  Q_INVOKABLE bool hasDefaultAccount() const;
  Q_INVOKABLE void setDefaultAccount(int index);
  Q_INVOKABLE void addInstanceToGroup(const QString &group);
  Q_INVOKABLE void createGroupFromQml();
  Q_INVOKABLE void deleteGroupFromQml(const QString &group);
  Q_INVOKABLE void exportSelectedInstanceToModrinth();

  void updateToolsMenu();

  void instanceActivated(QModelIndex);

  void instanceChanged(const QModelIndex &current, const QModelIndex &previous);

  void instanceSelectRequest(QString id);

  void instanceDataChanged(const QModelIndex &topLeft,
                           const QModelIndex &bottomRight);

  void selectionBad();

public:
  void showInstanceView();
  Q_INVOKABLE void goBackToInstanceView() { showInstanceView(); }
  void showPage(QWidget *page);

  void startTask(Task *task);

  void updateAvailable(GoUpdate::Status status);

  void updateNotAvailable();

  void notificationsChanged();

  void defaultAccountChanged();

  void droppedURLsOnIndex(QList<QUrl> urls, QModelIndex index);

  void updateNewsLabel();

  /*!
   * Runs the DownloadTask and installs updates.
   */
  void downloadUpdates(GoUpdate::Status status);

  void konamiTriggered();

  void globalSettingsClosed();

private:
  void retranslateUi();

  void addInstance(QString url = QString(), QString forcedGroup = QString());
  void confirmDeleteGroup(const QString &group);
  void activateInstance(InstancePtr instance);
  void setCatBackground(bool enabled);
  void updateInstanceToolIcon(QString new_icon);
  
public slots:
  void setSelectedInstanceById(const QString &id);
private:
  void updateStatusCenter();

  void runModalTask(Task *task);
  void instanceFromInstanceTask(InstanceTask *task);
  void finalizeInstance(InstancePtr inst);

private:
  std::unique_ptr<Ui> ui;

  // these are managed by Qt's memory management model!
  InstanceView *view = nullptr;
  
  QToolButton *newsLabel = nullptr;
  QLabel *m_statusLeft = nullptr;
  QLabel *m_statusCenter = nullptr;
  KonamiCode *secretEventFilter = nullptr;

  CustomTitleBar *m_titleBar = nullptr;
  QStackedWidget *m_pageStack = nullptr;
  QPushButton *m_backButton = nullptr;
  QWidget *m_instanceViewPage = nullptr;
  QLineEdit *m_searchBar = nullptr;
  QComboBox *m_sortBox = nullptr;
  QSystemTrayIcon *m_trayIcon = nullptr;

  unique_qobject_ptr<NewsChecker> m_newsChecker;
  unique_qobject_ptr<NotificationChecker> m_notificationChecker;

  InstancePtr m_selectedInstance;
  QString m_currentInstIcon;

  // managed by the application object
  Task *m_versionLoadTask = nullptr;

  // Background image caching
  QPixmap m_cachedBackground;
  void updateCachedBackground();

  QString m_accountIconUrl;
};
