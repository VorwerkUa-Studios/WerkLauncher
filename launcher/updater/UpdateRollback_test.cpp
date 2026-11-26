#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "updater/UpdateRollback.h"

class UpdateRollbackTest : public QObject {
  Q_OBJECT

private slots:
  void restoresBackupWhenInstallingReplacementFails() {
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    const QString original = QDir(temp.path()).filePath("app.bin");
    const QString backup = QDir(temp.path()).filePath("backup.bin");
    const QString update = QDir(temp.path()).filePath("update.bin");

    auto writeFile = [](const QString &path, const QByteArray &contents) {
      QFile file(path);
      if (!file.open(QFile::WriteOnly)) {
        return false;
      }
      return file.write(contents) == contents.size();
    };
    QVERIFY(writeFile(backup, "previous"));
    QVERIFY(writeFile(update, "replacement"));

    QVERIFY(restoreReplacementBackup(original, backup, update));

    QFile restored(original);
    QVERIFY(restored.open(QFile::ReadOnly));
    QCOMPARE(restored.readAll(), QByteArray("previous"));
    QVERIFY(!QFile::exists(backup));
    QVERIFY(QFile::exists(update));
  }
};

QTEST_GUILESS_MAIN(UpdateRollbackTest)

#include "UpdateRollback_test.moc"
