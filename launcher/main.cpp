#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui/ImGuiFileDialog.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <atomic>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "backend.h"
#include "launcher_font.h"
#include "launcher_icon.h"
#include "launcher_logo.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

namespace {
struct Logo {
    GLuint texture = 0;
    int width = 0, height = 0;
    Logo() {
        auto pixels =
            stbi_load_from_memory(cane_logo, sizeof(cane_logo), &width, &height, nullptr, 4);
        if (!pixels) throw std::runtime_error("Cannot load launcher logo");
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixels);
        stbi_image_free(pixels);
    }
    void draw(float requestedHeight) const {
        auto available = ImGui::GetContentRegionAvail().x;
        auto h = std::min(requestedHeight, available * height / width);
        auto w = h * width / height;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available - w) * .5f);
        ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture)), {w, h});
    }
};
void windowIcon(GLFWwindow* window) {
    // Wayland delegates window/taskbar icons to the installed desktop entry.
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) return;
    GLFWimage icon{};
    icon.pixels =
        stbi_load_from_memory(cane_icon, sizeof(cane_icon), &icon.width, &icon.height, nullptr, 4);
    if (!icon.pixels) throw std::runtime_error("Cannot load launcher icon");
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
}
struct Job {
    std::atomic<bool> busy{false}, cancel{false};
    std::thread worker;
    std::mutex mutex;
    std::deque<std::string> lines;
    std::string message = "Ready", stage, item;
    float progress = 0;
    void start(const std::string& label, cane::LauncherBackend& backend,
               std::function<void(const cane::LogFunction&)> task) {
        if (busy) return;
        if (worker.joinable()) worker.join();
        cancel = false;
        {
            std::lock_guard lock(mutex);
            lines.clear();
            message = label;
            stage.clear();
            item.clear();
            progress = 0;
        }
        busy = true;
        worker = std::thread([this, &backend, task] {
            try {
                std::ofstream output(backend.locations.logs / "launcher.log", std::ios::app);
                auto log = [&](const std::string& line) {
                    output << line << '\n';
                    output.flush();
                    std::lock_guard lock(mutex);
                    try {
                        auto event = cane::json::parse(line);
                        if (event.contains("stage")) {
                            stage = event.at("stage");
                            item = event.value("item", std::string{});
                            auto total = event.value("total", 0.0f);
                            progress = total > 0 ? event.value("completed", 0.0f) / total : 0;
                        }
                    } catch (...) {
                    }
                    lines.push_back(line);
                    if (lines.size() > 300) lines.pop_front();
                };
                task(log);
                {
                    std::lock_guard lock(mutex);
                    message = "Completed";
                    progress = 1;
                }
            } catch (const std::exception& e) {
                std::lock_guard lock(mutex);
                message = e.what();
                lines.push_back(message);
            }
            busy = false;
        });
    }
    ~Job() {
        cancel = true;
        if (worker.joinable()) worker.join();
    }
};
std::string stageLabel(const std::string& stage) {
    static const std::unordered_map<std::string, std::string> labels = {
        {"validate", "Checking your disc"},     {"levels", "Preparing worlds"},
        {"music", "Preparing music"},           {"sound-banks", "Preparing sound effects"},
        {"commentary", "Preparing commentary"}, {"dialogue", "Preparing dialogue"},
        {"ambient", "Preparing ambient audio"}, {"cutscenes", "Preparing movies"},
        {"verify", "Checking installed files"}, {"complete", "Ready to play"},
        {"error", "Installation failed"}};
    auto found = labels.find(stage);
    return found == labels.end() ? stage : found->second;
}
void style() {
    auto& s = ImGui::GetStyle();
    s.WindowRounding = 0;
    s.ChildRounding = 14;
    s.FrameRounding = 8;
    s.GrabRounding = 8;
    s.FramePadding = {14, 12};
    s.ItemSpacing = {14, 14};
    s.WindowPadding = {24, 24};
    auto& c = s.Colors;
    c[ImGuiCol_WindowBg] = {.025f, .042f, .09f, 1};
    c[ImGuiCol_ChildBg] = {.055f, .08f, .15f, 1};
    c[ImGuiCol_FrameBg] = {.09f, .13f, .22f, 1};
    c[ImGuiCol_Button] = {.08f, .34f, .65f, 1};
    c[ImGuiCol_ButtonHovered] = {.1f, .46f, .86f, 1};
    c[ImGuiCol_ButtonActive] = {.08f, .3f, .58f, 1};
    c[ImGuiCol_Text] = {.92f, .94f, 1, 1};
    c[ImGuiCol_TextDisabled] = {.52f, .61f, .75f, 1};
    c[ImGuiCol_Header] = {.09f, .24f, .4f, 1};
    c[ImGuiCol_PlotHistogram] = {1, .68f, .22f, 1};
    c[ImGuiCol_Border] = {.14f, .2f, .31f, 1};
}
void screenshot(GLFWwindow* window, const cane::fs::path& file) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    std::vector<unsigned char> pixels(static_cast<size_t>(w) * h * 4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    stbi_flip_vertically_on_write(true);
    if (!stbi_write_png(cane::utf8(file).c_str(), w, h, 4, pixels.data(), w * 4))
        throw std::runtime_error("Could not save screenshot");
}
int desktop(cane::LauncherBackend& backend, const cane::fs::path& screenshotPath) {
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW " << code << ": " << message << '\n';
    });
    if (!glfwInit()) throw std::runtime_error("Cannot initialize desktop display");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "ProjectCane");
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "projectcane");
    auto window = glfwCreateWindow(1100, 720, "ProjectCane Launcher", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Cannot create launcher window");
    }
    glfwSetWindowSizeLimits(window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    windowIcon(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize launcher graphics");
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;
    ImFontConfig font;
    font.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(cane_font), sizeof(cane_font), 20,
                                   &font);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    style();
    Logo logo;
    Job job;
    char iso[4096]{};
    char source[4096]{};
    auto remembered = backend.rememberedIso();
    std::snprintf(iso, sizeof(iso), "%s", remembered.c_str());
    struct Drop {
        char* iso;
        Job* job;
    };
    Drop drop{iso, &job};
    glfwSetWindowUserPointer(window, &drop);
    glfwSetDropCallback(window, [](GLFWwindow* w, int count, const char** paths) {
        auto d = static_cast<Drop*>(glfwGetWindowUserPointer(w));
        if (count && !d->job->busy) std::snprintf(d->iso, 4096, "%s", paths[0]);
    });
    bool showLogs = false, showAdvanced = false, playing = false, installAfterSelection = false;
    int frames = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        bool busy = job.busy.load();
        if (!busy && job.worker.joinable()) {
            job.worker.join();
            playing = false;
        }
        bool ready = !busy && backend.installed();
        std::string message, stage, item;
        float progress;
        std::deque<std::string> lines;
        {
            std::lock_guard lock(job.mutex);
            message = job.message;
            stage = job.stage;
            item = job.item;
            progress = job.progress;
            if (showLogs) lines = job.lines;
        }
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)});
        ImGui::Begin("ProjectCane", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings);
        logo.draw(std::min(showAdvanced ? 130.0f
                           : showLogs   ? 180.0f
                                        : 300.0f,
                           static_cast<float>(height) * .4f));
        ImGui::BeginChild("installation", {0, showAdvanced ? 240.0f : 260.0f}, true);
        ImGui::PushStyleColor(ImGuiCol_Text, {1, .72f, .3f, 1});
        ImGui::TextUnformatted(busy    ? (playing ? "GAME RUNNING" : "PREPARING YOUR GAME")
                               : ready ? "READY TO PLAY"
                                       : "SLY 1  /  USA ISO");
        ImGui::PopStyleColor();
        if (!busy) {
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 160);
            ImGui::InputTextWithHint("##iso", "Drop an ISO here, or choose a file", iso,
                                     sizeof(iso));
            ImGui::SameLine();
            if (ImGui::Button("Choose ISO", {140, 0}))
                ImGuiFileDialog::Instance()->OpenDialog(
                    "iso", "Choose your Sly 1 ISO", ".iso,.ISO",
                    cane::utf8(cane::environmentDirectory("HOME", backend.locations.user)), "", 1,
                    nullptr, ImGuiFileDialogFlags_Modal);
            if (ImGui::Button(ready ? "Reinstall / repair" : "Install game", {210, 52})) {
                std::string selected = iso;
                if (selected.empty() || !cane::fs::is_regular_file(cane::path(selected))) {
                    installAfterSelection = true;
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "iso", "Choose your Sly 1 ISO to install or repair", ".iso,.ISO",
                        cane::utf8(cane::environmentDirectory("HOME", backend.locations.user)), "",
                        1, nullptr, ImGuiFileDialogFlags_Modal);
                } else
                    job.start("Preparing your game", backend, [&, selected](const auto& log) {
                        backend.install(selected, job.cancel, log);
                    });
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, {.85f, .52f, .1f, 1});
            ImGui::PushStyleColor(ImGuiCol_Text, {.03f, .06f, .1f, 1});
            ImGui::BeginDisabled(!ready);
            if (ImGui::Button("PLAY", {180, 52})) {
                playing = true;
                job.start("Game running", backend,
                          [&](const auto& log) { backend.play(job.cancel, log); });
            }
            ImGui::EndDisabled();
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
            ImGui::BeginDisabled(!ready);
            if (ImGui::Button("Verify files", {160, 52}))
                job.start("Checking installed files", backend, [&](const auto& log) {
                    cane::verifyInstallation(
                        backend.locations.data,
                        [&](size_t n, size_t total, const std::string& name) {
                            log(cane::json({{"stage", "verify"},
                                            {"completed", n},
                                            {"total", total},
                                            {"item", name}})
                                    .dump());
                        },
                        [&] { return job.cancel.load(); });
                });
            ImGui::EndDisabled();
        }
        if (busy) {
            ImGui::ProgressBar(progress, {-1, 12}, "");
            ImGui::Text("%s", stage.empty() ? message.c_str() : stageLabel(stage).c_str());
            if (!item.empty()) ImGui::TextWrapped("%s", item.c_str());
            if (ImGui::Button(playing ? "Stop game" : "Cancel")) job.cancel = true;
        } else {
            if (message != "Ready") ImGui::TextWrapped("%s", message.c_str());
            if (!ready && cane::fs::exists(backend.locations.data / "installation.json"))
                ImGui::TextWrapped(
                    "This installation needs repair or a compatible runtime. Select your ISO and "
                    "reinstall.");
        }
        ImGui::EndChild();
        ImGui::Checkbox("Show logs", &showLogs);
        ImGui::SameLine();
        ImGui::Checkbox("Build from source", &showAdvanced);
        ImGui::SameLine();
        if (ImGui::Button("Save folder")) {
            try {
                cane::openFolder(backend.locations.user / "Saves");
            } catch (const std::exception& e) {
                std::lock_guard lock(job.mutex);
                job.message = e.what();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Log folder")) {
            try {
                cane::openFolder(backend.locations.logs);
            } catch (const std::exception& e) {
                std::lock_guard lock(job.mutex);
                job.message = e.what();
            }
        }
        if (showAdvanced) {
            ImGui::BeginChild("source", {0, 200}, true);
            ImGui::TextWrapped(
                "Requires CMake and a C++20 compiler. Builds may download dependencies.");
            ImGui::BeginDisabled(busy);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 160);
            ImGui::InputTextWithHint("##source", "ProjectCane source checkout", source,
                                     sizeof(source));
            ImGui::SameLine();
            if (ImGui::Button("Choose folder", {140, 0}))
                ImGuiFileDialog::Instance()->OpenDialog("source", "Choose ProjectCane source",
                                                        nullptr, ".", "", 1, nullptr,
                                                        ImGuiFileDialogFlags_Modal);
            if (ImGui::Button("Build and use runtime")) {
                std::string selected = source;
                job.start("Building ProjectCane", backend, [&, selected](const auto& log) {
                    backend.buildSource(cane::path(selected), job.cancel, log);
                });
            }
            ImGui::SameLine();
            if (ImGui::Button("Use bundled runtime")) {
                backend.tools = cane::executableDirectory();
                backend.config.erase("runtime_dir");
                cane::writeJson(backend.locations.config / "launcher.json", backend.config);
            }
            ImGui::EndDisabled();
            ImGui::EndChild();
        }
        if (showLogs) {
            ImGui::BeginChild("logs", {0, std::max(150.0f, ImGui::GetContentRegionAvail().y)},
                              true);
            for (const auto& line : lines) ImGui::TextUnformatted(line.c_str());
            if (busy) ImGui::SetScrollHereY(1);
            ImGui::EndChild();
        }
        ImGui::End();
        for (auto key : {"iso", "source"})
            if (ImGuiFileDialog::Instance()->Display(key, ImGuiWindowFlags_NoCollapse, {650, 400},
                                                     {1000, 700})) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    auto selected = std::string(key) == "iso"
                                        ? ImGuiFileDialog::Instance()->GetFilePathName()
                                        : ImGuiFileDialog::Instance()->GetCurrentPath();
                    std::snprintf(std::string(key) == "iso" ? iso : source, 4096, "%s",
                                  selected.c_str());
                    if (std::string(key) == "iso" && installAfterSelection) {
                        job.start("Preparing your game", backend, [&, selected](const auto& log) {
                            backend.install(selected, job.cancel, log);
                        });
                    }
                }
                if (std::string(key) == "iso") installAfterSelection = false;
                ImGuiFileDialog::Instance()->Close();
            }
        ImGui::Render();
        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClearColor(.02f, .04f, .09f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (!screenshotPath.empty() && ++frames == 5) {
            screenshot(window, screenshotPath);
            glfwSetWindowShouldClose(window, true);
        }
        glfwSwapBuffers(window);
    }
    job.cancel = true;
    if (job.worker.joinable()) job.worker.join();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteTextures(1, &logo.texture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
}  // namespace
int main(int argc, char** argv) {
    try {
        cane::LauncherBackend backend;
        std::atomic<bool> cancel{false};
        auto log = [](const std::string& s) { std::cout << s << std::endl; };
        auto args = cane::arguments(argc, argv);
        if (!args.empty()) {
            if (args[0] == "--install" && args.size() == 2) {
                backend.install(args[1], cancel, log);
                return 0;
            }
            if (args[0] == "--verify" && args.size() == 1) {
                cane::verifyInstallation(backend.locations.data, {}, [&] { return cancel.load(); });
                std::cout << "Installation verified\n";
                return 0;
            }
            if (args[0] == "--play" && args.size() == 1) {
                backend.play(cancel, log);
                return 0;
            }
            if (args[0] == "--build-source" && args.size() == 2) {
                backend.buildSource(cane::path(args[1]), cancel, log);
                return 0;
            }
            if (args[0] == "--screenshot" && args.size() == 2)
                return desktop(backend, cane::path(args[1]));
            std::cout
                << "cane-launcher [--install ISO | --verify | --play | --build-source CHECKOUT]\n";
            return args[0] == "--help" ? 0 : 1;
        }
        return desktop(backend, {});
    } catch (const std::exception& e) {
        std::cerr << "ProjectCane Launcher: " << e.what() << std::endl;
#ifdef _WIN32
        MessageBoxW(nullptr, cane::path(e.what()).c_str(), L"ProjectCane Launcher",
                    MB_OK | MB_ICONERROR);
#endif
        return 1;
    }
}

#if defined(_WIN32) && defined(CANE_GUI_SUBSYSTEM)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return main(0, nullptr); }
#endif
