#pragma once

#include <QDialog>
#include "ExportInstanceDialog.h"

class ModrinthExportDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString exportPath READ exportPath WRITE setExportPath NOTIFY exportPathChanged)
    Q_PROPERTY(bool includeGameConfig READ includeGameConfig WRITE setIncludeGameConfig NOTIFY includeGameConfigChanged)
    Q_PROPERTY(bool includeModConfigs READ includeModConfigs WRITE setIncludeModConfigs NOTIFY includeModConfigsChanged)
    Q_PROPERTY(bool includeResourcePacks READ includeResourcePacks WRITE setIncludeResourcePacks NOTIFY includeResourcePacksChanged)
    Q_PROPERTY(bool includeShaderPacks READ includeShaderPacks WRITE setIncludeShaderPacks NOTIFY includeShaderPacksChanged)
    Q_PROPERTY(bool treatDisabledAsOptional READ treatDisabledAsOptional WRITE setTreatDisabledAsOptional NOTIFY treatDisabledAsOptionalChanged)
    Q_PROPERTY(bool includeDatapacks READ includeDatapacks WRITE setIncludeDatapacks NOTIFY includeDatapacksChanged)
    Q_PROPERTY(QString datapacksPath READ datapacksPath WRITE setDatapacksPath NOTIFY datapacksPathChanged)
    Q_PROPERTY(bool canAccept READ canAccept NOTIFY canAcceptChanged)

public:
    explicit ModrinthExportDialog(InstancePtr instance, QWidget *parent = nullptr);
    ~ModrinthExportDialog() override;

    QString name() const { return m_name; }
    void setName(const QString &val);

    QString version() const { return m_version; }
    void setVersion(const QString &val);

    QString description() const { return m_description; }
    void setDescription(const QString &val);

    QString exportPath() const { return m_exportPath; }
    void setExportPath(const QString &val);

    bool includeGameConfig() const { return m_includeGameConfig; }
    void setIncludeGameConfig(bool val);

    bool includeModConfigs() const { return m_includeModConfigs; }
    void setIncludeModConfigs(bool val);

    bool includeResourcePacks() const { return m_includeResourcePacks; }
    void setIncludeResourcePacks(bool val);

    bool includeShaderPacks() const { return m_includeShaderPacks; }
    void setIncludeShaderPacks(bool val);

    bool treatDisabledAsOptional() const { return m_treatDisabledAsOptional; }
    void setTreatDisabledAsOptional(bool val);

    bool includeDatapacks() const { return m_includeDatapacks; }
    void setIncludeDatapacks(bool val);

    QString datapacksPath() const { return m_datapacksPath; }
    void setDatapacksPath(const QString &val);

    bool canAccept() const;

    Q_INVOKABLE void browseExportPath();
    Q_INVOKABLE void browseDatapacksPath();
    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();

signals:
    void nameChanged();
    void versionChanged();
    void descriptionChanged();
    void exportPathChanged();
    void includeGameConfigChanged();
    void includeModConfigsChanged();
    void includeResourcePacksChanged();
    void includeShaderPacksChanged();
    void treatDisabledAsOptionalChanged();
    void includeDatapacksChanged();
    void datapacksPathChanged();
    void canAcceptChanged();

private:
    InstancePtr m_instance;
    QString m_name;
    QString m_version = QStringLiteral("1.0");
    QString m_description;
    QString m_exportPath;
    bool m_includeGameConfig = true;
    bool m_includeModConfigs = true;
    bool m_includeResourcePacks = false;
    bool m_includeShaderPacks = false;
    bool m_treatDisabledAsOptional = false;
    bool m_includeDatapacks = false;
    QString m_datapacksPath;
};
