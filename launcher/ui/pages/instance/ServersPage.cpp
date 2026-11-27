#include "ServersPage.h"

#include <FileSystem.h>
#include <io/stream_reader.h>
#include <minecraft/MinecraftInstance.h>
#include <sstream>
#include <tag_compound.h>
#include <tag_list.h>
#include <tag_primitive.h>
#include <tag_string.h>


#include <QFileSystemWatcher>
#include <QDebug>
#include <QLabel>
#include <QQmlError>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QSet>
#include <QUrl>

static const int COLUMN_COUNT = 2; // 3 , TBD: latency and other nice things.

struct Server {
  // Types
  enum class AcceptsTextures : int { ASK = 0, ALWAYS = 1, NEVER = 2 };

  // Methods
  Server() { m_name = QObject::tr("Minecraft Server"); }
  Server(const QString &name, const QString &address) {
    m_name = name;
    m_address = address;
  }
  Server(nbt::tag_compound &server) {
    std::string addressStr(server["ip"]);
    m_address = QString::fromUtf8(addressStr.c_str());

    std::string nameStr(server["name"]);
    m_name = QString::fromUtf8(nameStr.c_str());

    if (server["icon"]) {
      std::string base64str(server["icon"]);
      m_icon = QByteArray::fromBase64(base64str.c_str());
    }

    if (server.has_key("acceptTextures", nbt::tag_type::Byte)) {
      bool value = server["acceptTextures"].as<nbt::tag_byte>().get();
      if (value) {
        m_acceptsTextures = AcceptsTextures::ALWAYS;
      } else {
        m_acceptsTextures = AcceptsTextures::NEVER;
      }
    }
  }

  void serialize(nbt::tag_compound &server) {
    server.insert("name", m_name.trimmed().toUtf8().toStdString());
    server.insert("ip", m_address.trimmed().toUtf8().toStdString());
    if (m_icon.size()) {
      server.insert("icon", m_icon.toBase64().toStdString());
    }
    if (m_acceptsTextures != AcceptsTextures::ASK) {
      server.insert("acceptTextures", nbt::tag_byte(m_acceptsTextures ==
                                                    AcceptsTextures::ALWAYS));
    }
  }

  // Data - persistent and user changeable
  QString m_name;
  QString m_address;
  AcceptsTextures m_acceptsTextures = AcceptsTextures::ASK;

  // Data - persistent and automatically updated
  QByteArray m_icon;

  // Data - temporary
  bool m_checked = false;
  bool m_up = false;
  QString m_motd; // https://mctools.org/motd-creator
  int m_ping = 0;
  int m_currentPlayers = 0;
  int m_maxPlayers = 0;
};

static std::unique_ptr<nbt::tag_compound>
parseServersDat(const QString &filename) {
  try {
    QByteArray input = FS::read(filename);
    std::istringstream foo(std::string(input.constData(), input.size()));
    auto pair = nbt::io::read_compound(foo);

    if (pair.first != "")
      return nullptr;

    if (pair.second == nullptr)
      return nullptr;

    return std::move(pair.second);
  } catch (...) {
    return nullptr;
  }
}

static bool serializeServerDat(const QString &filename,
                               nbt::tag_compound *levelInfo) {
  try {
    if (!FS::ensureFilePathExists(filename)) {
      return false;
    }
    std::ostringstream s;
    nbt::io::write_tag("", *levelInfo, s);
    QByteArray val(s.str().data(), (int)s.str().size());
    FS::write(filename, val);
    return true;
  } catch (...) {
    return false;
  }
}

class ServersModel : public QAbstractListModel {
  Q_OBJECT
public:
  enum Roles {
    ServerPtrRole = Qt::UserRole,
  };
  explicit ServersModel(const QString &path, QObject *parent = 0)
      : QAbstractListModel(parent) {
    m_path = path;
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this,
            &ServersModel::fileChanged);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
            &ServersModel::dirChanged);
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(5000);
    connect(&m_saveTimer, &QTimer::timeout, this, &ServersModel::save_internal);
  }
  virtual ~ServersModel() {};

  void observe() {
    if (m_observed) {
      return;
    }
    m_observed = true;

    if (!m_loaded) {
      load();
    }

    updateFSObserver();
  }

  void unobserve() {
    if (!m_observed) {
      return;
    }
    m_observed = false;

    updateFSObserver();
  }

  void lock() {
    if (m_locked) {
      return;
    }
    saveNow();

    m_locked = true;
    updateFSObserver();
  }

  void unlock() {
    if (!m_locked) {
      return;
    }
    m_locked = false;

    updateFSObserver();
  }

  int addEmptyRow(int position) {
    if (m_locked) {
      return -1;
    }
    if (position < 0 || position >= rowCount()) {
      position = rowCount();
    }
    beginInsertRows(QModelIndex(), position, position);
    m_servers.insert(position, Server());
    endInsertRows();
    scheduleSave();
    return position;
  }

  bool removeRow(int row) {
    if (m_locked) {
      return false;
    }
    if (row < 0 || row >= rowCount()) {
      return false;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_servers.removeAt(row);
    endRemoveRows(); // does absolutely nothing, the selected server stays as
                     // the next line...
    scheduleSave();
    return true;
  }

  bool moveUp(int row) {
    if (m_locked) {
      return false;
    }
    if (row <= 0) {
      return false;
    }
    beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1);
    m_servers.swapItemsAt(row - 1, row);
    endMoveRows();
    scheduleSave();
    return true;
  }

  bool moveDown(int row) {
    if (m_locked) {
      return false;
    }
    int count = rowCount();
    if (row + 1 >= count) {
      return false;
    }
    beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2);
    m_servers.swapItemsAt(row + 1, row);
    endMoveRows();
    scheduleSave();
    return true;
  }

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override {
    if (section < 0 || section >= COLUMN_COUNT)
      return QVariant();

    if (role == Qt::DisplayRole) {
      switch (section) {
      case 0:
        return tr("Name");
      case 1:
        return tr("Address");
      case 2:
        return tr("Latency");
      }
    }

    return QAbstractListModel::headerData(section, orientation, role);
  }

  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override {
    if (!index.isValid())
      return QVariant();

    int row = index.row();
    int column = index.column();
    if (column < 0 || column >= COLUMN_COUNT)
      return QVariant();

    if (row < 0 || row >= m_servers.size())
      return QVariant();

    switch (column) {
    case 0:
      switch (role) {
      case Qt::DecorationRole: {
        auto &bytes = m_servers[row].m_icon;
        if (bytes.size()) {
          QPixmap px;
          if (px.loadFromData(bytes))
            return QIcon(px);
        }
        return APPLICATION->getThemedIcon("unknown_server");
      }
      case Qt::DisplayRole:
        return m_servers[row].m_name;
      case ServerPtrRole:
        return QVariant::fromValue<void *>((void *)&m_servers[row]);
      default:
        return QVariant();
      }
    case 1:
      switch (role) {
      case Qt::DisplayRole:
        return m_servers[row].m_address;
      default:
        return QVariant();
      }
    case 2:
      switch (role) {
      case Qt::DisplayRole:
        return m_servers[row].m_ping;
      default:
        return QVariant();
      }
    default:
      return QVariant();
    }
  }

  virtual int
  rowCount(const QModelIndex &parent = QModelIndex()) const override {
    return m_servers.size();
  }
  int columnCount(const QModelIndex &parent) const override {
    return COLUMN_COUNT;
  }

  Server *at(int index) {
    if (index < 0 || index >= rowCount()) {
      return nullptr;
    }
    return &m_servers[index];
  }

  void setName(int row, const QString &name) {
    if (m_locked) {
      return;
    }
    auto server = at(row);
    if (!server || server->m_name == name) {
      return;
    }
    server->m_name = name;
    emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));
    scheduleSave();
  }

  void setAddress(int row, const QString &address) {
    if (m_locked) {
      return;
    }
    auto server = at(row);
    if (!server || server->m_address == address) {
      return;
    }
    server->m_address = address;
    emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));
    scheduleSave();
  }

  void setAcceptsTextures(int row, Server::AcceptsTextures textures) {
    if (m_locked) {
      return;
    }
    auto server = at(row);
    if (!server || server->m_acceptsTextures == textures) {
      return;
    }
    server->m_acceptsTextures = textures;
    emit dataChanged(index(row, 0), index(row, COLUMN_COUNT - 1));
    scheduleSave();
  }

  void load() {
    cancelSave();
    beginResetModel();
    QList<Server> servers;
    auto serversDat = parseServersDat(serversPath());
    if (serversDat) {
      auto &serversList = serversDat->at("servers").as<nbt::tag_list>();
      for (auto iter = serversList.begin(); iter != serversList.end(); iter++) {
        auto &serverTag = (*iter).as<nbt::tag_compound>();
        Server s(serverTag);
        servers.append(s);
      }
    }
    m_servers.swap(servers);
    m_loaded = true;
    endResetModel();
  }

  void saveNow() {
    if (saveIsScheduled()) {
      save_internal();
    }
  }

public slots:
  void dirChanged(const QString &path) {
    qDebug() << "Changed:" << path;
    load();
  }
  void fileChanged(const QString &path) { qDebug() << "Changed:" << path; }

private slots:
  void save_internal() {
    cancelSave();
    QString path = serversPath();
    qDebug() << "Server list about to be saved to" << path;

    nbt::tag_compound out;
    nbt::tag_list list;
    for (auto &server : m_servers) {
      nbt::tag_compound serverNbt;
      server.serialize(serverNbt);
      list.push_back(std::move(serverNbt));
    }
    out.insert("servers", nbt::value(std::move(list)));

    if (!serializeServerDat(path, &out)) {
      qDebug() << "Failed to save server list:" << path << "Will try again.";
      scheduleSave();
    }
  }

private:
  void scheduleSave() {
    if (!m_loaded) {
      qDebug() << "Server list should never save if it didn't successfully "
                  "load, path:"
               << m_path;
      return;
    }
    if (!m_dirty) {
      m_dirty = true;
      qDebug() << "Server list save is scheduled for" << m_path;
    }
    m_saveTimer.start();
  }

  void cancelSave() {
    m_dirty = false;
    m_saveTimer.stop();
  }

  bool saveIsScheduled() const { return m_dirty; }

  void updateFSObserver() {
    bool observingFS = m_watcher->directories().contains(m_path);
    if (m_observed && m_locked) {
      if (!observingFS) {
        qWarning() << "Will watch" << m_path;
        if (!m_watcher->addPath(m_path)) {
          qWarning() << "Failed to start watching" << m_path;
        }
      }
    } else {
      if (observingFS) {
        qWarning() << "Will stop watching" << m_path;
        if (!m_watcher->removePath(m_path)) {
          qWarning() << "Failed to stop watching" << m_path;
        }
      }
    }
  }

  QString serversPath() {
    QFileInfo foo(FS::PathCombine(m_path, "servers.dat"));
    return foo.filePath();
  }

private:
  bool m_loaded = false;
  bool m_locked = false;
  bool m_observed = false;
  bool m_dirty = false;
  QString m_path;
  QList<Server> m_servers;
  QFileSystemWatcher *m_watcher = nullptr;
  QTimer m_saveTimer;
};

ServersPage::ServersPage(InstancePtr inst, QWidget *parent)
    : QWidget(parent), m_inst(std::move(inst)) {
  m_model = new ServersModel(m_inst->gameRoot(), this);
  m_locked = m_inst->isRunning();
  if (m_locked) {
    m_model->lock();
  }

  const auto publishChanges = [this]() { updateState(); };
  connect(m_model, &QAbstractItemModel::rowsInserted, this, publishChanges);
  connect(m_model, &QAbstractItemModel::rowsMoved, this, publishChanges);
  connect(m_model, &QAbstractItemModel::dataChanged, this, publishChanges);
  connect(m_model, &QAbstractItemModel::modelReset, this, [this]() {
    if (currentServer >= m_model->rowCount()) {
      currentServer = m_model->rowCount() - 1;
    }
    updateState();
  });
  connect(m_model, &QAbstractItemModel::rowsRemoved, this,
          &ServersPage::rowsRemoved);
  connect(m_inst.get(), &MinecraftInstance::runningStatusChanged, this,
          &ServersPage::onRunningStatusChanged);

  auto *stack = new QStackedLayout(this);
  stack->setContentsMargins(0, 0, 0, 0);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("serversPageQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("pageController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});
  stack->addWidget(qmlWidget);
  stack->setCurrentWidget(qmlWidget);

  auto *errorLabel = new QLabel(tr("The servers page could not be loaded."), this);
  errorLabel->setAlignment(Qt::AlignCenter);
  errorLabel->setWordWrap(true);
  stack->addWidget(errorLabel);

  const auto showLoadError = [qmlWidget, stack, errorLabel]() {
    QStringList details;
    for (const auto &error : qmlWidget->errors()) {
      details.append(error.toString());
    }
    if (details.isEmpty()) {
      details.append(QObject::tr("No QML diagnostic was provided."));
    }
    qCritical() << "Servers page QML failed to load:" << details.join('\n');
    errorLabel->setToolTip(details.join('\n'));
    stack->setCurrentWidget(errorLabel);
  };
  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget, showLoadError](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error ||
                (status == QQuickWidget::Ready &&
                 !qmlWidget->rootObject())) {
              showLoadError();
            }
          });
  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/ServersPage.qml")));
  if (qmlWidget->status() == QQuickWidget::Error ||
      (qmlWidget->status() == QQuickWidget::Ready &&
       !qmlWidget->rootObject())) {
    showLoadError();
  }

  updateState();
}

ServersPage::~ServersPage() { m_model->saveNow(); }

QVariantList ServersPage::servers() const {
  QVariantList result;
  for (int row = 0; row < m_model->rowCount(); ++row) {
    const auto *server = m_model->at(row);
    if (!server) {
      continue;
    }
    QVariantMap item;
    item.insert(QStringLiteral("row"), row);
    item.insert(QStringLiteral("name"), server->m_name);
    item.insert(QStringLiteral("address"), server->m_address);
    item.insert(QStringLiteral("acceptsTextures"),
                static_cast<int>(server->m_acceptsTextures));
    item.insert(QStringLiteral("selected"), row == currentServer);
    item.insert(QStringLiteral("iconUrl"),
                server->m_icon.isEmpty()
                    ? QString()
                    : QStringLiteral("data:image/png;base64,") +
                          QString::fromLatin1(server->m_icon.toBase64()));
    result.append(item);
  }
  return result;
}

QVariantMap ServersPage::selectedServer() const {
  const auto *server = m_model->at(currentServer);
  if (!server) {
    return {};
  }
  QVariantMap item;
  item.insert(QStringLiteral("row"), currentServer);
  item.insert(QStringLiteral("name"), server->m_name);
  item.insert(QStringLiteral("address"), server->m_address);
  item.insert(QStringLiteral("acceptsTextures"),
              static_cast<int>(server->m_acceptsTextures));
  return item;
}

QVariantMap ServersPage::actionStates() const {
  const bool selected = m_model->at(currentServer) != nullptr;
  QVariantMap result;
  result.insert(QStringLiteral("add"), !m_locked);
  result.insert(QStringLiteral("remove"), selected && !m_locked);
  result.insert(QStringLiteral("moveUp"), selected && !m_locked && currentServer > 0);
  result.insert(QStringLiteral("moveDown"),
                selected && !m_locked && currentServer + 1 < m_model->rowCount());
  result.insert(QStringLiteral("join"), selected && !m_locked);
  result.insert(QStringLiteral("edit"), selected && !m_locked);
  return result;
}

void ServersPage::selectServer(int row) {
  if (row < -1 || row >= m_model->rowCount()) {
    qWarning() << "Ignoring invalid server selection" << row;
    return;
  }
  if (currentServer == row) {
    return;
  }
  currentServer = row;
  updateState();
}

void ServersPage::setServerName(const QString &name) {
  m_model->setName(currentServer, name);
}

void ServersPage::setServerAddress(const QString &address) {
  m_model->setAddress(currentServer, address);
}

void ServersPage::setAcceptsTextures(int index) {
  if (index < static_cast<int>(Server::AcceptsTextures::ASK) ||
      index > static_cast<int>(Server::AcceptsTextures::NEVER)) {
    qWarning() << "Ignoring invalid server resource-pack preference" << index;
    return;
  }
  m_model->setAcceptsTextures(
      currentServer, static_cast<Server::AcceptsTextures>(index));
}

void ServersPage::triggerAction(const QString &actionName) {
  static const QSet<QString> supportedActions{
      QStringLiteral("add"),      QStringLiteral("remove"),
      QStringLiteral("moveUp"),   QStringLiteral("moveDown"),
      QStringLiteral("join")};
  if (!supportedActions.contains(actionName)) {
    qWarning() << "Ignoring unknown servers-page action" << actionName;
    return;
  }
  if (!actionStates().value(actionName).toBool()) {
    return;
  }
  if (actionName == QStringLiteral("add")) addServer();
  else if (actionName == QStringLiteral("remove")) removeServer();
  else if (actionName == QStringLiteral("moveUp")) moveServerUp();
  else if (actionName == QStringLiteral("moveDown")) moveServerDown();
  else if (actionName == QStringLiteral("join")) joinServer();
}

void ServersPage::updateState() {
  emit serversChanged();
  emit selectedServerChanged();
  emit actionStatesChanged();
}

void ServersPage::rowsRemoved(const QModelIndex &, int first, int last) {
  if (currentServer >= first && currentServer <= last) {
    currentServer = first < m_model->rowCount() ? first : m_model->rowCount() - 1;
  } else if (currentServer > last) {
    currentServer -= last - first + 1;
  }
  updateState();
}

void ServersPage::onRunningStatusChanged(bool running) {
  if (m_locked == running) {
    return;
  }
  m_locked = running;
  if (m_locked) {
    m_model->lock();
  } else {
    m_model->unlock();
  }
  updateState();
}

void ServersPage::openedImpl() { m_model->observe(); }

void ServersPage::closedImpl() { m_model->unobserve(); }

void ServersPage::addServer() {
  const int position = m_model->addEmptyRow(currentServer + 1);
  if (position >= 0) {
    currentServer = position;
    updateState();
  }
}

void ServersPage::removeServer() {
  m_model->removeRow(currentServer);
}

void ServersPage::moveServerUp() {
  if (m_model->moveUp(currentServer)) {
    --currentServer;
    updateState();
  }
}

void ServersPage::moveServerDown() {
  if (m_model->moveDown(currentServer)) {
    ++currentServer;
    updateState();
  }
}

void ServersPage::joinServer() {
  const auto *server = m_model->at(currentServer);
  if (!server) {
    return;
  }
  APPLICATION->launch(
      m_inst, true, nullptr,
      std::make_shared<QuickPlayTarget>(
          QuickPlayTarget::parseMultiplayer(server->m_address)));
}

#include "ServersPage.moc"
