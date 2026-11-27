#pragma once

#include <QString>
#include <QWidget>
#include "BaseInstance.h"
#include "ui/pages/BasePage.h"
#include <QObjectPtr.h>
#include "JavaCommon.h"

class InstanceJavaPage : public QWidget, public BasePage
{
    Q_OBJECT
    Q_PROPERTY(bool overrideMemory READ overrideMemory WRITE setOverrideMemory NOTIFY overrideChanged)
    Q_PROPERTY(bool overrideJavaLocation READ overrideJavaLocation WRITE setOverrideJavaLocation NOTIFY overrideChanged)
    Q_PROPERTY(bool overrideJvmArgs READ overrideJvmArgs WRITE setOverrideJvmArgs NOTIFY overrideChanged)
    Q_PROPERTY(int minMemory READ minMemory WRITE setMinMemory NOTIFY memoryChanged)
    Q_PROPERTY(int maxMemory READ maxMemory WRITE setMaxMemory NOTIFY memoryChanged)
    Q_PROPERTY(int permGen READ permGen WRITE setPermGen NOTIFY memoryChanged)
    Q_PROPERTY(int memoryMaximum READ memoryMaximum CONSTANT)
    Q_PROPERTY(QString javaPath READ javaPath WRITE setJavaPath NOTIFY javaPathChanged)
    Q_PROPERTY(QString jvmArgs READ jvmArgs WRITE setJvmArgs NOTIFY jvmArgsChanged)

public:
    explicit InstanceJavaPage(BaseInstance *inst, QWidget *parent = 0);
    virtual ~InstanceJavaPage();
    virtual QString displayName() const override
    {
        return tr("Java");
    }
    virtual QIcon icon() const override;
    virtual QString id() const override
    {
        return "java-settings";
    }
    virtual bool apply() override;
    virtual QString helpPage() const override
    {
        return "Instance-java-settings";
    }
    virtual bool shouldDisplay() const override;

    bool overrideMemory() const { return m_overrideMemory; }
    void setOverrideMemory(bool value);
    bool overrideJavaLocation() const { return m_overrideJavaLocation; }
    void setOverrideJavaLocation(bool value);
    bool overrideJvmArgs() const { return m_overrideJvmArgs; }
    void setOverrideJvmArgs(bool value);
    int minMemory() const { return m_minMemory; }
    void setMinMemory(int value);
    int maxMemory() const { return m_maxMemory; }
    void setMaxMemory(int value);
    int permGen() const { return m_permGen; }
    void setPermGen(int value);
    int memoryMaximum() const { return m_memoryMaximum; }
    QString javaPath() const { return m_javaPath; }
    void setJavaPath(const QString &path);
    QString jvmArgs() const { return m_jvmArgs; }
    void setJvmArgs(const QString &args);

    Q_INVOKABLE void browseJava();
    Q_INVOKABLE void detectJava();
    Q_INVOKABLE void testJava();

signals:
    void overrideChanged();
    void memoryChanged();
    void javaPathChanged();
    void jvmArgsChanged();

private:
    BaseInstance *m_instance;
    SettingsObjectPtr m_settings;
    unique_qobject_ptr<JavaCommon::TestCheck> m_checker;
    bool m_overrideMemory = false;
    bool m_overrideJavaLocation = false;
    bool m_overrideJvmArgs = false;
    int m_minMemory = 512;
    int m_maxMemory = 1024;
    int m_permGen = 64;
    int m_memoryMaximum = 65536;
    QString m_javaPath;
    QString m_jvmArgs;
};
