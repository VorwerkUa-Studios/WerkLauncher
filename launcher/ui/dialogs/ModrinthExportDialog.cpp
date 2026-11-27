#include "ModrinthExportDialog.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QQuickWidget>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>

#include "BaseInstance.h"
#include "CustomMessageBox.h"
#include "ProgressDialog.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "modplatform/modrinth/ModrinthInstanceExportTask.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

ModrinthExportDialog::ModrinthExportDialog(InstancePtr instance, QWidget *parent)
    : QDialog(parent), m_instance(instance) {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setWindowTitle(tr("Export Modrinth modpack"));
    resize(640, 560);

    if (m_instance) {
        m_name = m_instance->name();
    }

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleBar = new CustomTitleBar(this);
    titleBar->setTitle(tr("Export Modrinth modpack"));
    mainLayout->addWidget(titleBar);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("modrinthExportDialogQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("dialogController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});

    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    for (const auto &err : qmlWidget->errors()) {
                        qCritical() << "ModrinthExportDialog QML error:" << err.toString();
                    }
                }
            });

    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/ModrinthExportDialog.qml")));
    mainLayout->addWidget(qmlWidget);

    GuiUtil::fitWindowToAvailableScreen(this, size());
}

ModrinthExportDialog::~ModrinthExportDialog() = default;

void ModrinthExportDialog::setName(const QString &val) {
    if (m_name != val) {
        m_name = val;
        emit nameChanged();
        emit canAcceptChanged();
    }
}

void ModrinthExportDialog::setVersion(const QString &val) {
    if (m_version != val) {
        m_version = val;
        emit versionChanged();
        emit canAcceptChanged();
    }
}

void ModrinthExportDialog::setDescription(const QString &val) {
    if (m_description != val) {
        m_description = val;
        emit descriptionChanged();
    }
}

void ModrinthExportDialog::setExportPath(const QString &val) {
    if (m_exportPath != val) {
        m_exportPath = val;
        emit exportPathChanged();
        emit canAcceptChanged();
    }
}

void ModrinthExportDialog::setIncludeGameConfig(bool val) {
    if (m_includeGameConfig != val) {
        m_includeGameConfig = val;
        emit includeGameConfigChanged();
    }
}

void ModrinthExportDialog::setIncludeModConfigs(bool val) {
    if (m_includeModConfigs != val) {
        m_includeModConfigs = val;
        emit includeModConfigsChanged();
    }
}

void ModrinthExportDialog::setIncludeResourcePacks(bool val) {
    if (m_includeResourcePacks != val) {
        m_includeResourcePacks = val;
        emit includeResourcePacksChanged();
    }
}

void ModrinthExportDialog::setIncludeShaderPacks(bool val) {
    if (m_includeShaderPacks != val) {
        m_includeShaderPacks = val;
        emit includeShaderPacksChanged();
    }
}

void ModrinthExportDialog::setTreatDisabledAsOptional(bool val) {
    if (m_treatDisabledAsOptional != val) {
        m_treatDisabledAsOptional = val;
        emit treatDisabledAsOptionalChanged();
    }
}

void ModrinthExportDialog::setIncludeDatapacks(bool val) {
    if (m_includeDatapacks != val) {
        m_includeDatapacks = val;
        emit includeDatapacksChanged();
        emit canAcceptChanged();
    }
}

void ModrinthExportDialog::setDatapacksPath(const QString &val) {
    if (m_datapacksPath != val) {
        m_datapacksPath = val;
        emit datapacksPathChanged();
        emit canAcceptChanged();
    }
}

bool ModrinthExportDialog::canAccept() const {
    if (m_name.trimmed().isEmpty() || m_version.trimmed().isEmpty() || !m_exportPath.endsWith(QStringLiteral(".mrpack"), Qt::CaseInsensitive)) {
        return false;
    }
    if (m_includeDatapacks) {
        if (m_datapacksPath.isEmpty() || !m_instance) {
            return false;
        }
        if (!QDir(m_instance->gameRoot() + "/" + m_datapacksPath).exists()) {
            return false;
        }
    }
    return true;
}

void ModrinthExportDialog::browseExportPath() {
    QFileDialog dialog(
        this, tr("Select modpack file"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setDefaultSuffix("mrpack");
    dialog.setNameFilter("Modrinth modpacks (*.mrpack)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.selectFile(m_name + ".mrpack");

    if (dialog.exec()) {
        auto files = dialog.selectedFiles();
        if (!files.isEmpty()) {
            setExportPath(files.at(0));
        }
    }
}

void ModrinthExportDialog::browseDatapacksPath() {
    if (!m_instance) return;
    QFileDialog dialog(this, tr("Select global datapacks folder"),
                       m_instance->gameRoot());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);

    if (dialog.exec()) {
        auto files = dialog.selectedFiles();
        if (!files.isEmpty()) {
            setDatapacksPath(QDir(m_instance->gameRoot()).relativeFilePath(files.at(0)));
        }
    }
}

void ModrinthExportDialog::acceptDialog() {
    if (!canAccept() || !m_instance) {
        return;
    }

    Modrinth::ExportSettings settings;
    settings.name = m_name;
    settings.version = m_version;
    settings.description = m_description;

    settings.includeGameConfig = m_includeGameConfig;
    settings.includeModConfigs = m_includeModConfigs;
    settings.includeResourcePacks = m_includeResourcePacks;
    settings.includeShaderPacks = m_includeShaderPacks;
    settings.treatDisabledAsOptional = m_treatDisabledAsOptional;

    if (m_includeDatapacks) {
        settings.datapacksPath = m_datapacksPath;
    }

    MinecraftInstancePtr minecraftInstance =
        std::dynamic_pointer_cast<MinecraftInstance>(m_instance);
    if (minecraftInstance) {
        minecraftInstance->getPackProfile()->reload(Net::Mode::Offline);

        for (int i = 0; i < minecraftInstance->getPackProfile()->rowCount(); i++) {
            auto component = minecraftInstance->getPackProfile()->getComponent(i);
            if (component && component->isCustom()) {
                CustomMessageBox::selectable(
                    this, tr("Warning"),
                    tr("Instance contains a custom component: %1\nThis cannot be "
                       "exported to a Modrinth pack; the exported pack may not work "
                       "correctly!")
                        .arg(component->getName()),
                    QMessageBox::Warning)
                    ->exec();
            }
        }

        settings.gameVersion =
            minecraftInstance->getPackProfile()->getComponentVersion("net.minecraft");
        settings.forgeVersion =
            minecraftInstance->getPackProfile()->getComponentVersion("net.minecraftforge");
        settings.fabricVersion =
            minecraftInstance->getPackProfile()->getComponentVersion("net.fabricmc.fabric-loader");
        settings.quiltVersion =
            minecraftInstance->getPackProfile()->getComponentVersion("org.quiltmc.quilt-loader");
        settings.neoforgeVersion =
            minecraftInstance->getPackProfile()->getComponentVersion("net.neoforged");
    }

    settings.exportPath = m_exportPath;

    auto *task = new Modrinth::InstanceExportTask(m_instance, settings);

    connect(task, &Task::failed, [this](QString reason) {
        QString text;
        if (reason.length() > 1000) {
            text = reason.left(1000) + "...";
        } else {
            text = reason;
        }
        CustomMessageBox::selectable(parentWidget(), tr("Error"), text,
                                     QMessageBox::Critical)
            ->show();
    });
    connect(task, &Task::succeeded, [this, task]() {
        QStringList warnings = task->warnings();
        if (warnings.count()) {
            CustomMessageBox::selectable(parentWidget(), tr("Warnings"),
                                         warnings.join('\n'), QMessageBox::Warning)
                ->show();
        }
    });

    ProgressDialog loadDialog(this);
    loadDialog.setSkipButton(true, tr("Abort"));
    loadDialog.execWithTask(task);

    QDialog::accept();
}

void ModrinthExportDialog::rejectDialog() {
    QDialog::reject();
}
