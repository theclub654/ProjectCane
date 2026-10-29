// `cane_world_sweep units`: checks of game code that need the game's objects
// linked in but no game data, display or Startup.
//
// Custom options (option ids >= 1173, the scripts' AddO/EnsureO/SetO) live in
// a per-object sidebag allocated with std::make_shared. libc++ places the
// object of a make_shared block 8 bytes past a 16-byte boundary (its control
// block header is 24 bytes), and EnsureBasicSidebag once treated any sidebag
// that was not 16-byte aligned as corrupt and replaced it with an empty one.
// On macOS every option write after the first then wiped the object's
// earlier options (A Stealthy Approach: the van loses option 1178 when 1179
// is added, so its door never opens; theclub654/ProjectCane#61).

#include "sweep_child.h"
#include "sweep_display.h"
#include "sweep_game.h"
#include "sweep_sidebag.h"
#include "../background_app.h"

#include "../../Sly1-Release/basic.h"
#include "../../Sly1-Release/gl.h"
#include "../../Sly1-Release/mouse.h"
#include "../../Sly1-Release/shadow.h"
#include "../../Sly1-Release/sidebag.h"
#include "../../Sly1-Release/spliceobj.h"

#include <SDL.h>

#include <cstdlib>

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define CANE_UNITS_ASAN 1
#endif
#endif
#ifndef CANE_UNITS_ASAN
#define CANE_UNITS_ASAN 0
#endif

namespace
{
int g_failures = 0;
int g_checks = 0;

#define CHECK(condition) Check((condition), #condition, __FILE__, __LINE__)

void Check(bool condition, const char* expression, const char* file, int line)
{
    ++g_checks;
    if (condition)
        return;
    ++g_failures;
    std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", file, line, expression);
}

CRef S32(int value)
{
    CRef ref;
    ref.SetS32(value);
    return ref;
}

// The option's value, or -1 when the object does not hold it.
int OptionValue(BASIC* pbasic, int optid)
{
    const CRef ref = RefGetOption(pbasic, optid);
    return ref.m_tagk == TAGK_S32 ? ref.m_n : -1;
}

unsigned AddressMod16(const void* p)
{
    return static_cast<unsigned>(reinterpret_cast<uintptr_t>(p) & 15u);
}

constexpr int kOptionA = 1178; // the van's first custom option
constexpr int kOptionB = 1179;

// The van's sequence: two AddO on an object with no sidebag yet. The first
// allocates the sidebag through PsidebagNew (make_shared).
void TestAddTwoOptionsFromNoSidebag()
{
    VTBASIC vt;
    BASIC basic{};
    basic.pvtbasic = &vt;

    const int allocations = sweep_sidebag::Allocations();
    CRef a = S32(11);
    RefAddOption(&basic, kOptionA, &a);
    CSidebag* const first = basic.psidebag.get();
    CHECK(first != nullptr);
    // The sidebag came through the probe's PsidebagNew wrapper
    // (sidebag_probe.cpp), so the probe sees what the game allocates.
    CHECK(sweep_sidebag::Allocations() == allocations + 1);
    std::printf("units: first sidebag %p (address mod 16 = %u, alignof(CSidebag) = %zu)\n",
        static_cast<void*>(first), AddressMod16(first), alignof(CSidebag));

    CRef b = S32(22);
    RefAddOption(&basic, kOptionB, &b);
    CHECK(basic.psidebag.get() == first);
    CHECK(OptionValue(&basic, kOptionA) == 11);
    CHECK(OptionValue(&basic, kOptionB) == 22);
    CHECK(basic.psidebag != nullptr && basic.psidebag->m_csbb == 2);
    CHECK(sweep_sidebag::TakeRebuilds().empty());
}

// A make_shared sidebag that is not 16-byte aligned, through every write
// path (AddO, EnsureO on a new and an existing id, SetO).
void TestMisalignedMakeSharedSidebag()
{
    // Keep every candidate alive so the allocator cannot hand the same
    // block back.
    std::vector<std::shared_ptr<CSidebag>> candidates;
    std::shared_ptr<CSidebag> misaligned;
    const int allocations = sweep_sidebag::Allocations();
    for (int i = 0; i < 32 && misaligned == nullptr; ++i)
    {
        candidates.push_back(PsidebagNew());
        if (AddressMod16(candidates.back().get()) != 0)
            misaligned = candidates.back();
    }
    CHECK(sweep_sidebag::Allocations() == allocations + static_cast<int>(candidates.size()));
    // The precondition for this test to mean anything: libc++'s allocator
    // gives 8 mod 16, so without one this case would pass having checked
    // nothing. AddressSanitizer's allocator (the macos-asan build) may align
    // every block, and there the case is skipped.
    if (misaligned == nullptr)
    {
#if CANE_UNITS_ASAN
        std::printf("units: skipped the misaligned-sidebag case: AddressSanitizer's allocator gave no make_shared "
            "sidebag off 16-byte alignment in 32 tries\n");
#else
        std::printf("units: no make_shared sidebag off 16-byte alignment in 32 tries; the misaligned-sidebag case "
            "did not run\n");
        CHECK(misaligned != nullptr);
#endif
        return;
    }
    std::printf("units: misaligned make_shared sidebag %p (address mod 16 = %u)\n",
        static_cast<void*>(misaligned.get()), AddressMod16(misaligned.get()));
    // The old check (address & 0xf) rejects it; the type itself only needs
    // alignof(CSidebag), which must be smaller than 16 for the fix to differ.
    CHECK(alignof(CSidebag) < 16);
    CHECK(reinterpret_cast<uintptr_t>(misaligned.get()) % alignof(CSidebag) == 0);

    VTBASIC vt;
    BASIC basic{};
    basic.pvtbasic = &vt;
    basic.psidebag = misaligned;
    CSidebag* const sidebag = misaligned.get();

    CRef a = S32(1);
    RefAddOption(&basic, kOptionA, &a);
    CRef b = S32(2);
    RefEnsureOption(&basic, kOptionB, &b); // new id: adds
    CRef b2 = S32(3);
    RefEnsureOption(&basic, kOptionB, &b2); // existing id: sets
    CRef a2 = S32(4);
    RefSetOption(&basic, kOptionA, &a2);

    CHECK(basic.psidebag.get() == sidebag);
    CHECK(OptionValue(&basic, kOptionA) == 4);
    CHECK(OptionValue(&basic, kOptionB) == 3);
    CHECK(sidebag->m_csbb == 2);
    CHECK(sweep_sidebag::TakeRebuilds().empty());
}

// A pointer that really is invalid (an address in the first 64 KiB, no
// control block) is still replaced by an empty sidebag, and the probe does
// not count it: only sidebags the game allocated are "valid".
void TestInvalidPointerStillReplaced()
{
    VTBASIC vt;
    BASIC basic{};
    basic.pvtbasic = &vt;
    // Aliasing constructor: no control block, so nothing is ever freed
    // through it. EnsureBasicSidebag constructs over it without releasing.
    basic.psidebag = std::shared_ptr<CSidebag>(std::shared_ptr<void>(), reinterpret_cast<CSidebag*>(0x1008));

    CRef a = S32(7);
    RefAddOption(&basic, kOptionA, &a);
    CHECK(reinterpret_cast<uintptr_t>(basic.psidebag.get()) != 0x1008);
    CHECK(basic.psidebag != nullptr);
    CHECK(OptionValue(&basic, kOptionA) == 7);
    CHECK(sweep_sidebag::TakeRebuilds().empty());
}

// display-fullscreen on the built-in 2x panel (1512x982 points, 3024x1964
// pixels): a window covering the display has a 3024x1964 framebuffer. The
// sweep once required framebuffer == video mode, which compares pixels with
// points and fails for this correct window.
void TestFullscreenGeometryRetina()
{
    sweep_display::FullscreenGeometry g;
    g.modeWidth = g.windowWidth = 1512;
    g.modeHeight = g.windowHeight = 982;
    g.framebufferWidth = 3024;
    g.framebufferHeight = 1964;
    g.scaleX = g.scaleY = 2.0f;
    CHECK(sweep_display::WindowCoversMonitor(g));
    CHECK(sweep_display::FramebufferMatchesScale(g));
    CHECK(!(g.framebufferWidth == g.modeWidth && g.framebufferHeight == g.modeHeight)); // the old check

    // The external 1x monitor: points and pixels agree.
    sweep_display::FullscreenGeometry x;
    x.modeWidth = x.windowWidth = x.framebufferWidth = 5120;
    x.modeHeight = x.windowHeight = x.framebufferHeight = 1440;
    CHECK(sweep_display::WindowCoversMonitor(x));
    CHECK(sweep_display::FramebufferMatchesScale(x));

    // What GLFW's exclusive fullscreen left on that panel: a 1920x1200
    // framebuffer. Neither the window nor the framebuffer fits the display.
    sweep_display::FullscreenGeometry w5 = g;
    w5.windowWidth = w5.framebufferWidth = 1920;
    w5.windowHeight = w5.framebufferHeight = 1200;
    w5.scaleX = w5.scaleY = 1.0f;
    CHECK(!sweep_display::WindowCoversMonitor(w5));
    // A window off the monitor's origin does not cover it either.
    sweep_display::FullscreenGeometry offset = g;
    offset.windowY = 33;
    CHECK(!sweep_display::WindowCoversMonitor(offset));
    // A 1x framebuffer in a 2x window.
    sweep_display::FullscreenGeometry half = g;
    half.framebufferWidth = 1512;
    half.framebufferHeight = 982;
    CHECK(!sweep_display::FramebufferMatchesScale(half));
}

// The display mode before and after fullscreen: any switch is caught.
void TestDisplayModeUnchanged()
{
    sweep_display::DisplayMode panel{ true, 1512, 982, 3024, 1964, 120.0 };
    sweep_display::DisplayMode same = panel;
    CHECK(sweep_display::SameMode(panel, same));
    // The switch GLFW made on the panel (inferred from the 1920x1200 framebuffer).
    const sweep_display::DisplayMode switched{ true, 1920, 1200, 1920, 1200, 120.0 };
    CHECK(!sweep_display::SameMode(panel, switched));
    // Same points, other backing scale.
    const sweep_display::DisplayMode scale1{ true, 1512, 982, 1512, 982, 120.0 };
    CHECK(!sweep_display::SameMode(panel, scale1));
    // Same size, other refresh rate.
    sweep_display::DisplayMode hz60 = panel;
    hz60.refreshRate = 60.0;
    CHECK(!sweep_display::SameMode(panel, hz60));
    // A mode that could not be read never passes.
    CHECK(!sweep_display::SameMode(panel, sweep_display::DisplayMode{}));
}

// The test-process check counts every device SDL shows, not only game
// controllers: a plain joystick (here a virtual one; it is VID/PID
// 0000/0000, so the allow-list lets it through, and it has no controller
// mapping) is input the process could read too.
void TestVisibleInputDevices()
{
    // As in the sweep's children: never see the user's own controllers, and
    // stay in the background.
    setenv("SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT", "0x0000/0x0000", 1);
    cane_test::MakeBackgroundOnly();
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::printf("units: SDL joystick init failed (%s); input-device case not exercised\n", SDL_GetError());
        CHECK(false);
        return;
    }
    const int before = sweep_child::VisibleInputDevices();
    std::printf("units: input devices visible with the allow-list: %d\n", before);
    CHECK(before == 0);
    const int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_UNKNOWN, 2, 4, 0);
    CHECK(index >= 0);
    if (index >= 0)
    {
        CHECK(SDL_IsGameController(index) == SDL_FALSE);
        CHECK(sweep_child::VisibleInputDevices() == before + 1);
        SDL_JoystickDetachVirtual(index);
    }
    CHECK(sweep_child::VisibleInputDevices() == before);
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
}

// The tours press square through the game's own input layer (JOY::Update
// reads a mouse-bound button from MOUSE::Button, which GLFW's mouse-button
// callback sets), not through GLFW internals: while it is held the button is
// bound to a mouse button that is pressed, and the release puts the player's
// binding back.
void TestInjectButton()
{
    const int keyBinding = g_keyboardBindings[BTN_SQUARE];
    CHECK(!FMouseBinding(keyBinding)); // square is a key (J) by default
    sweep_child::InjectButton(BTN_SQUARE, true);
    const int held = g_keyboardBindings[BTN_SQUARE];
    CHECK(FMouseBinding(held));
    CHECK(FMouseBinding(held) && MOUSE::Button(MouseButtonFromBinding(held)));
    sweep_child::InjectButton(BTN_SQUARE, false);
    CHECK(g_keyboardBindings[BTN_SQUARE] == keyBinding);
    CHECK(FMouseBinding(held) && !MOUSE::Button(MouseButtonFromBinding(held)));

    // A button the player bound to a mouse button is pressed on it, and keeps it.
    const int mouseBinding = MouseBindingFromButton(GLFW_MOUSE_BUTTON_RIGHT);
    g_keyboardBindings[BTN_SQUARE] = mouseBinding;
    sweep_child::InjectButton(BTN_SQUARE, true);
    CHECK(g_keyboardBindings[BTN_SQUARE] == mouseBinding && MOUSE::Button(GLFW_MOUSE_BUTTON_RIGHT));
    sweep_child::InjectButton(BTN_SQUARE, false);
    CHECK(g_keyboardBindings[BTN_SQUARE] == mouseBinding && !MOUSE::Button(GLFW_MOUSE_BUTTON_RIGHT));
    g_keyboardBindings[BTN_SQUARE] = keyBinding;
}

// Before a level allocates its shadow buffers (and after DeallocateSwShadows)
// the shadow buffer names are 0. BindSwShadowTextures then still binds a
// texture on glob's two shadow buffer units, the units' placeholders, so the
// 4.1 driver never sees those samplers on empty units (DrawSw binds the
// placeholders first; this bind must not take them away again). Needs a GL
// 4.1 context: a hidden window in this background-only process.
void TestShadowUnitsWithoutShadowBuffers()
{
    cane_test::MakeBackgroundOnly();
    if (glfwInit() != GLFW_TRUE)
    {
        std::printf("units: glfwInit failed (no display?); shadow unit case not exercised\n");
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(64, 64, "cane_world_sweep units", nullptr, nullptr);
    if (window == nullptr)
    {
        std::printf("units: no OpenGL 4.1 context; shadow unit case not exercised\n");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    CHECK(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0);

    BindSwShadowTextures(nullptr);
    GLint shadowTexture = 0;
    GLint activeShadowTexture = 0;
    glActiveTexture(GL_TEXTURE0 + kShadowTextureUnit);
    glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &shadowTexture);
    glActiveTexture(GL_TEXTURE0 + kActiveShadowTextureUnit);
    glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &activeShadowTexture);
    glActiveTexture(GL_TEXTURE0);
    std::printf("units: shadow buffer units with no shadow buffers hold textures %d and %d\n",
        shadowTexture, activeShadowTexture);
    CHECK(shadowTexture != 0);
    CHECK(activeShadowTexture != 0);
    CHECK(static_cast<GLuint>(shadowTexture) == BufferTextureOrPlaceholder(kShadowTextureUnit, 0));
    CHECK(static_cast<GLuint>(activeShadowTexture) == BufferTextureOrPlaceholder(kActiveShadowTextureUnit, 0));
    CHECK(glGetError() == GL_NO_ERROR);

    DeletePlaceholderBufferTextures();
    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
}

namespace sweep_game
{
int RunUnits()
{
    TestAddTwoOptionsFromNoSidebag();
    TestMisalignedMakeSharedSidebag();
    TestInvalidPointerStillReplaced();
    TestFullscreenGeometryRetina();
    TestDisplayModeUnchanged();
    TestVisibleInputDevices();
    TestInjectButton();
    TestShadowUnitsWithoutShadowBuffers();
    std::printf("units: %d checks, %d failed\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
}
