#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "InstanceList.h"
#include "settings/INISettingsObject.h"

class InstanceListTest : public QObject {
  Q_OBJECT

private:
  static bool createInstance(const QString &root, const QString &id) {
    const QString path = QDir(root).filePath(id);
    if (!QDir().mkpath(path)) {
      return false;
    }
    QFile config(QDir(path).filePath("instance.cfg"));
    return config.open(QFile::WriteOnly) &&
           config.write("[General]\nInstanceType=Unknown\n") > 0;
  }

private slots:
  void removesAndReloadsIdIndexes() {
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    const QString instances = QDir(temp.path()).filePath("instances");
    const QString settingsPath = QDir(temp.path()).filePath("global.ini");
    QVERIFY(QDir().mkpath(instances));
    QVERIFY(createInstance(instances, "first"));
    QVERIFY(createInstance(instances, "middle"));
    QVERIFY(createInstance(instances, "last"));

    auto settings = std::make_shared<INISettingsObject>(settingsPath);
    settings->registerSetting("PreLaunchCommand", "");
    settings->registerSetting("WrapperCommand", "");
    settings->registerSetting("PostExitCommand", "");
    settings->registerSetting("ShowConsole", false);
    settings->registerSetting("AutoCloseConsole", false);
    settings->registerSetting("ShowConsoleOnError", true);
    settings->registerSetting("LogPrePostOutput", true);
    settings->registerSetting("ConsoleMaxLines", 100000);
    settings->registerSetting("ConsoleOverflowStop", true);
    InstanceList list(settings, instances);
    QCOMPARE(list.loadList(), InstanceList::NoError);
    QCOMPARE(list.count(), 3);

    QVERIFY(QDir(QDir(instances).filePath("middle")).removeRecursively());
    QCOMPARE(list.loadList(), InstanceList::NoError);
    QCOMPARE(list.count(), 2);
    QVERIFY(!list.getInstanceById("middle"));
    QVERIFY(list.getInstanceById("first"));
    QVERIFY(list.getInstanceById("last"));
    QCOMPARE(list.getInstanceIndexById("last").row(), 1);

    QVERIFY(createInstance(instances, "middle"));
    QCOMPARE(list.loadList(), InstanceList::NoError);
    QCOMPARE(list.count(), 3);
    QVERIFY(list.getInstanceById("middle"));

    QVERIFY(QDir(QDir(instances).filePath("first")).removeRecursively());
    QVERIFY(QDir(QDir(instances).filePath("last")).removeRecursively());
    QCOMPARE(list.loadList(), InstanceList::NoError);
    QCOMPARE(list.count(), 1);
    QVERIFY(!list.getInstanceById("first"));
    QVERIFY(!list.getInstanceById("last"));
    QVERIFY(list.getInstanceById("middle"));
    QCOMPARE(list.getInstanceIndexById("middle").row(), 0);
  }

  void persistsNamedAndEmptyGroups() {
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    const QString instances = QDir(temp.path()).filePath("instances");
    const QString settingsPath = QDir(temp.path()).filePath("global.ini");
    QVERIFY(QDir().mkpath(instances));
    QVERIFY(createInstance(instances, "one"));

    auto settings = std::make_shared<INISettingsObject>(settingsPath);
    settings->registerSetting("PreLaunchCommand", "");
    settings->registerSetting("WrapperCommand", "");
    settings->registerSetting("PostExitCommand", "");
    settings->registerSetting("ShowConsole", false);
    settings->registerSetting("AutoCloseConsole", false);
    settings->registerSetting("ShowConsoleOnError", true);
    settings->registerSetting("LogPrePostOutput", true);
    settings->registerSetting("ConsoleMaxLines", 100000);
    settings->registerSetting("ConsoleOverflowStop", true);

    InstanceList list(settings, instances);
    QCOMPARE(list.loadList(), InstanceList::NoError);
    list.addGroup("Empty group");
    list.addGroup("Filled group");
    list.setInstanceGroup("one", "Filled group");

    InstanceList reloaded(settings, instances);
    QCOMPARE(reloaded.loadList(), InstanceList::NoError);
    const auto groups = reloaded.getGroups();
    QVERIFY(groups.contains("Empty group"));
    QVERIFY(groups.contains("Filled group"));
    QCOMPARE(reloaded.getInstanceGroup("one"), QString("Filled group"));
  }
};

QTEST_GUILESS_MAIN(InstanceListTest)

#include "InstanceList_test.moc"
