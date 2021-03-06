#include "pch.h"
#include "SystemSettings.h"
#include "Console.h"

const int DefaultScreenResolutionX = 1280;
const int DefaultScreenResolutionY = 900;

//////////////////////////////////////////////////////////////////////////

void SystemSettings::SetDefaults()
{
    mScreenDimensions.x = DefaultScreenResolutionX;
    mScreenDimensions.y = DefaultScreenResolutionY; 
    mFullscreen = false;
    mResizeableWindow = false;
    mEnableVSync = false;
}

void SystemSettings::SetFromJsonDocument(const cxx::json_document& sourceDocument)
{
    cxx::json_document_node rootNode = sourceDocument.get_root_node();

    // graphics settings
    cxx::json_get_attribute(rootNode, "screen_size_w", mScreenDimensions.x);
    cxx::json_get_attribute(rootNode, "screen_size_h", mScreenDimensions.y);
    cxx::json_get_attribute(rootNode, "set_fullscreen", mFullscreen);
    cxx::json_get_attribute(rootNode, "enable_vsync", mEnableVSync);
    cxx::json_get_attribute(rootNode, "resizeable_window", mResizeableWindow);
}

void SystemSettings::StoreToJsonDocument(cxx::json_document& sourceDocument)
{
    cxx::json_document_node rootNode = sourceDocument.get_root_node();

    // graphics settings
    sourceDocument.create_numeric_node(rootNode, "screen_size_w", mScreenDimensions.x);
    sourceDocument.create_numeric_node(rootNode, "screen_size_h", mScreenDimensions.y);
    sourceDocument.create_boolean_node(rootNode, "set_fullscreen", mFullscreen);
    sourceDocument.create_boolean_node(rootNode, "resizeable_window", mResizeableWindow);
    sourceDocument.create_boolean_node(rootNode, "enable_vsync", mEnableVSync);
}

//////////////////////////////////////////////////////////////////////////

bool SystemStartupParams::ParseStartupParams(int argc, char *argv[])
{
    Clear();

    for (int iarg = 0; iarg < argc; )
    {
        if (cxx_stricmp(argv[iarg], "-config") == 0 && (argc > iarg + 1))
        {
            mCustomConfigFileName.assign(argv[iarg + 1]);

            iarg += 2;
            continue;
        }

        if (cxx_stricmp(argv[iarg], "-gamedir") == 0 && (argc > iarg + 1))
        {
            mDungeonKeeperGamePath.assign(argv[iarg + 1]);

            iarg += 2;
            continue;
        }

        if (cxx_stricmp(argv[iarg], "-mapname") == 0 && (argc > iarg + 1))
        {
            mStartupMapName.assign(argv[iarg + 1]);

            iarg += 2;
            continue;
        }

        ++iarg;
    }

    return true;
}

void SystemStartupParams::Clear()
{
    mCustomConfigFileName.clear();
    mDungeonKeeperGamePath.clear();
    mStartupMapName.clear();
}
