#pragma once

#include "ModFolderPage.h"

class ShaderPackPage : public ModFolderPage
{
    Q_OBJECT
public:
    explicit ShaderPackPage(MinecraftInstance *instance, QWidget *parent = 0)
        : ModFolderPage(instance, instance->shaderPackList(), "shaderpacks",
                        "shaderpacks", tr("Shader packs"), "Resource-packs", parent)
    {
        setShowConfigFolder(false);
    }
    virtual ~ShaderPackPage() {}

    virtual bool shouldDisplay() const override
    {
        return true;
    }
};
