#include <QTest>

#include "minecraft/auth/SyncUrl.h"

class SyncUrlTest : public QObject {
  Q_OBJECT

private slots:
  void allowsHttpsAndLoopbackHttp() {
    QVERIFY(isSafeSyncUrl(QUrl("https://sync.example/api")));
    QVERIFY(isSafeSyncUrl(QUrl("http://localhost:5000/api")));
    QVERIFY(isSafeSyncUrl(QUrl("http://127.0.0.1:5000/api")));
    QVERIFY(isSafeSyncUrl(QUrl("http://[::1]:5000/api")));
  }

  void rejectsRemotePlainHttp() {
    QVERIFY(!isSafeSyncUrl(QUrl("http://sync.example/api")));
    QVERIFY(!isSafeSyncUrl(QUrl("http://0.0.0.0:5000/api")));
    QVERIFY(!isSafeSyncUrl(QUrl("ftp://localhost/api")));
  }
};

QTEST_GUILESS_MAIN(SyncUrlTest)

#include "SyncUrl_test.moc"
