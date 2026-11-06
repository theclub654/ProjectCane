#include "installer.h"
#include "../installer/view.h"
#include "installer_assets.h"
#include "../installer/iso_extractor.h"
#include "graphics.h"
#include <glad/glad.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <switch.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_ONLY_PNG
#define STB_IMAGE_NO_STDIO
#include <stb/stb_image.h>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
namespace
{
struct LoadingBoost
{
    bool active = R_SUCCEEDED(appletSetCpuBoostMode(ApmCpuBoostMode_FastLoad));
    ~LoadingBoost() { if (active) appletSetCpuBoostMode(ApmCpuBoostMode_Normal); }
};
struct SharedFont
{
    PlFontData data{};
    bool open = false;
    SharedFont()
    {
        open = R_SUCCEEDED(plInitialize(PlServiceType_User));
        if (open && R_FAILED(plGetSharedFontByType(&data, PlSharedFontType_Standard))) data = {};
    }
    ~SharedFont() { if (open) plExit(); }
};
std::string Lower(std::string value)
{
    for (char& c : value) c = char(std::tolower(static_cast<unsigned char>(c)));
    return value;
}
struct Installer
{
    CaneSetup::View view;
    SharedFont font;
    PadState pad;
    fs::path folder = "sdmc:/";
    std::vector<fs::path> paths;
    GLuint background = 0;
    u64 heldDirection = 0;
    std::chrono::steady_clock::time_point repeatAt;
    Installer()
    {
        if (!CaneGraphics::Window()) CaneGraphics::Initialize();
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&pad);
        SetFont(24);
        int width, height;
        std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels(
            stbi_load_from_memory(CaneSetup::backgroundBytes, sizeof(CaneSetup::backgroundBytes), &width, &height, nullptr, 4), stbi_image_free);
        if (!pixels) throw std::runtime_error("Cannot load installer artwork");
        glGenTextures(1, &background);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, background);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
        Refresh();
    }
    ~Installer()
    {
        glDeleteTextures(1, &background);
        SetFont(13);
    }
    void SetFont(float pixels)
    {
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        auto& fonts = *ImGui::GetIO().Fonts;
        fonts.Clear();
        ImFontConfig config;
        config.SizePixels = pixels;
        if (pixels > 13 && font.data.address)
        {
            config.FontDataOwnedByAtlas = false;
            fonts.AddFontFromMemoryTTF(font.data.address, font.data.size, pixels, &config);
        }
        else fonts.AddFontDefault(&config);
        ImGui_ImplOpenGL3_CreateFontsTexture();
    }
    void Refresh()
    {
        std::vector<fs::directory_entry> entries;
        std::error_code error;
        for (fs::directory_iterator it(folder, fs::directory_options::skip_permission_denied, error), end;
             it != end && !error; it.increment(error))
        {
            const auto& item = *it;
            const std::string name = item.path().filename().string();
            if (name.empty() || name.front() == '.' || item.is_symlink(error)) continue;
            if (item.is_directory(error) || (item.is_regular_file(error) && Lower(item.path().extension().string()) == ".iso"))
                entries.push_back(item);
        }
        if (error) throw std::runtime_error("Cannot open this folder. Check the SD card.");
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            if (a.is_directory() != b.is_directory()) return a.is_directory();
            return Lower(a.path().filename().string()) < Lower(b.path().filename().string());
        });
        paths.clear(); view.entries.clear(); view.selected = view.firstVisible = 0;
        view.directory = "SD card";
        const auto relative = folder.lexically_relative(fs::path("sdmc:/"));
        if (!relative.empty() && relative != ".") view.directory += " / " + relative.generic_string();
        for (const auto& item : entries)
        {
            paths.push_back(item.path());
            view.entries.push_back({item.path().filename().string(), item.is_directory()});
        }
    }
    void Back()
    {
        if (folder == fs::path("sdmc:/")) return;
        folder = folder.parent_path();
        if (folder == fs::path("sdmc:")) folder = "sdmc:/";
        Refresh();
    }
    CaneSetup::Action Frame()
    {
        using namespace CaneSetup;
        glfwPollEvents();
        if (glfwWindowShouldClose(CaneGraphics::Window())) return Action::Exit;
        padUpdate(&pad);
        const u64 held = padGetButtons(&pad);
        u64 pressed = padGetButtonsDown(&pad);
        const u64 direction = held & (HidNpadButton_Up | HidNpadButton_Down | HidNpadButton_Left | HidNpadButton_Right);
        const auto now = std::chrono::steady_clock::now();
        if (direction != heldDirection)
        { heldDirection = direction; repeatAt = now + std::chrono::milliseconds(300); }
        else if (direction && now >= repeatAt)
        { pressed |= direction; repeatAt = now + std::chrono::milliseconds(100); }
        Action input = Action::None;
        if (pressed & HidNpadButton_Plus) input = Action::Exit;
        else if (view.screen == Screen::Browse)
        {
            if (!view.entries.empty())
            {
                if (pressed & HidNpadButton_Up) view.selected = (view.selected + view.entries.size() - 1) % view.entries.size();
                if (pressed & HidNpadButton_Down) view.selected = (view.selected + 1) % view.entries.size();
                if (pressed & HidNpadButton_A) input = Action::Select;
            }
            if (pressed & HidNpadButton_B) input = Action::Back;
        }
        else if (view.screen == Screen::Complete)
        {
            if (pressed & (HidNpadButton_Left | HidNpadButton_Right)) view.deleteIso = !view.deleteIso;
            if (pressed & HidNpadButton_A) input = view.deleteIso ? Action::DeleteIso : Action::KeepIso;
            if (pressed & HidNpadButton_B) input = Action::KeepIso;
        }
        else if (view.screen == Screen::Extract)
        {
            if (pressed & HidNpadButton_B) input = Action::Back;
        }
        else if (pressed & (HidNpadButton_A | HidNpadButton_B)) input = Action::Back;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int width, height;
        glfwGetFramebufferSize(CaneGraphics::Window(), &width, &height);
        glViewport(0, 0, width, height);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.03f, 0.05f, 0.09f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1280, 720);
        io.DisplayFramebufferScale = ImVec2(width / 1280.0f, height / 720.0f);
        ImGui::NewFrame();
        const auto clicked = Draw(view, reinterpret_cast<ImTextureID>(uintptr_t(background)));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(CaneGraphics::Window());
        return clicked != Action::None ? clicked : input;
    }
};
}

bool CaneSwitchInstall(const fs::path& directory)
{
    using namespace CaneSetup;
    Installer installer;
    fs::path selectedIso;
    bool exitRequested = false;
    if (CaneExtract::NeedsRebuild(directory))
    {
        LoadingBoost boost;
        installer.view.screen = Screen::Extract;
        CaneExtract::RebuildGame(directory, [&](const CaneExtract::Progress& progress) {
            installer.view.stage = progress.stage;
            installer.view.file = progress.file;
            installer.view.progress = progress.total ? float(progress.completed - 1) / progress.total : 0;
            return installer.Frame() != Action::Exit;
        });
        return CaneExtract::IsReady(directory);
    }
    while (true)
    {
        const auto action = installer.Frame();
        if (action == Action::Exit) return false;
        auto& view = installer.view;
        try
        {
            if (view.screen == Screen::Browse)
            {
                if (action == Action::Back) installer.Back();
                if (action != Action::Select || view.entries.empty()) continue;
                const auto path = installer.paths.at(view.selected);
                if (view.entries[view.selected].folder)
                { installer.folder = path; installer.Refresh(); continue; }
                selectedIso = path;
                view.screen = Screen::Extract;
                view.progress = 0;
                view.stage = "Checking your ISO";
                view.file = selectedIso.filename().string();
                exitRequested = false;
                {
                    LoadingBoost boost;
                    CaneExtract::ExtractGame(selectedIso, directory, [&](const CaneExtract::Progress& progress) {
                        view.progress = progress.total ? float(progress.completed) / float(progress.total) : 0;
                        view.stage = progress.stage;
                        view.file = progress.file;
                        const auto input = installer.Frame();
                        exitRequested = input == Action::Exit;
                        return input != Action::Exit && input != Action::Back;
                    });
                }
                view.gameReady = CaneExtract::IsReady(directory);
                if (!view.gameReady) throw std::runtime_error("Some game files are missing. Please run extraction again.");
                view.screen = Screen::Complete;
                view.deleteIso = false;
            }
            else if (view.screen == Screen::Complete)
            {
                if (action == Action::KeepIso) return true;
                if (action == Action::DeleteIso)
                {
                    if (!CaneExtract::IsReady(directory)) throw std::runtime_error("Game files are not ready; the ISO has been kept.");
                    std::error_code error;
                    fs::remove(selectedIso, error);
                    if (error) throw std::runtime_error("The ISO could not be deleted. Your game is ready and the ISO has been kept.");
                    return true;
                }
            }
            else if (view.screen == Screen::Error && action == Action::Back)
            {
                if (view.gameReady) return true;
                view.screen = Screen::Browse;
            }
        }
        catch (const CaneExtract::Cancelled&)
        {
            if (exitRequested) return false;
            view.error = "Extraction cancelled. Select your ISO again when you're ready.";
            view.screen = Screen::Error;
            view.gameReady = false;
        }
        catch (const std::exception& error)
        {
            view.error = error.what();
            view.screen = Screen::Error;
        }
    }
}
