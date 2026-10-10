#include "runtime.h"
#include "core_policy.h"
#include "io_worker.h"
#include "installer.h"
#include "../installer/iso_extractor.h"
#include "graphics.h"
#include <switch.h>
#include <filesystem>
#include <cstdio>
#include <unistd.h>

extern "C"
{
u32 __nx_applet_PerformanceConfiguration[2] = {0x00020004, 0};
}

void CaneSwitchError(const char* message)
{
    consoleInit(nullptr);
    std::printf("ProjectCane for Nintendo Switch\n\n%s\n\nPress + to return to the Homebrew Menu.\n", message);
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    while (appletMainLoop())
    {
        padUpdate(&pad);
        if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break;
        consoleUpdate(nullptr);
    }
    consoleExit(nullptr);
}

bool CaneSwitchStartup(int argc, char** argv)
{
    std::filesystem::path base("sdmc:/switch/ProjectCane");
    if (argc > 0 && argv[0] && *argv[0])
    {
        std::filesystem::path executable(argv[0]);
        std::error_code error;
        if (executable.has_parent_path() &&
            std::filesystem::is_directory(executable.parent_path(), error))
            base = executable.parent_path();
    }
    if (appletGetAppletType() != AppletType_Application &&
        appletGetAppletType() != AppletType_SystemApplication)
    {
        CaneSwitchError("Full application memory is required. Hold R while launching an installed game, then start ProjectCane from the Homebrew Menu.");
        return false;
    }
    std::error_code error;
    std::filesystem::create_directories(base, error);
    if (error || chdir(base.string().c_str()) != 0)
    {
        CaneSwitchError("Cannot open the game folder. Check the SD card.");
        return false;
    }
    CaneThreads::Initialize();
    CaneIo::Initialize();
    if (!CaneExtract::IsReady("."))
    {
        try
        {
            if (CaneSwitchInstall(".")) return true;
        }
        catch (const std::exception& exception)
        {
            if (CaneGraphics::Window()) CaneGraphics::Shutdown();
            CaneIo::Shutdown();
            CaneSwitchError(exception.what());
            return false;
        }
        if (CaneGraphics::Window()) CaneGraphics::Shutdown();
        CaneIo::Shutdown();
        return false;
    }
    return true;
}

void CaneSwitchShutdown()
{
    CaneIo::Shutdown();
}
