#include "Input.h"
#include "content.h"
float g_controllerDeadzone = 0.35f;
#include "clock.h"
#include "credit.h"
#include "pzo.h"
#include "sound.h"
#include "transition.h"
#ifdef _WIN32
#include "spu_reverb_xapo.h"
#endif
#include "ui.h"
#include "mb.h"
#ifdef __SWITCH__
#include "runtime.h"
#else
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif
#include <cstdio>
#include <iterator>
#include <vector>

namespace
{
constexpr uint16_t JBC_SELECT   = 0x0001;
constexpr uint16_t JBC_L3       = 0x0002;
constexpr uint16_t JBC_START    = 0x0008;
constexpr uint16_t JBC_UP       = 0x0010;
constexpr uint16_t JBC_RIGHT    = 0x0020;
constexpr uint16_t JBC_DOWN     = 0x0040;
constexpr uint16_t JBC_LEFT     = 0x0080;
constexpr uint16_t JBC_L2       = 0x0100;
constexpr uint16_t JBC_R2       = 0x0200;
constexpr uint16_t JBC_R1       = 0x0800;
constexpr uint16_t JBC_TRIANGLE = 0x1000;
constexpr uint16_t JBC_CIRCLE   = 0x2000;
constexpr uint16_t JBC_CROSS    = 0x4000;
constexpr uint16_t JBC_SQUARE   = 0x8000;

// Device selection should be stable around stick drift, but it must not pin a
// stale Steam Input/DirectInput device forever. A button reports activity 2.0,
// while axes report 0..1, so this epsilon only resolves near-equal samples.
constexpr float GAMEPAD_ACTIVITY_TIE_EPSILON = 0.05f;

CODE* s_pcode = nullptr;
float s_tCodeCheck = 0.0f;

uint16_t GrfbtnFromState(const std::unordered_map<JOY_BUTTON, bool>& state)
{
    uint16_t grfbtn = 0;
    const auto held = [&state](JOY_BUTTON button)
    {
        const auto it = state.find(button);
        return it != state.end() && it->second;
    };

    if (held(BTN_SELECT))   grfbtn |= JBC_SELECT;
    if (held(BTN_L3))       grfbtn |= JBC_L3;
    if (held(BTN_START))    grfbtn |= 0x0008;
    if (held(BTN_UP))       grfbtn |= JBC_UP;
    if (held(BTN_RIGHT))    grfbtn |= JBC_RIGHT;
    if (held(BTN_DOWN))     grfbtn |= JBC_DOWN;
    if (held(BTN_LEFT))     grfbtn |= JBC_LEFT;
    if (held(BTN_L2))       grfbtn |= JBC_L2;
    if (held(BTN_R2))       grfbtn |= 0x0200;
    if (held(BTN_L1))       grfbtn |= 0x0400;
    if (held(BTN_R1))       grfbtn |= JBC_R1;
    if (held(BTN_TRIANGLE)) grfbtn |= JBC_TRIANGLE;
    if (held(BTN_CIRCLE))   grfbtn |= JBC_CIRCLE;
    if (held(BTN_CROSS))    grfbtn |= JBC_CROSS;
    if (held(BTN_SQUARE))   grfbtn |= JBC_SQUARE;
    return grfbtn;
}

void CodeResetWorld(uint32_t nParam)
{
    g_transition.ResetWorld(static_cast<int>(nParam));
}

void CodeResetCheats(uint32_t)
{
    g_grfcht = 0;
    g_transition.ResetWorld(0);
}

void CodeSetCheatFlags(uint32_t nParam)
{
    g_grfcht |= static_cast<GRFCHT>(nParam & ~0x4000U);
    if ((nParam & 0x4000U) != 0)
        g_transition.ResetWorld(0);
}

void CodeCollectAllClues(uint32_t)
{
    if (g_psw != nullptr)
        CollectAllClues();
}

void CodeSetVaultFlags(uint32_t nParam)
{
    if (g_pgsCur != nullptr)
        g_pgsCur->grfvault = nParam;
}

void CompleteWorld(GAMEWORLD gameWorld)
{
    if (g_pgsCur == nullptr)
        return;

    WS& world = g_pgsCur->aws[static_cast<int>(gameWorld)];
    for (LS& level : world.als)
        level.grfls = static_cast<GRFLS>(level.grfls | 1U);
    world.fws |= 31;
}

void CodeCompleteAllWorlds(uint32_t)
{
    for (int i = 0; i < 6; ++i)
        CompleteWorld(static_cast<GAMEWORLD>(i));

    // Retail compares this field against the raw UIS_Hub value.
    if (g_ui.uisPlaying == UIS_Hub)
        g_transition.ResetWorld(0);
}

void CodeShowPassword(uint32_t)
{
    if (g_pgsCur == nullptr)
        return;

    const int worldLevel =
        (static_cast<int>(g_pgsCur->gameWorldCur) << 8) |
        static_cast<int>(g_pgsCur->worldLevelCur);

    if (worldLevel != 0x0400 || (GetGameProgress() & 6U) != 6U ||
        g_pgsCur->ccoin != 99 || g_pgsCur->clife != 0)
        return;

    char message[64]{};
    std::snprintf(message, sizeof(message), ContentUiText(
        "The password is: %s", "Le mot de passe est : %s", "Das Passwort lautet: %s",
        "La contraseña es: %s", "La password è: %s"), "chetkido");
    g_pnote.pvtblot->pfnSetBlotAchzDraw(&g_pnote, message);
    SetBlotDtVisible(&g_pnote, 10.0f);
    g_pnote.pvtblot->pfnShowBlot(&g_pnote);
}

constexpr uint16_t s_codeResetWorld[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_SELECT };
constexpr uint16_t s_codeResetCheats[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_START };
constexpr uint16_t s_codeCheat4[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_LEFT };
constexpr uint16_t s_codeCheat8[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_RIGHT };
constexpr uint16_t s_codeCheat2[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_CIRCLE };
constexpr uint16_t s_codeClues[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_UP };
constexpr uint16_t s_codeVaultFlags[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_DOWN };
constexpr uint16_t s_codeWorlds[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_TRIANGLE };
constexpr uint16_t s_codePassword[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_SQUARE, JBC_SQUARE, JBC_SQUARE };

CODE s_codes[] =
{
    { s_codeResetWorld,  static_cast<int>(std::size(s_codeResetWorld)),  CodeResetWorld,        0 },
    { s_codeResetCheats, static_cast<int>(std::size(s_codeResetCheats)), CodeResetCheats,       0 },
    { s_codeCheat4,      static_cast<int>(std::size(s_codeCheat4)),      CodeSetCheatFlags, 0x4004 },
    { s_codeCheat8,      static_cast<int>(std::size(s_codeCheat8)),      CodeSetCheatFlags,      8 },
    { s_codeCheat2,      static_cast<int>(std::size(s_codeCheat2)),      CodeSetCheatFlags,      2 },
    { s_codeClues,       static_cast<int>(std::size(s_codeClues)),       CodeCollectAllClues,    0 },
    { s_codeVaultFlags,  static_cast<int>(std::size(s_codeVaultFlags)),  CodeSetVaultFlags, 0xF000FFFF },
    { s_codeWorlds,      static_cast<int>(std::size(s_codeWorlds)),      CodeCompleteAllWorlds,  0 },
    { s_codePassword,    static_cast<int>(std::size(s_codePassword)),    CodeShowPassword,       0 }
};

constexpr std::array<int, BTN_MAX> kDefaultKeyboardBindings =
{
    GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    GLFW_KEY_J, GLFW_KEY_L, GLFW_KEY_K, GLFW_KEY_I,
    GLFW_KEY_ENTER, GLFW_KEY_BACKSPACE,
    GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_Z, GLFW_KEY_C,
    GLFW_KEY_UNKNOWN, GLFW_KEY_UNKNOWN
};

constexpr std::array<int, BTN_MAX> kDefaultGamepadBindings =
{
    GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
    GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
    GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_B,
    GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_Y,
    GLFW_GAMEPAD_BUTTON_START, GLFW_GAMEPAD_BUTTON_BACK,
    GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
    GAMEPAD_BINDING_LEFT_TRIGGER, GAMEPAD_BINDING_RIGHT_TRIGGER,
    GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
};

float GamepadActivity(const GLFWgamepadstate& state)
{
    float activity = 0.0f;

    for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button)
    {
        if (state.buttons[button] == GLFW_PRESS)
            activity = (std::max)(activity, 2.0f);
    }

    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]));
    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]));
    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]));
    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]));

    // GLFW trigger axes rest at -1. Convert them to a zero-to-one range so
    // an untouched trigger does not make every connected pad look active.
    activity = (std::max)(activity,
        (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f);
    activity = (std::max)(activity,
        (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) * 0.5f);

    return activity;
}

void InitializeGamepadState(GLFWgamepadstate* state)
{
    *state = {};
    // GLFW's standardized trigger convention rests at -1.  A zero-initialized
    // trigger otherwise looks half pressed to activity selection.
    state->axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;
    state->axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;
}

#ifndef __SWITCH__
struct SdlControllerDevice
{
    SDL_GameController* controller = nullptr;
    SDL_JoystickID instanceId = -1;
};

std::vector<SdlControllerDevice> s_sdlControllers;
double s_nextControllerScan = 0.0;
#endif

// The PS2 used a binary high-speed motor and an 8-bit low-speed motor. These
// patterns retain that representation so the retail rumble scheduler can be
// used unchanged while SDL performs the platform-specific actuator call.
constexpr RUMINS s_ruminsSteadyBuzz[] = { { 0.08f, 1, 72 } };
constexpr RUMINS s_ruminsLowThrob[] =
{
    { 0.10f, 0, 150 }, { 0.12f, 0, 0 }
};
constexpr RUMINS s_ruminsMediumThrob[] =
{
    { 0.10f, 1, 175 }, { 0.10f, 0, 20 }
};
constexpr RUMINS s_ruminsHardThrob[] =
{
    { 0.12f, 1, 255 }, { 0.08f, 0, 40 }
};
constexpr RUMINS s_ruminsBlunt[] =
{
    { 0.07f, 1, 255 }, { 0.08f, 0, 90 }, { 0.10f, 0, 0 }
};
constexpr RUMINS s_ruminsElectric[] =
{
    { 0.035f, 1, 80 }, { 0.035f, 0, 210 }, { 0.035f, 1, 130 }, { 0.035f, 0, 40 }
};
constexpr RUMINS s_ruminsFire[] =
{
    { 0.06f, 1, 95 }, { 0.07f, 0, 155 }, { 0.05f, 1, 55 }
};
constexpr RUMINS s_ruminsWater[] =
{
    { 0.12f, 0, 85 }, { 0.10f, 0, 145 }, { 0.12f, 0, 55 }
};
constexpr RUMINS s_ruminsCrush[] =
{
    { 0.16f, 1, 255 }, { 0.10f, 1, 190 }, { 0.10f, 0, 80 }
};
constexpr RUMINS s_ruminsBreak[] =
{
    { 0.06f, 1, 255 }, { 0.05f, 0, 35 }, { 0.08f, 1, 185 }, { 0.10f, 0, 0 }
};
constexpr RUMINS s_ruminsBomb[] =
{
    { 0.14f, 1, 255 }, { 0.12f, 1, 210 }, { 0.16f, 0, 125 }, { 0.12f, 0, 0 }
};

constexpr RUMPAT s_arumpat[RUMK_Max] =
{
    { s_ruminsSteadyBuzz,  static_cast<int>(std::size(s_ruminsSteadyBuzz)) },
    { s_ruminsLowThrob,    static_cast<int>(std::size(s_ruminsLowThrob)) },
    { s_ruminsMediumThrob, static_cast<int>(std::size(s_ruminsMediumThrob)) },
    { s_ruminsHardThrob,   static_cast<int>(std::size(s_ruminsHardThrob)) },
    { s_ruminsBlunt,       static_cast<int>(std::size(s_ruminsBlunt)) },
    { s_ruminsElectric,    static_cast<int>(std::size(s_ruminsElectric)) },
    { s_ruminsFire,        static_cast<int>(std::size(s_ruminsFire)) },
    { s_ruminsWater,       static_cast<int>(std::size(s_ruminsWater)) },
    { s_ruminsCrush,       static_cast<int>(std::size(s_ruminsCrush)) },
    { s_ruminsBreak,       static_cast<int>(std::size(s_ruminsBreak)) },
    { s_ruminsBomb,        static_cast<int>(std::size(s_ruminsBomb)) }
};

#ifndef __SWITCH__
float NormalizeSdlAxis(Sint16 value)
{
    return value < 0
        ? static_cast<float>(value) / 32768.0f
        : static_cast<float>(value) / 32767.0f;
}

float NormalizeSdlTrigger(Sint16 value)
{
    const float normalized = static_cast<float>((std::max)(0, static_cast<int>(value))) / 32767.0f;
    return normalized * 2.0f - 1.0f;
}

void CloseSdlController(SdlControllerDevice& device)
{
    if (device.controller != nullptr)
    {
        SDL_GameControllerRumble(device.controller, 0, 0, 0);
        SDL_GameControllerClose(device.controller);
    }
    device.controller = nullptr;
    device.instanceId = -1;
}

SDL_GameController* PcontrollerFromInstanceId(int instanceId)
{
    const auto it = std::find_if(s_sdlControllers.begin(), s_sdlControllers.end(),
        [instanceId](const SdlControllerDevice& device)
        {
            return device.instanceId == instanceId;
        });
    return it != s_sdlControllers.end() ? it->controller : nullptr;
}

#endif

void SetSdlRumble(const JOY* pjoy, uint8_t highSpeedMotor, uint8_t lowSpeedMotor)
{
    if (pjoy == nullptr || !pjoy->gamepadConnected)
        return;

#ifdef __SWITCH__
    CaneSwitchRumble(lowSpeedMotor / 255.0f, highSpeedMotor != 0 ? 1.0f : 0.0f);
#else
    SDL_GameController* controller = PcontrollerFromInstanceId(pjoy->joystickId);
    if (controller == nullptr || SDL_GameControllerHasRumble(controller) != SDL_TRUE)
        return;

    const Uint16 lowFrequency = static_cast<Uint16>(lowSpeedMotor * 257U);
    const Uint16 highFrequency = highSpeedMotor != 0 ? static_cast<Uint16>(0xFFFF) : 0;
    // Each pattern transition refreshes this timeout. It is deliberately
    // finite so vibration cannot remain stuck if the game stops updating.
    SDL_GameControllerRumble(controller, lowFrequency, highFrequency, 1000);
#endif
}

#ifndef __SWITCH__
void RefreshSdlControllers(bool forceScan)
{
    SDL_PumpEvents();
    SDL_GameControllerUpdate();

    for (auto it = s_sdlControllers.begin(); it != s_sdlControllers.end();)
    {
        if (it->controller == nullptr || !SDL_GameControllerGetAttached(it->controller))
        {
            CloseSdlController(*it);
            it = s_sdlControllers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    const double now = glfwGetTime();
    if (!forceScan && now < s_nextControllerScan)
        return;
    s_nextControllerScan = now + 0.5;

    const int joystickCount = SDL_NumJoysticks();
    for (int deviceIndex = 0; deviceIndex < joystickCount; ++deviceIndex)
    {
        if (!SDL_IsGameController(deviceIndex))
            continue;

        const SDL_JoystickID instanceId = SDL_JoystickGetDeviceInstanceID(deviceIndex);
        const bool alreadyOpen = std::any_of(s_sdlControllers.begin(), s_sdlControllers.end(),
            [instanceId](const SdlControllerDevice& device)
            {
                return device.instanceId == instanceId;
            });
        if (alreadyOpen)
            continue;

        SDL_GameController* controller = SDL_GameControllerOpen(deviceIndex);
        if (controller == nullptr)
            continue;

        SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
        s_sdlControllers.push_back({ controller, SDL_JoystickInstanceID(joystick) });
    }
}

bool GetSdlControllerState(SDL_GameController* controller, GLFWgamepadstate* state)
{
    if (controller == nullptr || state == nullptr || !SDL_GameControllerGetAttached(controller))
        return false;

    InitializeGamepadState(state);
    const auto button = [controller](SDL_GameControllerButton value)
    {
        return SDL_GameControllerGetButton(controller, value) != 0 ? GLFW_PRESS : GLFW_RELEASE;
    };

    state->buttons[GLFW_GAMEPAD_BUTTON_A] = button(SDL_CONTROLLER_BUTTON_A);
    state->buttons[GLFW_GAMEPAD_BUTTON_B] = button(SDL_CONTROLLER_BUTTON_B);
    state->buttons[GLFW_GAMEPAD_BUTTON_X] = button(SDL_CONTROLLER_BUTTON_X);
    state->buttons[GLFW_GAMEPAD_BUTTON_Y] = button(SDL_CONTROLLER_BUTTON_Y);
    state->buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] = button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    state->buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    state->buttons[GLFW_GAMEPAD_BUTTON_BACK] = button(SDL_CONTROLLER_BUTTON_BACK);
    state->buttons[GLFW_GAMEPAD_BUTTON_START] = button(SDL_CONTROLLER_BUTTON_START);
    state->buttons[GLFW_GAMEPAD_BUTTON_GUIDE] = button(SDL_CONTROLLER_BUTTON_GUIDE);
    state->buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] = button(SDL_CONTROLLER_BUTTON_LEFTSTICK);
    state->buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] = button(SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    state->buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] = button(SDL_CONTROLLER_BUTTON_DPAD_UP);
    state->buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    state->buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = button(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    state->buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = button(SDL_CONTROLLER_BUTTON_DPAD_LEFT);

    state->axes[GLFW_GAMEPAD_AXIS_LEFT_X] = NormalizeSdlAxis(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX));
    state->axes[GLFW_GAMEPAD_AXIS_LEFT_Y] = NormalizeSdlAxis(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY));
    state->axes[GLFW_GAMEPAD_AXIS_RIGHT_X] = NormalizeSdlAxis(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX));
    state->axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] = NormalizeSdlAxis(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY));
    state->axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] =
        NormalizeSdlTrigger(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
    state->axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] =
        NormalizeSdlTrigger(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
    return true;
}

#endif

const char* PchzKeyboardKeyNameInternal(int key)
{
    switch (key)
    {
    case GLFW_KEY_UNKNOWN: return ContentUiText("Unbound", "Non assigné", "Nicht belegt", "Sin asignar", "Non assegnato");
    case GLFW_KEY_SPACE: return ContentUiText("Space", "Espace", "Leertaste", "Espacio", "Spazio");
    case GLFW_KEY_ENTER: return ContentUiText("Enter", "Entrée", "Eingabe", "Intro", "Invio");
    case GLFW_KEY_TAB: return "Tab";
    case GLFW_KEY_BACKSPACE: return ContentUiText("Backspace", "Retour arrière", "Rücktaste", "Retroceso", "Backspace");
    case GLFW_KEY_LEFT_SHIFT: return ContentUiText("Left Shift", "Maj gauche", "Umschalt links", "Mayús izquierdo", "Maiusc sinistro");
    case GLFW_KEY_RIGHT_SHIFT: return ContentUiText("Right Shift", "Maj droite", "Umschalt rechts", "Mayús derecho", "Maiusc destro");
    case GLFW_KEY_LEFT_CONTROL: return ContentUiText("Left Ctrl", "Ctrl gauche", "Strg links", "Ctrl izquierdo", "Ctrl sinistro");
    case GLFW_KEY_RIGHT_CONTROL: return ContentUiText("Right Ctrl", "Ctrl droite", "Strg rechts", "Ctrl derecho", "Ctrl destro");
    case GLFW_KEY_LEFT_ALT: return ContentUiText("Left Alt", "Alt gauche", "Alt links", "Alt izquierdo", "Alt sinistro");
    case GLFW_KEY_RIGHT_ALT: return ContentUiText("Right Alt", "Alt droite", "Alt rechts", "Alt derecho", "Alt destro");
    case GLFW_KEY_UP: return ContentUiText("Up Arrow", "Flèche haut", "Pfeil oben", "Flecha arriba", "Freccia su");
    case GLFW_KEY_DOWN: return ContentUiText("Down Arrow", "Flèche bas", "Pfeil unten", "Flecha abajo", "Freccia giù");
    case GLFW_KEY_LEFT: return ContentUiText("Left Arrow", "Flèche gauche", "Pfeil links", "Flecha izquierda", "Freccia sinistra");
    case GLFW_KEY_RIGHT: return ContentUiText("Right Arrow", "Flèche droite", "Pfeil rechts", "Flecha derecha", "Freccia destra");
    default:
        if (const char* name = glfwGetKeyName(key, 0))
            return name;
        return ContentUiText("Key", "Touche", "Taste", "Tecla", "Tasto");
    }
}
}

bool StartupControllerInput()
{
#ifndef __SWITCH__
    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0)
    {
        std::fprintf(stderr, "[Input] SDL controller initialization failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GameControllerEventState(SDL_ENABLE);
    RefreshSdlControllers(true);
#endif
    return true;
}

void ShutdownControllerInput()
{
    StopJoyRumble(&g_joy);
#ifndef __SWITCH__
    for (SdlControllerDevice& device : s_sdlControllers)
        CloseSdlController(device);
    s_sdlControllers.clear();
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
#endif
}

bool FControllerInputAvailable()
{
#ifdef __SWITCH__
    return glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
#else
    if ((SDL_WasInit(SDL_INIT_GAMECONTROLLER) & SDL_INIT_GAMECONTROLLER) == 0)
        return false;
    RefreshSdlControllers(false);
    return !s_sdlControllers.empty();
#endif
}

const char* PchzKeyboardKeyName(int key)
{
    if (FMouseBinding(key))
    {
        switch (MouseButtonFromBinding(key))
        {
        case GLFW_MOUSE_BUTTON_LEFT: return ContentUiText("Mouse Left", "Souris gauche", "Maus links", "Ratón izquierdo", "Mouse sinistro");
        case GLFW_MOUSE_BUTTON_RIGHT: return ContentUiText("Mouse Right", "Souris droite", "Maus rechts", "Ratón derecho", "Mouse destro");
        case GLFW_MOUSE_BUTTON_MIDDLE: return ContentUiText("Mouse Middle", "Souris milieu", "Maus Mitte", "Ratón central", "Mouse centrale");
        case GLFW_MOUSE_BUTTON_4: return ContentUiText("Mouse 4", "Souris 4", "Maus 4", "Ratón 4", "Mouse 4");
        case GLFW_MOUSE_BUTTON_5: return ContentUiText("Mouse 5", "Souris 5", "Maus 5", "Ratón 5", "Mouse 5");
        case GLFW_MOUSE_BUTTON_6: return ContentUiText("Mouse 6", "Souris 6", "Maus 6", "Ratón 6", "Mouse 6");
        case GLFW_MOUSE_BUTTON_7: return ContentUiText("Mouse 7", "Souris 7", "Maus 7", "Ratón 7", "Mouse 7");
        case GLFW_MOUSE_BUTTON_8: return ContentUiText("Mouse 8", "Souris 8", "Maus 8", "Ratón 8", "Mouse 8");
        default: return ContentUiText("Mouse Button", "Bouton de souris", "Maustaste", "Botón del ratón", "Pulsante mouse");
        }
    }
    return PchzKeyboardKeyNameInternal(key);
}

const char* PchzGamepadBindingName(int binding)
{
#ifdef __SWITCH__
    switch (binding)
    {
    case GLFW_GAMEPAD_BUTTON_A: return "B / Cross";
    case GLFW_GAMEPAD_BUTTON_B: return "A / Circle";
    case GLFW_GAMEPAD_BUTTON_X: return "Y / Square";
    case GLFW_GAMEPAD_BUTTON_Y: return "X / Triangle";
    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return "L";
    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return "R";
    case GLFW_GAMEPAD_BUTTON_BACK: return "Minus / Select";
    case GLFW_GAMEPAD_BUTTON_START: return "Plus / Start";
    case GAMEPAD_BINDING_LEFT_TRIGGER: return "ZL";
    case GAMEPAD_BINDING_RIGHT_TRIGGER: return "ZR";
    default: break;
    }
#endif
    switch (binding)
    {
    case GLFW_GAMEPAD_BUTTON_A: return ContentUiText("A / Cross", "A / Croix", "A / Kreuz", "A / Cruz", "A / Croce");
    case GLFW_GAMEPAD_BUTTON_B: return ContentUiText("B / Circle", "B / Cercle", "B / Kreis", "B / Círculo", "B / Cerchio");
    case GLFW_GAMEPAD_BUTTON_X: return ContentUiText("X / Square", "X / Carré", "X / Quadrat", "X / Cuadrado", "X / Quadrato");
    case GLFW_GAMEPAD_BUTTON_Y: return ContentUiText("Y / Triangle", "Y / Triangle", "Y / Dreieck", "Y / Triángulo", "Y / Triangolo");
    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return ContentUiText("Left Bumper", "Bouton supérieur gauche", "Linke Schultertaste", "Botón superior izquierdo", "Dorsale sinistro");
    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return ContentUiText("Right Bumper", "Bouton supérieur droit", "Rechte Schultertaste", "Botón superior derecho", "Dorsale destro");
    case GLFW_GAMEPAD_BUTTON_BACK: return ContentUiText("Back / Select", "Retour / Select", "Zurück / Select", "Atrás / Select", "Indietro / Select");
    case GLFW_GAMEPAD_BUTTON_START: return "Start";
    case GLFW_GAMEPAD_BUTTON_GUIDE: return ContentUiText("Guide", "Guide", "Guide", "Guía", "Guida");
    case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return ContentUiText("Left Stick", "Stick gauche", "Linker Stick", "Stick izquierdo", "Levetta sinistra");
    case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return ContentUiText("Right Stick", "Stick droit", "Rechter Stick", "Stick derecho", "Levetta destra");
    case GLFW_GAMEPAD_BUTTON_DPAD_UP: return ContentUiText("D-Pad Up", "Croix haut", "Steuerkreuz oben", "Cruceta arriba", "Croce direzionale su");
    case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return ContentUiText("D-Pad Right", "Croix droite", "Steuerkreuz rechts", "Cruceta derecha", "Croce direzionale destra");
    case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return ContentUiText("D-Pad Down", "Croix bas", "Steuerkreuz unten", "Cruceta abajo", "Croce direzionale giù");
    case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return ContentUiText("D-Pad Left", "Croix gauche", "Steuerkreuz links", "Cruceta izquierda", "Croce direzionale sinistra");
    case GAMEPAD_BINDING_LEFT_TRIGGER: return ContentUiText("Left Trigger", "Gâchette gauche", "Linker Trigger", "Gatillo izquierdo", "Grilletto sinistro");
    case GAMEPAD_BINDING_RIGHT_TRIGGER: return ContentUiText("Right Trigger", "Gâchette droite", "Rechter Trigger", "Gatillo derecho", "Grilletto destro");
    default: return ContentUiText("Unbound", "Non assigné", "Nicht belegt", "Sin asignar", "Non assegnato");
    }
}

void JOY::Update(GLFWwindow* window)
{
    const int previousJoystickId = joystickId;
    const uint16_t grfbtnPrev = GrfbtnFromState(current);
    previous = current;
    handled.clear();
    forcedHeld.clear();

    for (int button = 0; button < BTN_MAX; ++button)
        current[static_cast<JOY_BUTTON>(button)] = false;

    for (int button = 0; button < BTN_MAX; ++button)
    {
        const int key = g_keyboardBindings[button];
        if (FMouseBinding(key))
            current[static_cast<JOY_BUTTON>(button)] =
                MOUSE::Button(MouseButtonFromBinding(key));
        else if (key != GLFW_KEY_UNKNOWN)
            current[static_cast<JOY_BUTTON>(button)] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    float xRaw = 0.0f;
    float yRaw = 0.0f;
    float x2Raw = 0.0f;
    float y2Raw = 0.0f;

    GLFWgamepadstate state{};
    InitializeGamepadState(&state);
    bool haveGamepadState = false;
    int activeJoystickId = JOYID_Nil;
    float bestActivity = -1.0f;
    const bool allowControllerInput = !g_fDisableControllerInputWhenUnfocused ||
        glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;

#ifdef __SWITCH__
    if (allowControllerInput && glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
    {
        haveGamepadState = true;
        activeJoystickId = GLFW_JOYSTICK_1;
    }
#else
    if (allowControllerInput &&
        (SDL_WasInit(SDL_INIT_GAMECONTROLLER) & SDL_INIT_GAMECONTROLLER) != 0)
    {
        RefreshSdlControllers(!gamepadConnected);
        for (const SdlControllerDevice& device : s_sdlControllers)
        {
            GLFWgamepadstate candidate{};
            if (!GetSdlControllerState(device.controller, &candidate))
                continue;

            const float activity = GamepadActivity(candidate);
            const bool selectedTie = device.instanceId == joystickId &&
                std::abs(activity - bestActivity) <= GAMEPAD_ACTIVITY_TIE_EPSILON;
            if (!haveGamepadState ||
                activity > bestActivity + GAMEPAD_ACTIVITY_TIE_EPSILON || selectedTie)
            {
                state = candidate;
                activeJoystickId = static_cast<int>(device.instanceId);
                bestActivity = activity;
                haveGamepadState = true;
            }
        }
    }

#endif

    if (haveGamepadState)
    {
        if (!gamepadConnected || activeJoystickId != joystickId)
        {
            calibration = StickCalibration{};
            calibration2 = StickCalibration{};
        }

        joystickId = activeJoystickId;
        gamepadConnected = true;
        gamepadState = state;
        hasGamepadState = true;
        for (int button = 0; button < BTN_MAX; ++button)
        {
            const int binding = g_gamepadBindings[button];
            bool held = false;
            if (binding >= 0 && binding <= GLFW_GAMEPAD_BUTTON_LAST)
                held = state.buttons[binding] == GLFW_PRESS;
            else if (binding == GAMEPAD_BINDING_LEFT_TRIGGER)
                held = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.25f;
            else if (binding == GAMEPAD_BINDING_RIGHT_TRIGGER)
                held = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.25f;
            current[static_cast<JOY_BUTTON>(button)] = current[static_cast<JOY_BUTTON>(button)] || held;
        }

        xRaw = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        // STEP/JT expects the original PlayStation convention: forward is
        // positive Y. GLFW reports stick-up as -1, so invert it here.
        yRaw = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        x2Raw = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        y2Raw = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
    }
    else
    {
        joystickId = JOYID_Nil;
        gamepadConnected = false;
        hasGamepadState = false;
        InitializeGamepadState(&gamepadState);
    }

    if (previousJoystickId != JOYID_Nil && previousJoystickId != joystickId)
    {
#ifdef __SWITCH__
        CaneSwitchRumble(0.0f, 0.0f);
#else
        if (SDL_GameController* previousController = PcontrollerFromInstanceId(previousJoystickId))
            SDL_GameControllerRumble(previousController, 0, 0, 0);
#endif
    }

    // Keyboard movement emulates the left analog stick.  The base CPMAN
    // policy is normally active during gameplay, so it must not suppress
    // these values or the player receives a permanently centered stick.
    if (current[BTN_LEFT] && !current[BTN_RIGHT]) xRaw = -1.0f;
    if (current[BTN_RIGHT] && !current[BTN_LEFT]) xRaw = 1.0f;
    if (current[BTN_UP] && !current[BTN_DOWN]) yRaw = 1.0f;
    if (current[BTN_DOWN] && !current[BTN_UP]) yRaw = -1.0f;

    // Keyboard camera input emulates the right analog stick. UpdateCptn
    // consumes x2 for manual orbiting around the player.
    const bool cameraLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool cameraRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool cameraUp = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool cameraDown = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    if (cameraLeft && !cameraRight) x2Raw = -1.0f;
    if (cameraRight && !cameraLeft) x2Raw = 1.0f;
    if (cameraUp && !cameraDown) y2Raw = 1.0f;
    if (cameraDown && !cameraUp) y2Raw = -1.0f;

    ApplyStickDeadZone(xRaw, yRaw, &calibration, &x, &y, &uDeflect);
    ApplyStickDeadZone(x2Raw, y2Raw, &calibration2, &x2, &y2, &uDeflect2);

    _MatchCodes(this, grfbtnPrev);

    UpdateRumble(&rumble, this);

    joys = JOYS_Ready;
}

void AddCode(CODE* pcode)
{
    for (CODE* current = s_pcode; current != nullptr; current = current->pcodeNext)
        if (current == pcode)
            return;

    pcode->pcodeNext = s_pcode;
    s_pcode = pcode;
    pcode->ijbcCur = GrfbtnFromState(g_joy.current) == pcode->ajbc[0] ? 1 : 0;
}

void _ResetCodes()
{
    for (CODE* code = s_pcode; code != nullptr; code = code->pcodeNext)
        code->ijbcCur = 0;
}

void _MatchCodes(JOY* pjoy, uint16_t grfbtnPrev)
{
    const uint16_t grfbtn = GrfbtnFromState(g_joy.current);
    if (s_pcode == nullptr || pjoy != &g_joy || grfbtnPrev == grfbtn || grfbtn == 0)
        return;

    for (CODE* code = s_pcode; code != nullptr; code = code->pcodeNext)
    {
        if (code->ijbcCur < code->cjbc)
            code->ijbcCur = grfbtn == code->ajbc[code->ijbcCur] ? code->ijbcCur + 1 : 0;
    }

    s_tCodeCheck = g_clock.tReal + 1.0f;
}

void UpdateCodes()
{
    if (s_tCodeCheck == 0.0f || s_tCodeCheck > g_clock.tReal)
        return;

    CODE* matched = nullptr;
    CODE** link = &s_pcode;
    while (*link != nullptr)
    {
        CODE* code = *link;
        if (code->fRemove != 0)
        {
            *link = code->pcodeNext;
            code->pcodeNext = nullptr;
            code->ijbcCur = 0;
            code->fRemove = 0;
            continue;
        }

        if (code->ijbcCur >= code->cjbc &&
            (matched == nullptr || matched->cjbc < code->cjbc))
            matched = code;

        link = &code->pcodeNext;
    }

    if (matched != nullptr)
    {
        StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr,
            3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        matched->pfn(matched->nParam);
    }

    _ResetCodes();
    s_tCodeCheck = 0.0f;
}

void StartupCodes()
{
    for (CODE& code : s_codes)
        AddCode(&code);
}

bool JOY::IsPressed(JOY_BUTTON button)
{
    return current[button] && !previous[button] && !handled[button];
}

bool JOY::IsHeld(JOY_BUTTON button)
{
    return current[button] || forcedHeld[button];
}

bool JOY::IsReleased(JOY_BUTTON button)
{
    return !current[button] && previous[button] && !handled[button];
}

void JOY::SetHandled(JOY_BUTTON button)
{
    handled[button] = true;
}

int JOY::DxSelectionJoy(float currentTime)
{
    if (dxLatch == 0)
    {
        if (IsPressed(BTN_LEFT))
        {
            SetHandled(BTN_LEFT);
            dxLatch = -1;
        }
        else if (IsPressed(BTN_RIGHT))
        {
            SetHandled(BTN_RIGHT);
            dxLatch = 1;
        }
        else if (uDeflect >= 0.8f && std::abs(y) < std::abs(x))
        {
            dxLatch = x < 0.0f ? -1 : 1;
        }

        if (dxLatch == 0)
            return 0;

        dtLatchX = 0.3333f;
    }
    else
    {
        bool fStillHeld;
        if (dxLatch < 0)
        {
            fStillHeld = IsHeld(BTN_LEFT) ||
                (uDeflect >= 0.25f && std::abs(y) < std::abs(x) && x < 0.0f);
        }
        else
        {
            fStillHeld = IsHeld(BTN_RIGHT) ||
                (uDeflect >= 0.25f && std::abs(y) < std::abs(x) && x > 0.0f);
        }

        if (!fStillHeld)
        {
            dxLatch = 0;
            return 0;
        }

        if (currentTime < tLatchX)
            return 0;

        dtLatchX = glm::clamp(dtLatchX * 0.6666667f, 0.1f, 0.3333f);
    }

    tLatchX = currentTime + dtLatchX;
    return dxLatch;
}

int JOY::DySelectionJoy(float currentTime)
{
    if (dyLatch == 0)
    {
        if (IsPressed(BTN_UP))
        {
            SetHandled(BTN_UP);
            dyLatch = -1;
        }
        else if (IsPressed(BTN_DOWN))
        {
            SetHandled(BTN_DOWN);
            dyLatch = 1;
        }
        else if (uDeflect >= 0.8f && std::abs(x) < std::abs(y))
        {
            dyLatch = y > 0.0f ? -1 : 1;
        }

        if (dyLatch == 0)
            return 0;

        dtLatchY = 0.3333f;
    }
    else
    {
        bool fStillHeld;
        if (dyLatch < 0)
        {
            fStillHeld = IsHeld(BTN_UP) ||
                (uDeflect >= 0.25f && std::abs(x) < std::abs(y) && y > 0.0f);
        }
        else
        {
            fStillHeld = IsHeld(BTN_DOWN) ||
                (uDeflect >= 0.25f && std::abs(x) < std::abs(y) && y < 0.0f);
        }

        if (!fStillHeld)
        {
            dyLatch = 0;
            return 0;
        }

        if (currentTime < tLatchY)
            return 0;

        dtLatchY = glm::clamp(dtLatchY * 0.6666667f, 0.1f, 0.3333f);
    }

    tLatchY = currentTime + dtLatchY;
    return dyLatch;
}

void JOY::StartJoySelection()
{
    dxLatch = 0;
    dyLatch = 0;
    tLatchX = 0.0f;
    tLatchY = 0.0f;
    dtLatchX = 0.3333f;
    dtLatchY = 0.3333f;
}

void AddGrfusr(GRFUSR grfusr)
{
    g_grfusr |= grfusr;
    UpdateGrfjoytFromGrfusr();
}

void RemoveGrfusr(GRFUSR grfusr)
{
    g_grfusr &= ~grfusr;
    UpdateGrfjoytFromGrfusr();
}

void UpdateGrfjoytFromGrfusr()
{
    if ((g_grfusr & 4) != 0)
        g_grfjoyt = 0;
    else if ((g_grfusr & 1) != 0)
        g_grfjoyt = 5;
    else if ((g_grfusr & 2) != 0)
        g_grfjoyt = 4;
    else
        g_grfjoyt = 7;
}

void EnableVibration()
{
    if (vibrationSetting == 0)
        vibrationSetting = 1;
}

void StopJoyRumble(JOY* pjoy)
{
    if (pjoy == nullptr)
        return;

    SetSdlRumble(pjoy, 0, 0);
    pjoy->rumble.rums = RUMS_Idle;
    pjoy->rumble.prumpat = nullptr;
    pjoy->rumble.irumins = 0;
    pjoy->rumble.dtRumins = 0.0f;
    pjoy->rumble.dtRumble = 0.0f;
}

void TriggerJoyRumbleRumk(JOY* pjoy, RUMK rumk, float dt)
{
    if (pjoy == nullptr || rumk < RUMK_SteadyBuzz || rumk >= RUMK_Max || dt <= 0.0f ||
        g_pgsCur == nullptr || !FVibrationEnabled(g_pgsCur))
        return;

    const RUMPAT& pattern = s_arumpat[static_cast<int>(rumk)];
    if (pattern.crumins <= 0)
        return;

    RUMBLE& rumble = pjoy->rumble;
    rumble.dtRumble = dt;
    rumble.prumpat = &pattern;
    rumble.irumins = 0;
    rumble.dtRumins = pattern.arumins[0].dt;
    rumble.rums = RUMS_Rumble;
    SetSdlRumble(pjoy, pattern.arumins[0].fHighSpeedMotor,
        pattern.arumins[0].bLowSpeedMotor);
}

void UpdateRumble(RUMBLE* prumble, JOY* pjoy)
{
    if (prumble == nullptr || pjoy == nullptr || prumble->rums != RUMS_Rumble)
        return;

    if (!g_clock.fEnabled || prumble->dtRumble <= 0.0f || g_pgsCur == nullptr ||
        !FVibrationEnabled(g_pgsCur) || !pjoy->gamepadConnected ||
        prumble->prumpat == nullptr || prumble->prumpat->crumins <= 0)
    {
        StopJoyRumble(pjoy);
        return;
    }

    const float dt = (std::max)(0.0f, g_clock.dtReal);
    prumble->dtRumble -= dt;
    prumble->dtRumins -= dt;

    // Advance every elapsed instruction. The loop matters after a long frame:
    // retail does not stretch a short pulse merely because a frame was late.
    while (prumble->dtRumins <= 0.0f && prumble->dtRumble > 0.0f)
    {
        prumble->irumins = (prumble->irumins + 1) % prumble->prumpat->crumins;
        const RUMINS& instruction = prumble->prumpat->arumins[prumble->irumins];
        prumble->dtRumins += (std::max)(instruction.dt, 0.001f);
        SetSdlRumble(pjoy, instruction.fHighSpeedMotor, instruction.bLowSpeedMotor);
    }

    if (prumble->dtRumble <= 0.0f)
        StopJoyRumble(pjoy);
}

void ToggleVibration(GS* pgs)
{
    constexpr uint32_t GRFGS_VibrationDisabled = 0x20;

    switch (vibrationSetting)
    {
    case 1:
        // Toggle the saved vibration flag.
        pgs->grfgs ^= GRFGS_VibrationDisabled;
        break;

    case 2:
        // Currently forced on; switch it off.
        pgs->grfgs |= GRFGS_VibrationDisabled;
        break;

    case 3:
        // Currently forced off; switch it on.
        pgs->grfgs &= ~GRFGS_VibrationDisabled;
        break;

    default:
        break;
    }

    if ((pgs->grfgs & GRFGS_VibrationDisabled) == 0)
        vibrationSetting = 2; // Enabled
    else
        vibrationSetting = 3; // Disabled

    if (!FVibrationEnabled(pgs))
        StopJoyRumble(&g_joy);
}

void ApplyVibrationSetting(GS* pgs)
{
    if (vibrationSetting == 2) {
        pgs->grfgs = pgs->grfgs & 4294967263;
        return;
    }
    if ((2 < vibrationSetting) && (vibrationSetting == 3)) {
        pgs->grfgs = pgs->grfgs | 32;
        StopJoyRumble(&g_joy);
        return;
    }
}

bool FVibrationEnabled(GS* pgs)
{
    if (vibrationSetting == 0 || vibrationSetting == 3)
        return 0;

    if (vibrationSetting == 2)
        return 1;

    return (pgs->grfgs & 0x20) == 0;
}

JOY g_joy{};
JOY g_joyZero{};
int g_grfjoyt;
GRFUSR g_grfusr;
int vibrationSetting = 1;
std::array<int, BTN_MAX> g_keyboardBindings = kDefaultKeyboardBindings;
std::array<int, BTN_MAX> g_gamepadBindings = kDefaultGamepadBindings;

void ResetKeyboardMouseBindings()
{
    g_keyboardBindings = kDefaultKeyboardBindings;
}

void ResetControllerBindings()
{
    g_gamepadBindings = kDefaultGamepadBindings;
}
bool g_fDisableControllerInputWhenUnfocused = false;
bool g_fMouseCamera = false;
float g_mouseCameraSensitivity = 1.0f;
