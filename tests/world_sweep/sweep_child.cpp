// Child modes of the world sweep. Each runs in its own process, boots the
// real game (Startup) on a hidden GL 4.1 window and drives it through the
// game's own functions: CTransition for loads, main.cpp's RunGameFrame for
// frames, the camera policies for extra views, fmv.cpp for movies, sound.cpp
// for banks and save.cpp for saves; van-door, hub-tour and charm-exit play
// scripted situations (see each mode); the behaviour and unlock modes live
// in sweep_behaviour_child.cpp (sweep_child.h). Results go to a JSON file
// named by the runner; [ERROR] lines from ReportRuntimeError go to stderr,
// which the runner captures and attributes using the "=== LEVEL ... ==="
// markers. The sweep links its own basic.cpp/sidebag.cpp with a probe
// (sidebag_probe.cpp), and CANE_SWEEP_GL_TRACE=1 logs the GL call behind
// each GL error.

#include "sweep_game.h"
#include "sweep_child.h"
#include "sweep_core.h"
#include "sweep_display.h"
#include "sweep_sidebag.h"
#include "../background_app.h"

#include "../../Sly1-Release/main.h"
#include "../../Sly1-Release/mouse.h"
#include "../../Sly1-Release/cm.h"
#include "../../Sly1-Release/clock.h"
#include "../../Sly1-Release/coin.h"
#include "../../Sly1-Release/content.h"
#include "../../Sly1-Release/find.h"
#include "../../Sly1-Release/fmv.h"
#include "../../Sly1-Release/game.h"
#include "../../Sly1-Release/gl.h"
#include "../../Sly1-Release/jt.h"
#include "../../Sly1-Release/po.h"
#include "../../Sly1-Release/pzo.h"
#include "../../Sly1-Release/save.h"
#include "../../Sly1-Release/sm.h"
#include "../../Sly1-Release/sound.h"
#include "../../Sly1-Release/sw.h"
#include "../../Sly1-Release/transition.h"
#include "../../Sly1-Release/tv.h"
#include "../../Sly1-Release/xform.h"

extern "C"
{
#include <libavformat/avformat.h>
}

#include <SDL.h>

#include <CoreGraphics/CoreGraphics.h>
#include <execinfo.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <thread>
#include <type_traits>

extern std::string file;

// NSApp's presentation options, read through the Objective-C runtime's C
// API: objc's own headers clash with the game's BOOL (spaprops.h).
struct objc_object;
struct objc_selector;
extern "C" objc_object* objc_getClass(const char* name);
extern "C" objc_selector* sel_registerName(const char* name);
extern "C" void objc_msgSend(void);

// Named, so the other child files reach the shared pieces (sweep_child.h).
namespace sweep_child
{
using Clock = std::chrono::steady_clock;
namespace fs = std::filesystem;

double MsSince(Clock::time_point start)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

// --- options -----------------------------------------------------------------

Options ParseOptions(const std::vector<std::string>& args, size_t first)
{
    Options options;
    for (size_t i = first; i < args.size(); ++i)
    {
        if (args[i].rfind("--", 0) == 0)
        {
            if (i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0)
                options.values[args[i]] = args[i + 1], ++i;
            else
                options.values[args[i]] = "1";
        }
        else
            options.positional.push_back(args[i]);
    }
    return options;
}

// --- crash reporting ---------------------------------------------------------

void CrashHandler(int signal)
{
    char header[64];
    const int length = std::snprintf(header, sizeof(header), "\n=== SIGNAL %d ===\n", signal);
    (void)!write(STDERR_FILENO, header, static_cast<size_t>(length));
    void* frames[96];
    const int count = backtrace(frames, 96);
    backtrace_symbols_fd(frames, count, STDERR_FILENO);
    _exit(128 + signal);
}

// The handler runs on its own stack (SA_ONSTACK), so a stack overflow,
// which leaves no room on the thread's stack, still gets its backtrace.
// Only the main thread gets the alternate stack; the game crashes there.
void InstallCrashHandler()
{
    static std::vector<char> s_alternateStack(static_cast<size_t>(std::max<long>(SIGSTKSZ, 256 * 1024)));
    stack_t stack{};
    stack.ss_sp = s_alternateStack.data();
    stack.ss_size = s_alternateStack.size();
    if (sigaltstack(&stack, nullptr) != 0)
        std::fprintf(stderr, "[ERROR] [Sweep] sigaltstack failed (%s); a stack overflow would lose its backtrace\n",
            std::strerror(errno));
    struct sigaction action{};
    action.sa_handler = CrashHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_ONSTACK;
    for (int signal : { SIGSEGV, SIGBUS, SIGABRT, SIGILL, SIGFPE, SIGTRAP })
        sigaction(signal, &action, nullptr);
}

// --- sampler audit (on unless CANE_SWEEP_SAMPLER_AUDIT=0) ---------------------
// Apple's GL 4.1 driver only logs "unit N GLD_TEXTURE_INDEX_BUFFER is
// unloadable" once per process and never raises a GL error. With the audit
// on, every draw checks that each buffer-texture sampler of the current
// program reads a unit with a buffer texture bound, and prints the program, uniform and call stack the first time one does not.

namespace audit
{
PFNGLDRAWARRAYSPROC realDrawArrays;
PFNGLDRAWELEMENTSPROC realDrawElements;
PFNGLDRAWELEMENTSBASEVERTEXPROC realDrawElementsBaseVertex;
PFNGLDRAWARRAYSINSTANCEDPROC realDrawArraysInstanced;
PFNGLDRAWELEMENTSINSTANCEDPROC realDrawElementsInstanced;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC realDrawElementsInstancedBaseVertex;
PFNGLDRAWRANGEELEMENTSPROC realDrawRangeElements;

struct SamplerUniform { std::string name; GLint location; GLenum type; };
std::map<GLuint, std::vector<SamplerUniform>> s_programs;
std::set<std::string> s_reported;
int s_problems = 0;

const std::vector<SamplerUniform>& BufferSamplers(GLuint program)
{
    auto it = s_programs.find(program);
    if (it != s_programs.end())
        return it->second;
    std::vector<SamplerUniform> list;
    GLint count = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    for (GLint i = 0; i < count; ++i)
    {
        char name[256];
        GLsizei length = 0;
        GLint size = 0;
        GLenum type = 0;
        glGetActiveUniform(program, static_cast<GLuint>(i), sizeof(name), &length, &size, &type, name);
        if (type == GL_SAMPLER_BUFFER || type == GL_INT_SAMPLER_BUFFER || type == GL_UNSIGNED_INT_SAMPLER_BUFFER ||
            type == GL_SAMPLER_2D || type == GL_SAMPLER_2D_ARRAY)
            list.push_back({ name, glGetUniformLocation(program, name), type });
    }
    return s_programs[program] = list;
}

void Check(const char* call)
{
    GLint program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program == 0)
        return;
    GLint activeUnit = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeUnit);
    for (const SamplerUniform& sampler : BufferSamplers(static_cast<GLuint>(program)))
    {
        GLint unit = 0;
        glGetUniformiv(static_cast<GLuint>(program), sampler.location, &unit);
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
        // A unit with no texture for the sampler's target is what the
        // driver calls "unloadable"; sampling it returns zero.
        const GLenum binding = sampler.type == GL_SAMPLER_2D ? GL_TEXTURE_BINDING_2D
            : sampler.type == GL_SAMPLER_2D_ARRAY ? GL_TEXTURE_BINDING_2D_ARRAY : GL_TEXTURE_BINDING_BUFFER;
        GLint texture = 0;
        glGetIntegerv(binding, &texture);
        if (texture != 0)
            continue;
        ++s_problems;
        const std::string key = std::to_string(program) + ":" + sampler.name;
        if (!s_reported.insert(key).second)
            continue;
        std::fprintf(stderr, "[sampler-audit] %s: program %d uniform '%s' unit %d has no %s texture\n",
            call, program, sampler.name.c_str(), unit,
            sampler.type == GL_SAMPLER_2D ? "2D" : sampler.type == GL_SAMPLER_2D_ARRAY ? "2D array" : "buffer");
        void* frames[24];
        const int depth = backtrace(frames, 24);
        backtrace_symbols_fd(frames, depth, STDERR_FILENO);
    }
    glActiveTexture(static_cast<GLenum>(activeUnit));
}

// Each level reports its own empty units, not only the first level's.
void NewLevel() { s_reported.clear(); }

void APIENTRY DrawArrays(GLenum m, GLint f, GLsizei c) { Check("glDrawArrays"); realDrawArrays(m, f, c); }
void APIENTRY DrawElements(GLenum m, GLsizei c, GLenum t, const void* i) { Check("glDrawElements"); realDrawElements(m, c, t, i); }
void APIENTRY DrawElementsBaseVertex(GLenum m, GLsizei c, GLenum t, const void* i, GLint b) { Check("glDrawElementsBaseVertex"); realDrawElementsBaseVertex(m, c, t, i, b); }
void APIENTRY DrawArraysInstanced(GLenum m, GLint f, GLsizei c, GLsizei n) { Check("glDrawArraysInstanced"); realDrawArraysInstanced(m, f, c, n); }
void APIENTRY DrawElementsInstanced(GLenum m, GLsizei c, GLenum t, const void* i, GLsizei n) { Check("glDrawElementsInstanced"); realDrawElementsInstanced(m, c, t, i, n); }
void APIENTRY DrawElementsInstancedBaseVertex(GLenum m, GLsizei c, GLenum t, const void* i, GLsizei n, GLint b) { Check("glDrawElementsInstancedBaseVertex"); realDrawElementsInstancedBaseVertex(m, c, t, i, n, b); }
void APIENTRY DrawRangeElements(GLenum m, GLuint s, GLuint e, GLsizei c, GLenum t, const void* i) { Check("glDrawRangeElements"); realDrawRangeElements(m, s, e, c, t, i); }

void Install()
{
    const char* flag = std::getenv("CANE_SWEEP_SAMPLER_AUDIT");
    if (flag != nullptr && std::strcmp(flag, "0") == 0)
        return;
    realDrawArrays = glad_glDrawArrays; glad_glDrawArrays = DrawArrays;
    realDrawElements = glad_glDrawElements; glad_glDrawElements = DrawElements;
    realDrawElementsBaseVertex = glad_glDrawElementsBaseVertex; glad_glDrawElementsBaseVertex = DrawElementsBaseVertex;
    realDrawArraysInstanced = glad_glDrawArraysInstanced; glad_glDrawArraysInstanced = DrawArraysInstanced;
    realDrawElementsInstanced = glad_glDrawElementsInstanced; glad_glDrawElementsInstanced = DrawElementsInstanced;
    realDrawElementsInstancedBaseVertex = glad_glDrawElementsInstancedBaseVertex; glad_glDrawElementsInstancedBaseVertex = DrawElementsInstancedBaseVertex;
    realDrawRangeElements = glad_glDrawRangeElements; glad_glDrawRangeElements = DrawRangeElements;
    std::fprintf(stderr, "[sampler-audit] installed\n");
}
}

// --- GL call trace (off unless CANE_SWEEP_GL_TRACE=1) -------------------------
// The sweep counts GL errors once per frame, which says which frame raised
// one but not which call. With the trace on, every GL function the game
// calls checks glGetError right after itself and prints the function, the
// error and the call stack the first time each pair occurs. The errors it
// takes are handed back to the frame count (TakeErrors), so the checks still
// see them. Slow; for finding a call, not for normal runs.

namespace gltrace
{
PFNGLGETERRORPROC realGetError = nullptr;
int s_taken = 0;
GLenum s_firstTaken = GL_NO_ERROR;
std::set<std::string> s_reported;

bool s_inside = false;
// The most recent calls, oldest first from s_next, and the stack of the most
// recent glBindVertexArray, to show what led up to a failed draw.
const char* s_recent[48] = {};
size_t s_next = 0;
void* s_bindStack[24];
int s_bindDepth = 0;

void Before(const char* name)
{
    // DescribeDraw's own queries are not part of the history.
    if (s_inside)
        return;
    s_recent[s_next++ % std::size(s_recent)] = name;
    if (std::strcmp(name, "glBindVertexArray") == 0)
        s_bindDepth = backtrace(s_bindStack, static_cast<int>(std::size(s_bindStack)));
}

// For a draw that failed with GL_INVALID_OPERATION: the bound program,
// vertex array and transform feedback state, and the driver's own reason
// from glValidateProgram.
void DescribeDraw()
{
    GLint program = 0, vao = 0, feedback = 0, paused = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_ACTIVE, &feedback);
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_PAUSED, &paused);
    std::fprintf(stderr, "[gl-trace]   program %d, vertex array %d, transform feedback active %d paused %d\n",
        program, vao, feedback, paused);
    if (program == 0)
        return;
    glValidateProgram(static_cast<GLuint>(program));
    GLint valid = 0, length = 0;
    glGetProgramiv(static_cast<GLuint>(program), GL_VALIDATE_STATUS, &valid);
    glGetProgramiv(static_cast<GLuint>(program), GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<size_t>((std::max)(length, 1)), '\0');
    glGetProgramInfoLog(static_cast<GLuint>(program), static_cast<GLsizei>(log.size()), nullptr, log.data());
    std::fprintf(stderr, "[gl-trace]   glValidateProgram: %s %s\n", valid ? "valid" : "INVALID", log.c_str());
    while (realGetError() != GL_NO_ERROR) {}
    std::fprintf(stderr, "[gl-trace]   calls before it, oldest first:");
    for (size_t i = 0; i < std::size(s_recent); ++i)
        if (const char* recent = s_recent[(s_next + i) % std::size(s_recent)])
            std::fprintf(stderr, " %s", recent);
    std::fprintf(stderr, "\n[gl-trace]   last glBindVertexArray was called from:\n");
    backtrace_symbols_fd(s_bindStack, s_bindDepth, STDERR_FILENO);
}

void After(const char* name)
{
    if (s_inside)
        return;
    for (GLenum error = realGetError(); error != GL_NO_ERROR; error = realGetError())
    {
        if (s_taken++ == 0)
            s_firstTaken = error;
        char key[96];
        std::snprintf(key, sizeof(key), "%s:%X", name, error);
        if (!s_reported.insert(key).second)
            continue;
        std::fprintf(stderr, "[gl-trace] %s raised 0x%X\n", name, error);
        if (error == GL_INVALID_OPERATION && std::strncmp(name, "glDraw", 6) == 0)
        {
            s_inside = true;
            DescribeDraw();
            s_inside = false;
        }
        void* frames[32];
        const int depth = backtrace(frames, 32);
        backtrace_symbols_fd(frames, depth, STDERR_FILENO);
    }
}

template <int N, typename R, typename... A>
struct Hook
{
    static inline R (*real)(A...) = nullptr;
    static inline const char* name = nullptr;
    static R APIENTRY Call(A... args)
    {
        Before(name);
        if constexpr (std::is_void_v<R>)
        {
            real(args...);
            After(name);
        }
        else
        {
            R value = real(args...);
            After(name);
            return value;
        }
    }
};

template <int N, typename R, typename... A>
void Wrap(R (APIENTRYP& slot)(A...), const char* name)
{
    if (slot == nullptr)
        return;
    Hook<N, R, A...>::real = slot;
    Hook<N, R, A...>::name = name;
    slot = &Hook<N, R, A...>::Call;
}

// Errors the trace took since the last call, and the first of them.
int TakeErrors(GLenum& first)
{
    const int taken = s_taken;
    first = s_firstTaken;
    s_taken = 0;
    s_firstTaken = GL_NO_ERROR;
    return taken;
}

// Every GL function the game's sources call, except glGetError.
#define CANE_SWEEP_TRACED_GL(X) \
    X(glActiveTexture) X(glAttachShader) X(glBeginTransformFeedback) X(glBindBuffer) X(glBindBufferBase) \
    X(glBindBufferRange) X(glBindFramebuffer) X(glBindRenderbuffer) X(glBindSampler) X(glBindTexture) \
    X(glBindVertexArray) X(glBlendEquation) X(glBlendEquationSeparate) X(glBlendFunc) X(glBlendFuncSeparate) \
    X(glBlitFramebuffer) X(glBufferData) X(glBufferSubData) X(glCheckFramebufferStatus) X(glClear) \
    X(glClearBufferfv) X(glClearColor) X(glClearDepth) X(glClearStencil) X(glClientWaitSync) X(glColorMask) \
    X(glCompileShader) X(glCompressedTexSubImage2D) X(glCreateProgram) X(glCreateShader) X(glCullFace) \
    X(glDeleteBuffers) X(glDeleteFramebuffers) X(glDeleteProgram) X(glDeleteRenderbuffers) X(glDeleteShader) \
    X(glDeleteSync) X(glDeleteTextures) X(glDeleteVertexArrays) X(glDepthFunc) X(glDepthMask) X(glDepthRange) \
    X(glDisable) X(glDisableVertexAttribArray) X(glDrawArrays) X(glDrawBuffer) X(glDrawElements) \
    X(glDrawElementsBaseVertex) X(glDrawElementsInstanced) X(glDrawElementsInstancedBaseInstance) X(glEnable) \
    X(glEnableVertexAttribArray) X(glEndTransformFeedback) X(glFenceSync) X(glFinish) \
    X(glFramebufferRenderbuffer) X(glFramebufferTexture2D) X(glFramebufferTextureLayer) X(glFrontFace) \
    X(glGenBuffers) X(glGenFramebuffers) X(glGenRenderbuffers) X(glGenSamplers) X(glGenTextures) \
    X(glGenVertexArrays) X(glGenerateMipmap) X(glGetBooleanv) X(glGetDoublev) X(glGetFloatv) X(glGetIntegerv) \
    X(glGetProgramInfoLog) X(glGetProgramiv) X(glGetShaderInfoLog) X(glGetShaderiv) X(glGetString) \
    X(glGetStringi) X(glGetTexImage) X(glGetTexLevelParameteriv) X(glGetUniformBlockIndex) \
    X(glGetUniformLocation) X(glIsEnabled) X(glLineWidth) X(glLinkProgram) X(glMemoryBarrier) X(glPixelStorei) \
    X(glPointSize) X(glPolygonMode) X(glProgramUniform1f) X(glProgramUniform1i) X(glProgramUniform2f) \
    X(glReadBuffer) X(glRenderbufferStorage) X(glRenderbufferStorageMultisample) X(glSamplerParameteri) \
    X(glScissor) X(glShaderSource) X(glStencilFunc) X(glStencilMask) X(glStencilOp) X(glTexBuffer) \
    X(glTexImage2D) X(glTexImage3D) X(glTexParameterf) X(glTexParameteri) X(glTexStorage2D) X(glTexStorage3D) \
    X(glTexSubImage2D) X(glTransformFeedbackVaryings) X(glUniform1f) X(glUniform1i) X(glUniform2f) \
    X(glUniform3f) X(glUniform3fv) X(glUniform4f) X(glUniform4fv) X(glUniformBlockBinding) \
    X(glUniformMatrix3fv) X(glUniformMatrix4fv) X(glUnmapBuffer) X(glUseProgram) X(glVertexAttrib2f) \
    X(glVertexAttrib3f) X(glVertexAttrib4f) X(glVertexAttribDivisor) X(glVertexAttribIPointer) \
    X(glVertexAttribPointer) X(glViewport)

void Install()
{
    const char* flag = std::getenv("CANE_SWEEP_GL_TRACE");
    if (flag == nullptr || std::strcmp(flag, "1") != 0)
        return;
    realGetError = glad_glGetError;
#define CANE_SWEEP_TRACE_ONE(fn) Wrap<__COUNTER__>(glad_##fn, #fn);
    CANE_SWEEP_TRACED_GL(CANE_SWEEP_TRACE_ONE)
#undef CANE_SWEEP_TRACE_ONE
    std::fprintf(stderr, "[gl-trace] installed\n");
}
}

// --- boot --------------------------------------------------------------------

// Every joystick SDL shows, game controller or not: a plain joystick is
// input the process could read too.
int VisibleInputDevices()
{
    return SDL_NumJoysticks();
}

void BootGame(const Options& options)
{
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    InstallCrashHandler();
    setenv("SDL_AUDIO_DRIVER", "dummy", 0);
    // Never read the user's controllers: the game enables background joystick
    // events (Input.cpp), so a test process would otherwise act on the
    // user's gamepad while they play (e.g. skip cutscenes). No device matches
    // VID/PID 0000/0000, so SDL ignores them all.
    setenv("SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT", "0x0000/0x0000", 1);
    // No Dock icon, and never take focus from the user.
    cane_test::MakeBackgroundOnly();

    // RunApplication's preamble, with the user folder always pointed at a
    // scratch directory so settings and saves never touch the real one.
    cane::ConfigureRuntime({ "--data-dir", options.Get("--data-dir"), "--user-dir", options.Get("--user-dir") });
    RefreshContentPackage(cane::DataDirectory());

    // glfwInit is idempotent, so InitGL's own call keeps this hint: the
    // window is never shown.
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    Startup();
    glfwSwapInterval(0);
    audit::Install();
    gltrace::Install();

    // A test process must never read the user's controllers (the user may be
    // playing) or show up in the Dock and take focus.
    if (const int devices = VisibleInputDevices(); devices != 0)
        std::fprintf(stderr, "[ERROR] [Sweep] this test process can see %d game controller(s) or joystick(s); "
            "SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT=0x0000/0x0000 must hide them\n", devices);
    if (cane_test::ActivationPolicy() != cane_test::kPolicyProhibited)
        std::fprintf(stderr, "[ERROR] [Sweep] this test process is not background-only "
            "(activation policy %ld)\n", cane_test::ActivationPolicy());

    // StartGame queued the Splash load (wipe straight to black, then
    // CTransition::Set). Drop it; each mode loads what it needs.
    g_transition.m_fPending = 0;
    g_transition.m_plevelPending = nullptr;
    g_transition.m_worldPending.clear();
}

void ShutdownGame()
{
    // RunApplication's tail.
    if (g_psw != nullptr)
        DeleteWorld(g_psw);
    ShutdownSound();
    ShutdownControllerInput();
    FreeTvGL();
    g_gl.TerminateGL();
    // TerminateGL frees the placeholder textures (glshaders.h) with the
    // game's other GL objects, while the context is still current.
    if (!PlaceholderBufferTextures().empty() || PlaceholderTexture2DArrayName() != 0)
        std::fprintf(stderr, "[ERROR] [Sweep] GL::TerminateGL left %zu placeholder buffer texture(s) and "
            "placeholder 2D array %u alive\n", PlaceholderBufferTextures().size(), PlaceholderTexture2DArrayName());
}

void WriteJson(const fs::path& path, const nlohmann::json& json)
{
    std::ofstream(path, std::ios::trunc) << json.dump(2) << "\n";
}

// Sidebags replaced since the last call (sidebag_probe.cpp), as report rows.
nlohmann::json TakeSidebagRebuilds(int& count)
{
    nlohmann::json list = nlohmann::json::array();
    const std::vector<sweep_sidebag::Rebuild> rebuilds = sweep_sidebag::TakeRebuilds();
    count += static_cast<int>(rebuilds.size());
    for (const sweep_sidebag::Rebuild& rebuild : rebuilds)
        list.push_back({ { "oid", rebuild.oid }, { "cid", rebuild.cid }, { "name", rebuild.name },
            { "lost_options", rebuild.lostOptions } });
    return list;
}

// --- frame stepping ----------------------------------------------------------

void Stepper::ScanTransforms(int frame)
{
    std::vector<uint64_t> frameHashes;
    if (hashes != nullptr)
        frameHashes.reserve(allSWAloObjs.size());
    for (ALO* palo : allSWAloObjs)
    {
        if (palo == nullptr)
            continue;
        const XF& xf = palo->xf;
        float values[30];
        std::memcpy(values + 0, &xf.pos, sizeof(float) * 3);
        std::memcpy(values + 3, &xf.mat, sizeof(float) * 9);
        std::memcpy(values + 12, &xf.posWorld, sizeof(float) * 3);
        std::memcpy(values + 15, &xf.matWorld, sizeof(float) * 9);
        std::memcpy(values + 24, &xf.v, sizeof(float) * 3);
        std::memcpy(values + 27, &xf.w, sizeof(float) * 3);
        // World positions and orientations; velocities can be large for
        // a frame without being wrong, so they only have to be finite.
        const sweep::TransformScan scan =
            world.Check(reinterpret_cast<uintptr_t>(palo), frame, values + 12, 12);
        sweep::TransformScan velocity;
        sweep::ScanValues(velocity, values + 24, 6, 1.0e30f);
        const bool bad = scan.nonFinite != 0 || velocity.nonFinite != 0 || scan.absurd != 0;
        if (result != nullptr)
        {
            const bool inWorld = FIsLoInWorld(palo) != 0;
            if (inWorld)
            {
                result->nonFinite += scan.nonFinite + velocity.nonFinite;
                result->absurd += scan.absurd;
                result->parked += scan.parked;
                result->maxAbsCoordinate = (std::max)(result->maxAbsCoordinate, scan.maxAbs);
            }
            else if (bad)
                ++result->outOfWorldBad;
            std::string& first = inWorld ? firstBadObject : firstBadOutOfWorld;
            if (bad && first.empty())
            {
                char text[320];
                std::snprintf(text, sizeof(text),
                    "first bad transform%s: oid %d cid %d '%s' parent oid %d frame %d posWorld (%g %g %g) v (%g %g %g)",
                    inWorld ? "" : " (not in world)", static_cast<int>(palo->oid),
                    static_cast<int>(palo->pvtbasic->cid), palo->pchzName != nullptr ? palo->pchzName : "",
                    palo->paloParent != nullptr ? static_cast<int>(palo->paloParent->oid) : -1, frame,
                    values[12], values[13], values[14], values[24], values[25], values[26]);
                first = text;
            }
        }
        if (hashes != nullptr)
        {
            uint64_t hash = sweep::Fnv1a(values, sizeof(values));
            const short oid = palo->oid;
            hash = sweep::Fnv1a(&oid, sizeof(oid), hash);
            frameHashes.push_back(hash);
        }
    }
    if (hashes != nullptr)
        hashes->push_back(std::move(frameHashes));
}

// One RunGameFrame. frame < 0 is the load frame.
double Stepper::Step(int frame)
{
    bool cutCutscene = false;
    if (FCutscenePending())
    {
        // A queued movie plays in real time inside RunGameFrame.
        // PlayCutscene stops at its first poll when the window should
        // close; the flag is cleared again after the frame.
        glfwSetWindowShouldClose(g_gl.window, GLFW_TRUE);
        cutCutscene = true;
        if (result != nullptr)
            ++result->cutscenesCut;
    }
    if (frame >= 0 && g_transition.m_fPending != 0 && !allowTransitions)
    {
        // The level asked to leave (attract loop, demo, exit). Keep the
        // level under test loaded and record where it wanted to go.
        if (result != nullptr && result->transitionRequested.empty())
            result->transitionRequested = g_transition.m_worldPending + "@" + std::to_string(frame);
        g_transition.m_fPending = 0;
        g_transition.m_plevelPending = nullptr;
        g_transition.m_worldPending.clear();
    }

    // MarkClockTick clamps dt to [1/fps, 1/30]. Restarting the frame tick
    // here makes every frame take exactly the 1/fps minimum unless the
    // work before MarkClockTick takes longer than a frame (load frames).
    g_clock.tickFrame = TickNow();
    const auto start = Clock::now();
    sweep_game::RunFrame();
    glfwPollEvents();
    const double ms = MsSince(start);

    if (cutCutscene)
        glfwSetWindowShouldClose(g_gl.window, GLFW_FALSE);

    int errors = 0;
    GLenum traced = GL_NO_ERROR;
    if (const int taken = gltrace::TakeErrors(traced); taken != 0)
    {
        if (result != nullptr && result->glErrors == 0)
        {
            result->firstGlError = traced;
            result->firstGlErrorFrame = frame;
        }
        errors += taken;
    }
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
    {
        if (result != nullptr && result->glErrors == 0 && errors == 0)
        {
            result->firstGlError = error;
            result->firstGlErrorFrame = frame;
        }
        ++errors;
    }
    if (result != nullptr)
    {
        result->glErrors += errors;
        if (frame >= 0)
        {
            result->frameStats.Add(ms, frame);
            if (g_clock.dtReal != 1.0f / static_cast<float>(g_targetFrameRate))
                ++result->nonFixedDtFrames;
        }
    }
    if (frame >= 0 && g_psw != nullptr)
        ScanTransforms(frame);
    return ms;
}

// --- screenshots -------------------------------------------------------------

sweep::Image CaptureScene()
{
    sweep::Image image;
    image.width = g_gl.renderWidth;
    image.height = g_gl.renderHeight;
    image.rgba.resize(static_cast<size_t>(image.width) * image.height * 4);
    GLint previousRead = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousRead);
    // The scene target after RunGameFrame holds the resolved world and UI,
    // exactly what the screen shader then composites to the window.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl.fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previousRead));
    for (size_t i = 3; i < image.rgba.size(); i += 4)
        image.rgba[i] = 255;
    sweep::FlipRows(image);
    return image;
}

std::string SafeName(std::string name)
{
    for (char& ch : name)
        if (ch == '/' || ch == ':')
            ch = '_';
    return name;
}

// --- camera views ------------------------------------------------------------

void EnableManualCamera()
{
    // debug.cpp's "Manual Free Camera": CPMAN at manual-override priority
    // keeps pos/mat as set while no input arrives.
    SetCpmanCpmt(&g_pcm->cpman, CPMT_Truck);
    SetCmPolicy(g_pcm, CPP_ManualOverride, &g_pcm->cpman, nullptr, nullptr);
}

void DisableManualCamera()
{
    RevokeCmPolicy(g_pcm, static_cast<GRFRCP>(3), CPP_ManualOverride, &g_pcm->cpman, nullptr, nullptr);
}

struct View
{
    std::string name;
    glm::vec3 eye;
    glm::vec3 center;
};

std::vector<View> LevelViews(int& warpCount)
{
    std::vector<View> views;
    std::vector<LO*> warps(256, nullptr);
    // 0x205: search the whole world, report the full count.
    warpCount = CploFindSwObjectsByClass(g_psw, static_cast<GRFFSO>(0x205), CID_WARP, nullptr,
        static_cast<int>(warps.size()), warps.data());
    const int usable = (std::min)(warpCount, static_cast<int>(warps.size()));
    std::vector<glm::vec3> chosen;
    for (int i = 0; i < usable && views.size() < 3; ++i)
    {
        WARP* pwarp = static_cast<WARP*>(warps[i]);
        glm::vec3 pos{};
        glm::mat3 mat{ 1.0f };
        GetXfmPos(pwarp, &pos);
        GetXfmMat(pwarp, &mat);
        bool duplicate = false;
        for (const glm::vec3& other : chosen)
            duplicate |= glm::length(other - pos) < 100.0f;
        if (duplicate || !std::isfinite(pos.x + pos.y + pos.z))
            continue;
        chosen.push_back(pos);
        // TriggerWarp's own camera placement when there is no player.
        constexpr float kCameraDistance = 304.0f;
        views.push_back({ "warp" + std::to_string(pwarp->oid), pos - mat[0] * kCameraDistance, pos });
    }

    // At least one view around the player (or the camera's focus when there
    // is no player), more when the level has too few warps for the minimum.
    // The default view counts towards the minimum.
    glm::vec3 focus = g_pcm->pos;
    if (PO* ppo = PpoCur())
        focus = ppo->xf.posWorld;
    constexpr float kOrbit = 600.0f;
    for (int i = 0; i < 3 && (i == 0 || views.size() + 1 < static_cast<size_t>(sweep::kMinimumViews)); ++i)
    {
        const float angle = glm::radians(45.0f + 120.0f * static_cast<float>(i));
        const glm::vec3 eye = focus + glm::vec3(std::cos(angle) * kOrbit, std::sin(angle) * kOrbit, 0.45f * kOrbit);
        views.push_back({ "orbit" + std::to_string(i), eye, focus });
    }
    return views;
}

// --- mode: levels ------------------------------------------------------------

// Recurses until the stack overflows (the crash self-check below). The
// addition after the call keeps it from becoming a loop.
[[gnu::noinline]] int OverflowStack(int depth)
{
    volatile char frame[1024];
    frame[0] = static_cast<char>(depth);
    return OverflowStack(depth + 1) + frame[0];
}

int ModeLevels(const Options& options)
{
    BootGame(options);
    const fs::path shots = options.Get("--shots");
    const fs::path results = options.Get("--results");
    const int frames = options.Int("--frames", sweep::kRequiredFrames);
    std::vector<std::string> names;
    {
        std::ifstream list(options.Get("--list"));
        for (std::string line; std::getline(list, line);)
            if (!line.empty())
                names.push_back(line);
    }

    // Errors pending from Startup belong to no level.
    while (glGetError() != GL_NO_ERROR) {}

    for (const std::string& name : names)
    {
        std::fprintf(stderr, "=== LEVEL %s ===\n", name.c_str());
        std::printf("=== LEVEL %s ===\n", name.c_str());
        std::fflush(stderr);
        audit::NewLevel();

        sweep::LevelResult result;
        result.level = name;
        nlohmann::json viewList = nlohmann::json::array();
        LEVELINFO* plevel = PlevelinfoFromLevelName(name);
        Stepper stepper;
        stepper.result = &result;
        stepper.plevel = plevel;
        if (plevel == nullptr)
            result.note = "not in g_levelTable";
        else
        {
            g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
            result.loadMs = stepper.Step(-1);
            result.loaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
        }

        // Self-check of the crash path: CANE_SWEEP_CRASH_LEVEL names a level
        // that faults right after loading; with CANE_SWEEP_CRASH_KIND=
        // stack-overflow it overflows the stack instead.
        if (const char* crash = std::getenv("CANE_SWEEP_CRASH_LEVEL"); crash != nullptr && name == crash)
        {
            const char* kind = std::getenv("CANE_SWEEP_CRASH_KIND");
            if (kind != nullptr && std::strcmp(kind, "stack-overflow") == 0)
                std::fprintf(stderr, "[crash-check] %d\n", OverflowStack(0));
            std::raise(SIGSEGV);
        }

        if (result.loaded)
        {
            result.objects = static_cast<int>(allSWAloObjs.size());
            for (int frame = 0; frame < frames; ++frame)
                stepper.Step(frame);
            result.frames = frames;

            auto save = [&](const std::string& viewName)
            {
                const sweep::Image image = CaptureScene();
                const fs::path path = shots / SafeName(name) / (std::to_string(viewList.size()) + "_" + viewName + ".png");
                const bool written = sweep::WritePng(path, image);
                viewList.push_back({ { "name", viewName }, { "file", path.string() }, { "written", written },
                    { "width", image.width }, { "height", image.height }, { "mean_luma", sweep::MeanLuma(image) } });
            };
            save("default");

            int warpCount = 0;
            const std::vector<View> views = LevelViews(warpCount);
            result.warps = warpCount;
            EnableManualCamera();
            int frame = frames;
            for (const View& view : views)
            {
                SetCmLookAt(g_pcm, &view.eye, &view.center);
                SetCmCut(g_pcm, nullptr);
                for (int i = 0; i < 3; ++i)
                    stepper.Step(frame++);
                save(view.name);
            }
            DisableManualCamera();
            stepper.Step(frame++);
            result.views = static_cast<int>(viewList.size());
        }
        for (const std::string* text : { &stepper.firstBadObject, &stepper.firstBadOutOfWorld })
            if (!text->empty())
                result.note += (result.note.empty() ? "" : "; ") + *text;
        const nlohmann::json rebuilds = TakeSidebagRebuilds(result.sidebagRebuilds);

        nlohmann::json line = sweep::ToJson(result);
        line["view_list"] = viewList;
        line["sidebag_rebuild_list"] = rebuilds;
        std::ofstream(results, std::ios::app) << line.dump() << "\n";
    }

    ShutdownGame();
    return 0;
}

// --- mode: van-door ------------------------------------------------------------
// A Stealthy Approach opens with Sly in the van. The level script stores
// custom options on the van (1178, then 1179); 1178 is the one that later
// puts the door's volume button (VOLBTN 1860) back in the world, and the
// button moves the door's state machine (SM 1863) to its open state (911),
// after which Sly jumps out on his own. The check plays the level with no
// input until the door is open, keeps playing outside the van (frames and a
// few views), puts Sly on every clue bottle and then at the vault, and
// finally loads Prowling the Grounds from there and plays that.

constexpr OID kVanDoorButton = static_cast<OID>(1860);
constexpr OID kVanDoorSm = static_cast<OID>(1863);
constexpr OID kVanDoorOpen = static_cast<OID>(911);

LO* PloFromOid(OID oid)
{
    for (const std::shared_ptr<LO>& plo : allWorldObjs)
        if (plo != nullptr && plo->oid == oid)
            return plo.get();
    return nullptr;
}

// The current state of the first SMA running `oidSm`, or OID_Nil.
OID SmStateOid(OID oidSm)
{
    if (g_psw == nullptr)
        return OID_Nil;
    for (SMA* psma = static_cast<SMA*>(g_psw->dlSma.pvFirst); psma != nullptr;
         psma = static_cast<SMA*>(psma->dleSw.pvNext))
    {
        const SM* psm = psma->psm;
        if (psm == nullptr || psm->oid != oidSm)
            continue;
        if (psma->ismsCur >= 0 && psma->ismsCur < static_cast<int>(psm->asms.size()))
            return psm->asms[psma->ismsCur].oid;
        return OID_Nil;
    }
    return OID_Nil;
}

bool VanDoorOpen()
{
    LO* pbutton = PloFromOid(kVanDoorButton);
    return pbutton != nullptr && FIsLoInWorld(pbutton) != 0 && SmStateOid(kVanDoorSm) == kVanDoorOpen;
}

nlohmann::json PlayerPosition()
{
    PO* ppo = PpoCur();
    if (ppo == nullptr)
        return nullptr;
    return { ppo->xf.posWorld.x, ppo->xf.posWorld.y, ppo->xf.posWorld.z };
}

// debug.cpp's "Teleport Sly": TranslateAloToPos with the motion cleared.
bool TeleportSly(const glm::vec3& pos)
{
    if (g_pjt == nullptr || g_pjt->pvtalo == nullptr || g_pjt->pvtalo->pfnTranslateAloToPos == nullptr)
        return false;
    glm::vec3 target = pos;
    g_pjt->xf.v = glm::vec3(0.0f);
    g_pjt->xf.w = glm::vec3(0.0f);
    g_pjt->pvtalo->pfnTranslateAloToPos(g_pjt, &target);
    g_pjt->posWorldPrev = g_pjt->xf.posWorld;
    g_pjt->xf.v = glm::vec3(0.0f);
    g_pjt->xf.w = glm::vec3(0.0f);
    return true;
}

// The mouse button a key-bound button is pressed through. No default
// binding uses it, and the hidden window never gets real mouse input.
constexpr int kInjectMouseButton = GLFW_MOUSE_BUTTON_LAST;

// One button press or release through the game's own input layer:
// JOY::Update (next frame) reads a mouse-bound button from MOUSE::Button,
// which MOUSE::MouseButtonCallback, GLFW's mouse-button callback, sets. A
// button bound to a key is bound to kInjectMouseButton from the press until
// the release, which puts the player's binding back (one such button at a
// time); one bound to a mouse button is pressed on that button.
void InjectButton(JOY_BUTTON button, bool down)
{
    static int s_savedButton = -1;
    static int s_savedBinding = GLFW_KEY_UNKNOWN;
    int& binding = g_keyboardBindings[button];
    if (down && !FMouseBinding(binding))
    {
        s_savedButton = button;
        s_savedBinding = binding;
        binding = MouseBindingFromButton(kInjectMouseButton);
    }
    if (FMouseBinding(binding))
        MOUSE::MouseButtonCallback(g_gl.window, MouseButtonFromBinding(binding), down ? GLFW_PRESS : GLFW_RELEASE, 0);
    if (!down && s_savedButton == button)
    {
        binding = s_savedBinding;
        s_savedButton = -1;
    }
}

// Steps `frames` frames at a tour stop, swinging Sly's cane (square) twice so
// the cane ribbon (DrawSqtrm) is drawn; returns the frames with a ribbon.
int StepWithCaneSwings(Stepper& stepper, int& frame, int frames)
{
    int ribbonFrames = 0;
    for (int i = 0; i < frames; ++i)
    {
        if (i == 5 || i == 25)
            InjectButton(BTN_SQUARE, true);
        else if (i == 6 || i == 26)
            InjectButton(BTN_SQUARE, false);
        stepper.Step(frame++);
        if (g_pjt != nullptr && g_pjt->sqtrmCane.isqtrMic != g_pjt->sqtrmCane.isqtrMac)
            ++ribbonFrames;
    }
    return ribbonFrames;
}

nlohmann::json LevelSummary(const sweep::LevelResult& r)
{
    return { { "level", r.level }, { "loaded", r.loaded }, { "frames", r.frames }, { "gl_errors", r.glErrors },
        { "first_gl_error", r.firstGlError }, { "first_gl_error_frame", r.firstGlErrorFrame },
        { "nan_inf", r.nonFinite }, { "absurd", r.absurd }, { "sidebag_rebuilds", r.sidebagRebuilds },
        { "transition_requested", r.transitionRequested }, { "note", r.note } };
}

int ModeVanDoor(const Options& options)
{
    BootGame(options);
    const fs::path shots = options.Get("--shots");
    const int openDeadline = options.Int("--open-deadline", 900);
    const int afterOpen = options.Int("--after-open", 600);
    const int nextFrames = options.Int("--next-frames", sweep::kRequiredFrames);
    const std::string nextLevel = options.Get("--next-level", "Prowling the Grounds");
    nlohmann::json out;
    nlohmann::json shotList = nlohmann::json::array();
    while (glGetError() != GL_NO_ERROR) {}

    auto save = [&](const std::string& level, const std::string& name)
    {
        const fs::path path = shots / SafeName(level) / (std::to_string(shotList.size()) + "_" + name + ".png");
        const sweep::Image image = CaptureScene();
        shotList.push_back({ { "name", name }, { "file", path.string() }, { "written", sweep::WritePng(path, image) },
            { "mean_luma", sweep::MeanLuma(image) } });
    };

    // A Stealthy Approach until the door is open, then outside the van.
    const std::string level = "A Stealthy Approach";
    std::fprintf(stderr, "=== LEVEL %s ===\n", level.c_str());
    sweep::LevelResult result;
    result.level = level;
    Stepper stepper;
    stepper.result = &result;
    LEVELINFO* plevel = PlevelinfoFromLevelName(level);
    if (plevel != nullptr)
    {
        stepper.plevel = plevel;
        g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        result.loadMs = stepper.Step(-1);
        result.loaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
    }
    int frame = 0;
    int openFrame = -1;
    int glErrorsBeforeOpen = 0;
    int glErrorsBeforeTour = 0;
    nlohmann::json playerAtOpen = nullptr;
    if (result.loaded)
    {
        out["door_button_found"] = PloFromOid(kVanDoorButton) != nullptr;
        out["door_sm_state_at_load"] = static_cast<int>(SmStateOid(kVanDoorSm));
        for (; frame < openDeadline && openFrame < 0; ++frame)
        {
            stepper.Step(frame);
            if (VanDoorOpen())
                openFrame = frame;
        }
        glErrorsBeforeOpen = result.glErrors;
        out["door_sm_state_after_wait"] = static_cast<int>(SmStateOid(kVanDoorSm));
        if (openFrame >= 0)
        {
            playerAtOpen = PlayerPosition();
            save(level, "door_open");
            // Outside the van: the default camera follows Sly.
            const int end = frame + afterOpen;
            for (; frame < end; ++frame)
            {
                stepper.Step(frame);
                if ((end - frame) % 150 == 0)
                    save(level, "outside_f" + std::to_string(frame));
            }
            // A few views around Sly with the manual camera.
            int warpCount = 0;
            const std::vector<View> views = LevelViews(warpCount);
            EnableManualCamera();
            for (const View& view : views)
            {
                SetCmLookAt(g_pcm, &view.eye, &view.center);
                SetCmCut(g_pcm, nullptr);
                for (int i = 0; i < 10; ++i)
                    stepper.Step(frame++);
                save(level, view.name);
            }
            DisableManualCamera();
            for (int i = 0; i < 30; ++i)
                stepper.Step(frame++);
            glErrorsBeforeTour = result.glErrors;

            // Play through what the player does next: Sly is put on each clue
            // bottle in turn (it is collected, the HUD counter shows), then at
            // the vault, which opens once every clue is in, swinging his cane
            // at each stop.
            const int cluesAtStart = g_plsCur != nullptr ? g_plsCur->abitClue.m_cbitSet : -1;
            int cluesVisited = 0;
            nlohmann::json tour = nlohmann::json::array();
            std::vector<ALO*> clues;
            std::vector<VAULT*> vaults;
            for (const std::shared_ptr<LO>& plo : allWorldObjs)
            {
                if (plo == nullptr || plo->pvtbasic == nullptr || FIsLoInWorld(plo.get()) == 0)
                    continue;
                if (plo->pvtbasic->cid == CID_CLUE)
                    clues.push_back(static_cast<ALO*>(plo.get()));
                else if (plo->pvtbasic->cid == CID_VAULT)
                    vaults.push_back(static_cast<VAULT*>(plo.get()));
            }
            int ribbonFrames = 0;
            auto stop = [&](const std::string& name, const glm::vec3& pos, int frames)
            {
                const int before = result.glErrors;
                const bool moved = TeleportSly(pos);
                const int ribbon = StepWithCaneSwings(stepper, frame, frames);
                ribbonFrames += ribbon;
                tour.push_back({ { "stop", name }, { "teleported", moved }, { "gl_errors", result.glErrors - before },
                    { "cane_ribbon_frames", ribbon }, { "clues", g_plsCur != nullptr ? g_plsCur->abitClue.m_cbitSet : -1 } });
            };
            for (ALO* pclue : clues)
            {
                // The clue may already be gone (collected, or hidden by script).
                if (FIsLoInWorld(pclue) == 0)
                    continue;
                ++cluesVisited;
                stop("clue " + std::to_string(cluesVisited) + " (oid " + std::to_string(pclue->oid) + ")",
                    pclue->xf.posWorld, 45);
                if (cluesVisited % 5 == 0)
                    save(level, "clue_" + std::to_string(cluesVisited));
            }
            for (VAULT* pvault : vaults)
            {
                ALO* pgoad = reinterpret_cast<ALO*>(pvault->pvolbtnGoad);
                const glm::vec3 pos = pgoad != nullptr ? pgoad->xf.posWorld : pvault->xf.posWorld;
                stop("vault " + std::to_string(pvault->oid), pos, 300);
                save(level, "vault");
            }
            out["clues_in_level"] = g_psw->cclueAll;
            out["clues_found"] = static_cast<int>(clues.size());
            out["clues_visited"] = cluesVisited;
            out["clues_at_start"] = cluesAtStart;
            out["clues_collected"] = g_plsCur != nullptr ? g_plsCur->abitClue.m_cbitSet : -1;
            out["vaults"] = static_cast<int>(vaults.size());
            out["tour"] = tour;
            out["tour_cane_ribbon_frames"] = ribbonFrames;
        }
        else
            glErrorsBeforeTour = result.glErrors;
        result.frames = frame;
    }
    const nlohmann::json stealthyRebuilds = TakeSidebagRebuilds(result.sidebagRebuilds);
    out["door_opened"] = openFrame >= 0;
    out["open_frame"] = openFrame;
    out["frames_after_open"] = openFrame >= 0 ? frame - openFrame - 1 : 0;
    out["gl_errors_before_open"] = glErrorsBeforeOpen;
    out["gl_errors_after_open"] = glErrorsBeforeTour - glErrorsBeforeOpen;
    // The tour swings the cane: a cel border drawn after the ribbon
    // (DrawSqtr leaves no vertex array bound) once raised GL errors here
    // (theclub654/ProjectCane#62).
    out["tour_gl_errors"] = result.glErrors - glErrorsBeforeTour;
    out["player_at_open"] = playerAtOpen;
    out["player_at_end"] = PlayerPosition();
    out["stealthy"] = LevelSummary(result);
    out["stealthy"]["sidebag_rebuild_list"] = stealthyRebuilds;

    // The next level, loaded from inside A Stealthy Approach.
    std::fprintf(stderr, "=== LEVEL %s (after %s) ===\n", nextLevel.c_str(), level.c_str());
    sweep::LevelResult next;
    next.level = nextLevel;
    Stepper nextStepper;
    nextStepper.result = &next;
    LEVELINFO* pnext = PlevelinfoFromLevelName(nextLevel);
    if (result.loaded && pnext != nullptr)
    {
        nextStepper.plevel = pnext;
        g_transition.Set(pnext, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        next.loadMs = nextStepper.Step(-1);
        next.loaded = g_transition.m_plevelCurrent == pnext && g_psw != nullptr;
        if (next.loaded)
        {
            for (int i = 0; i < nextFrames; ++i)
                nextStepper.Step(i);
            next.frames = nextFrames;
            save(nextLevel, "default");
        }
    }
    const nlohmann::json nextRebuilds = TakeSidebagRebuilds(next.sidebagRebuilds);
    out["next"] = LevelSummary(next);
    out["next"]["sidebag_rebuild_list"] = nextRebuilds;
    out["shots"] = shotList;
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

// --- mode: hub-tour -------------------------------------------------------------
// Prowling the Grounds loaded on its own (a fresh game), then Sly is put on
// each level door (EXIT; the level change itself is held back, as in every
// check), volume button and tutorial trigger (TN) in turn, so the doors'
// exit scripts run and Bentley's TV dialogs play; he swings his cane at
// each stop (the cane ribbon then a cel border pass). The TV speaker is a glob
// drawn in the foreground pass, after the blip pass; the sampler audit and
// the driver's "unloadable" warning catch a material unit left without a
// texture there.

int ModeHubTour(const Options& options)
{
    BootGame(options);
    const std::string name = options.Get("--level", "Prowling the Grounds");
    const int framesPerStop = options.Int("--per-stop", 90);
    nlohmann::json out;
    while (glGetError() != GL_NO_ERROR) {}
    std::fprintf(stderr, "=== LEVEL %s ===\n", name.c_str());
    sweep::LevelResult result;
    result.level = name;
    Stepper stepper;
    stepper.result = &result;
    LEVELINFO* plevel = PlevelinfoFromLevelName(name);
    nlohmann::json tour = nlohmann::json::array();
    int ribbonFrames = 0;
    if (plevel != nullptr)
    {
        stepper.plevel = plevel;
        g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        result.loadMs = stepper.Step(-1);
        result.loaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
    }
    int frame = 0;
    if (result.loaded)
    {
        for (; frame < 60; ++frame)
            stepper.Step(frame);
        std::vector<ALO*> stops;
        for (const std::shared_ptr<LO>& plo : allWorldObjs)
            if (plo != nullptr && plo->pvtbasic != nullptr &&
                (plo->pvtbasic->cid == CID_EXIT || plo->pvtbasic->cid == CID_VOLBTN || plo->pvtbasic->cid == CID_TN) &&
                FIsLoInWorld(plo.get()) != 0)
                stops.push_back(static_cast<ALO*>(plo.get()));
        for (ALO* palo : stops)
        {
            const int before = result.glErrors;
            const bool moved = TeleportSly(palo->xf.posWorld);
            const int ribbon = StepWithCaneSwings(stepper, frame, framesPerStop);
            ribbonFrames += ribbon;
            tour.push_back({ { "stop", "oid " + std::to_string(palo->oid) + " cid " +
                std::to_string(static_cast<int>(palo->pvtbasic->cid)) }, { "teleported", moved },
                { "cane_ribbon_frames", ribbon },
                { "gl_errors", result.glErrors - before } });
        }
        result.frames = frame;
    }
    TakeSidebagRebuilds(result.sidebagRebuilds);
    out["level"] = LevelSummary(result);
    out["stops"] = static_cast<int>(tour.size());
    out["cane_ribbon_frames"] = ribbonFrames;
    out["tour"] = tour;
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

// --- mode: charm-exit ----------------------------------------------------------
// A charm awarded in Prowling the Grounds swirls towards the player's charm
// bracelet (DPRIZES_Swirl). Entering a level door runs the door's exit script,
// which re-parents Sly and so takes him off the player list; PpziCur() then
// returns the default PZI, whose charm links are null. If that happens inside
// the object update while the charm still swirls, the solve that follows in
// the same frame (SolveSw -> ProjectDprizeTransform) reads them
// (theclub654/ProjectCane#57). Cases
// (--case): in-update (the crash: exit 10 frames after the charm, from inside
// the object update, right after the charm's own update), in-stick (the same
// while the charm sticks to the bracelet, DPRIZES_Stick: the Stick projection
// and then the Stick update read the links), no-charm, after-swirl (exit once
// the charm is collected) and outside (TriggerExit between frames, outside
// the update).

// Runs `after` once, right after `palo`'s own update in the next object
// update (UpdateAloHierarchy calls pvtalo->pfnUpdateAlo): the object gets a
// copy of its class's function table with that one slot wrapped, and its own
// table back once the hook has run.
template <typename VTX>
struct UpdateHook
{
    static inline VTX copy{};
    static inline VTX* original = nullptr;
    static inline VTX** slot = nullptr;
    static inline void (*update)(ALO*, float) = nullptr;
    static inline void (*after)() = nullptr;

    static void Update(ALO* palo, float dt)
    {
        update(palo, dt);
        *slot = original;
        if (void (*fn)() = after)
        {
            after = nullptr;
            fn();
        }
    }

    static void Install(VTX*& vt, void (*fn)())
    {
        original = vt;
        slot = &vt;
        copy = *vt;
        // An ALO class's table (VTJT, VTCHARM in basic.h) starts with
        // VTALO's slots in the same order, typed for the class, and the
        // object's pvtalo is the same pointer as its pvtjt/pvtcharm: the
        // game's UpdateAloHierarchy calls pfnUpdateAlo through that VTALO
        // view, so it is the slot to wrap.
        VTALO* view = reinterpret_cast<VTALO*>(&copy);
        update = view->pfnUpdateAlo;
        view->pfnUpdateAlo = Update;
        after = fn;
        vt = &copy;
    }
};

EXIT* s_pexitHooked = nullptr;
// TriggerExit ran, from the hook or (case "outside") between frames.
bool s_fExitTriggered = false;

void TriggerHookedExit()
{
    s_fExitTriggered = true;
    std::fprintf(stderr, "[charm-exit] exit %d triggered inside the object update\n", static_cast<int>(s_pexitHooked->oid));
    TriggerExit(s_pexitHooked);
}

int ModeCharmExit(const Options& options)
{
    BootGame(options);
    const std::string level = options.Get("--level", "Prowling the Grounds");
    const std::string exitLevel = options.Get("--exit-level", "Treasure in the Depths");
    const std::string mode = options.Get("--case", "in-update");
    const int charmAt = mode == "no-charm" ? -1 : options.Int("--charm-at", 100);
    // in-stick: 4 frames after the charm reaches DPRIZES_Stick (set below).
    int exitAt = options.Int("--exit-at", mode == "after-swirl" ? 400 : mode == "in-stick" ? -1 : 110);
    const int frames = options.Int("--frames", mode == "in-stick" ? 450 : exitAt + 200);
    nlohmann::json out = { { "case", mode }, { "charm_at", charmAt }, { "exit_at", exitAt } };
    std::fprintf(stderr, "=== LEVEL %s (%s) ===\n", level.c_str(), mode.c_str());
    sweep::LevelResult result;
    result.level = level;
    Stepper stepper;
    stepper.result = &result;
    LEVELINFO* plevel = PlevelinfoFromLevelName(level);
    if (plevel != nullptr)
    {
        stepper.plevel = plevel;
        g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        result.loadMs = stepper.Step(-1);
        result.loaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
    }
    EXIT* pexit = nullptr;
    DPRIZE* pcharm = nullptr;
    nlohmann::json states = nlohmann::json::array();
    nlohmann::json positions = nlohmann::json::array();
    int lastState = -100;
    int stateAtExit = -100;
    bool playerAfterExit = true;
    if (result.loaded)
    {
        for (const std::shared_ptr<LO>& plo : allWorldObjs)
            if (plo != nullptr && plo->pvtbasic != nullptr && plo->pvtbasic->cid == CID_EXIT && FIsLoInWorld(plo.get()) != 0)
            {
                EXIT* candidate = static_cast<EXIT*>(static_cast<ALO*>(plo.get()));
                if (candidate->plevel != nullptr && candidate->plevel->levelName == exitLevel)
                    pexit = candidate;
            }
        out["exit_oid"] = pexit != nullptr ? static_cast<int>(pexit->oid) : -1;
        for (int frame = 0; frame < frames && pexit != nullptr; ++frame)
        {
            if (frame == charmAt)
            {
                std::vector<DPRIZE*> before;
                for (const std::shared_ptr<LO>& plo : allWorldObjs)
                    if (plo != nullptr && plo->pvtbasic != nullptr && plo->pvtbasic->cid == CID_CHARM && FIsLoInWorld(plo.get()) != 0)
                        before.push_back(static_cast<DPRIZE*>(static_cast<ALO*>(plo.get())));
                CreateSwCharm(g_psw);
                for (const std::shared_ptr<LO>& plo : allWorldObjs)
                    if (plo != nullptr && plo->pvtbasic != nullptr && plo->pvtbasic->cid == CID_CHARM && FIsLoInWorld(plo.get()) != 0)
                    {
                        DPRIZE* p = static_cast<DPRIZE*>(static_cast<ALO*>(plo.get()));
                        if (std::find(before.begin(), before.end(), p) == before.end() && p->dprizes == DPRIZES_Swirl)
                            pcharm = p;
                    }
                std::fprintf(stderr, "[charm-exit] charm %s at frame %d\n", pcharm != nullptr ? "swirling" : "NOT created", frame);
            }
            if (frame == exitAt)
            {
                stateAtExit = pcharm != nullptr ? static_cast<int>(pcharm->dprizes) : -1;
                s_pexitHooked = pexit;
                if (mode == "outside")
                {
                    s_fExitTriggered = true;
                    TriggerExit(pexit);
                }
                // In the crash case the exit fires right after the swirling
                // charm's own update, before SolveSw projects it; otherwise
                // right after Sly's.
                else if (pcharm != nullptr && (pcharm->dprizes == DPRIZES_Swirl || pcharm->dprizes == DPRIZES_Stick))
                    UpdateHook<VTCHARM>::Install(pcharm->pvtcharm, TriggerHookedExit);
                else if (g_pjt != nullptr)
                    UpdateHook<VTJT>::Install(g_pjt->pvtjt, TriggerHookedExit);
            }
            stepper.Step(frame);
            if (frame == exitAt)
                playerAfterExit = PpoCur() != nullptr;
            const int state = pcharm != nullptr ? static_cast<int>(pcharm->dprizes) : -1;
            // Where the charm is around the exit: the frame step must stay
            // small (no snap to the world origin with the player gone).
            if (pcharm != nullptr && exitAt >= 0 && frame >= exitAt - 1 && frame <= exitAt + 3)
                positions.push_back({ { "frame", frame }, { "pos", { pcharm->xf.posWorld.x, pcharm->xf.posWorld.y,
                    pcharm->xf.posWorld.z } } });
            if (mode == "in-stick" && exitAt < 0 && state == DPRIZES_Stick)
                exitAt = frame + 4;
            if (state != lastState)
            {
                states.push_back({ { "frame", frame }, { "state", state } });
                lastState = state;
            }
            result.frames = frame + 1;
        }
    }
    TakeSidebagRebuilds(result.sidebagRebuilds);
    out["level"] = LevelSummary(result);
    out["exit_found"] = pexit != nullptr;
    out["charm_created"] = pcharm != nullptr;
    out["charm_state_at_exit"] = stateAtExit;
    out["charm_states"] = states;
    out["charm_final_state"] = lastState;
    out["charm_positions"] = positions;
    out["player_after_exit"] = playerAfterExit;
    out["exit_triggered"] = s_fExitTriggered;
    out["exit_at"] = exitAt;
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

// --- mode: hash (determinism) -----------------------------------------------

int ModeHash(const Options& options)
{
    BootGame(options);
    const std::string name = options.Get("--level");
    const int frames = options.Int("--frames", sweep::kRequiredFrames);
    LEVELINFO* plevel = PlevelinfoFromLevelName(name);
    sweep::LevelResult result;
    result.level = name;
    sweep::FrameHashes hashes;
    Stepper stepper;
    stepper.result = &result;
    stepper.plevel = plevel;
    stepper.hashes = &hashes;
    if (plevel != nullptr)
    {
        g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        result.loadMs = stepper.Step(-1);
        result.loaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
    }
    nlohmann::json objects = nlohmann::json::array();
    if (result.loaded)
    {
        for (ALO* palo : allSWAloObjs)
            objects.push_back({ { "oid", palo != nullptr ? palo->oid : -1 },
                { "name", palo != nullptr && palo->pchzName != nullptr ? palo->pchzName : "" } });
        for (int frame = 0; frame < frames; ++frame)
            stepper.Step(frame);
        result.frames = frames;
    }
    sweep::WriteFrameHashes(options.Get("--hashes"), hashes);
    nlohmann::json out = sweep::ToJson(result);
    out["object_list"] = objects;
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return result.loaded ? 0 : 1;
}

// --- mode: cutscene ----------------------------------------------------------

struct MediaInfo
{
    double seconds = 0.0;
    long long packets = 0;
    double fps = 0.0;
};

// Reference numbers for the movie, read straight from the container.
MediaInfo ProbeVideo(const fs::path& path)
{
    MediaInfo info;
    AVFormatContext* format = nullptr;
    if (avformat_open_input(&format, path.string().c_str(), nullptr, nullptr) < 0)
        return info;
    if (avformat_find_stream_info(format, nullptr) >= 0)
    {
        const int stream = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (stream >= 0)
        {
            info.fps = av_q2d(av_guess_frame_rate(format, format->streams[stream], nullptr));
            AVPacket* packet = av_packet_alloc();
            while (av_read_frame(format, packet) >= 0)
            {
                info.packets += packet->stream_index == stream ? 1 : 0;
                av_packet_unref(packet);
            }
            av_packet_free(&packet);
            if (info.fps > 0.0)
                info.seconds = static_cast<double>(info.packets) / info.fps;
        }
    }
    avformat_close_input(&format);
    return info;
}

int ModeCutscene(const Options& options)
{
    BootGame(options);
    const int index = options.Int("--index", -1);
    const double maxSeconds = std::atof(options.Get("--max-seconds", "0").c_str());
    nlohmann::json out;
    if (index < 0 || index >= CUTSCENE_COUNT)
    {
        out["error"] = "bad index";
        WriteJson(options.Get("--result"), out);
        return 1;
    }
    const CUTSCENEINFO& info = g_cutsceneTable[index];
    const MediaInfo media = ProbeVideo(cane::DataDirectory() / "Cutscenes" / info.videoFileName);

    std::atomic_bool done = false;
    std::atomic_bool capped = false;
    std::atomic_bool stalled = false;
    // Always armed: the cap when one is given, else 1.5 x the movie's length
    // + 60 s, so a movie that stops making progress ends here (with its stacks
    // in the log) instead of at the parent's backstop. Awake time, so a
    // system sleep never trips it.
    const double watchdogSeconds = sweep::CutsceneWatchdogSeconds(media.seconds, maxSeconds);
    std::thread watchdog([&]
    {
        if (watchdogSeconds <= 0.0)
            return;
        const double deadline = sweep::AwakeSeconds() + watchdogSeconds;
        while (!done.load() && sweep::AwakeSeconds() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (!done.load())
        {
            if (maxSeconds > 0.0)
                capped = true;
            else
            {
                stalled = true;
                std::fprintf(stderr, "[ERROR] [Sweep] cutscene still playing after %.0f s (movie %.1f s); stopping it\n",
                    watchdogSeconds, media.seconds);
                sweep::SampleProcessToLog(static_cast<int>(getpid()), 3, std::string("/dev/stderr"));
            }
            // Documented as callable from any thread. PlayCutscene treats it
            // as the user closing the window and stops cleanly.
            glfwSetWindowShouldClose(g_gl.window, GLFW_TRUE);
        }
    });

    const auto start = Clock::now();
    QueueCutscene(info.identityId);
    const bool queued = FCutscenePending();
    ExecutePendingCutscenes();
    const double seconds = MsSince(start) / 1000.0;
    done = true;
    watchdog.join();
    glfwSetWindowShouldClose(g_gl.window, GLFW_FALSE);

    int glErrors = 0;
    while (glGetError() != GL_NO_ERROR)
        ++glErrors;

    out = {
        { "index", index }, { "video", info.videoFileName }, { "audio", info.audioFileName },
        { "queued", queued }, { "elapsed_s", seconds }, { "capped", capped.load() },
        { "stalled", stalled.load() }, { "watchdog_s", watchdogSeconds },
        { "max_seconds", maxSeconds }, { "media_seconds", media.seconds }, { "media_packets", media.packets },
        { "media_fps", media.fps }, { "gl_errors", glErrors }
    };
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

// --- mode: audio -------------------------------------------------------------

// The retail music ids (MUSID_34..MUSID_68.mus) lie outside the shared
// MUSID enum's value range (MUSID_Max is 16 and the enum has no fixed
// underlying type), so loading a MUSID object that holds one is undefined
// behaviour (UBSan invalid-enum-load; theclub654/ProjectCane#66). The sweep
// keeps ids as int and reads the stream's id through its bytes, so it never
// loads such a MUSID from memory.
static_assert(sizeof(MUSID) == sizeof(int), "MUSID is read through its int bytes");

// The shared music functions take a MUSID, so the id is cast to one here.
// This cast is the undefined behaviour #66 describes (a value outside the
// enum's range), the same the game commits for these ids; the sweep does it
// only to call the game's own functions with the game's own ids.
MUSID MusidArgument(int id)
{
    return static_cast<MUSID>(id);
}

int StreamMusidId()
{
    int id = 0;
    std::memcpy(&id, &s_musicStream.musid, sizeof(id));
    return id;
}

int ModeAudio(const Options& options)
{
    BootGame(options);
    const fs::path data = cane::DataDirectory();
    // The bank finders search beside the loaded world's "Worlds" folder.
    file = (data / "Worlds" / "Splash" / "Splash.brx").string();
    const int vagEvery = options.Int("--vag-every", 50);

    nlohmann::json out;
    nlohmann::json failures = nlohmann::json::array();

    // Music: every MUSID_xx.mus through PreloadMusidSong / StartMusidSong.
    int musicTotal = 0, musicLoaded = 0, musicStarted = 0;
    std::vector<fs::path> music;
    for (const auto& entry : fs::directory_iterator(data / "Worlds" / "Music"))
        if (entry.path().extension() == ".mus")
            music.push_back(entry.path());
    std::sort(music.begin(), music.end());
    for (const fs::path& path : music)
    {
        ++musicTotal;
        const std::string stem = path.stem().string(); // MUSID_55
        const int id = static_cast<int>(std::strtol(stem.substr(stem.find('_') + 1).c_str(), nullptr, 16));
        std::fprintf(stderr, "=== LEVEL music %s ===\n", stem.c_str());
        PreloadMusidSong(MusidArgument(id));
        const bool loaded = s_musicStream.state == MusicState::Loaded && StreamMusidId() == id;
        musicLoaded += loaded ? 1 : 0;
        StartMusidSong(MusidArgument(id));
        for (int i = 0; i < 10; ++i)
        {
            ServiceAudioDevice();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        const bool started = s_musicStream.state == MusicState::Playing && StreamMusidId() == id;
        musicStarted += started ? 1 : 0;
        if (!loaded || !started)
            failures.push_back(stem + (loaded ? ": did not start" : ": did not load"));
        KillMusic();
    }

    // Effect banks: SbpEnsureBank for every IBNK.
    int banksLoaded = 0;
    nlohmann::json banks = nlohmann::json::array();
    for (int ibnk = 0; ibnk < static_cast<int>(kEffectBankCount); ++ibnk)
    {
        std::fprintf(stderr, "=== LEVEL bank %d ===\n", ibnk);
        SoundBankPtr bank = SbpEnsureBank(ibnk);
        const bool ok = bank != nullptr && !s_effectBanks[ibnk]->sounds.empty();
        banksLoaded += ok ? 1 : 0;
        banks.push_back({ { "ibnk", ibnk }, { "file", fs::path(kEffectBankNames[ibnk]).string() },
            { "loaded", ok }, { "sounds", ok ? s_effectBanks[ibnk]->sounds.size() : 0 } });
        if (!ok)
            failures.push_back("bank " + std::to_string(ibnk) + " did not load");
    }

    // WAVs: every extracted sound (dialogue beside each level, Sounds/) and
    // movie track through LoadPcmWave, and every vagEvery-th one through the
    // streaming VAG player.
    std::vector<fs::path> waves;
    for (const fs::path& root : { data / "Worlds", data / "Cutscenes" })
    {
        std::error_code error;
        for (auto it = fs::recursive_directory_iterator(root, error); !error && it != fs::recursive_directory_iterator(); it.increment(error))
            if (it->is_regular_file() && it->path().extension() == ".wav")
                waves.push_back(it->path());
    }
    std::sort(waves.begin(), waves.end());
    int wavLoaded = 0, vagTried = 0, vagStarted = 0;
    std::fprintf(stderr, "=== LEVEL wav ===\n");
    for (size_t i = 0; i < waves.size(); ++i)
    {
        SoundWaveFormat format;
        std::vector<uint8_t> pcm;
        const bool ok = LoadPcmWave(waves[i], format, pcm) && format.channels > 0 &&
            format.samplesPerSec > 0 && !pcm.empty();
        wavLoaded += ok ? 1 : 0;
        if (!ok)
            failures.push_back("wav did not load: " + fs::relative(waves[i], data).string());
        if (vagEvery > 0 && i % static_cast<size_t>(vagEvery) == 0)
        {
            ++vagTried;
            PreloadVag(waves[i]);
            const auto deadline = Clock::now() + std::chrono::seconds(10);
            while (FPauseForVag() && Clock::now() < deadline)
            {
                ServiceAudioDevice();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            const bool started = FVagPlaying() != 0;
            vagStarted += started ? 1 : 0;
            if (!started)
                failures.push_back("vag did not start: " + fs::relative(waves[i], data).string());
            StopVagAndWaitForFade();
        }
    }

    out = {
        { "music_total", musicTotal }, { "music_loaded", musicLoaded }, { "music_started", musicStarted },
        { "banks_total", static_cast<int>(kEffectBankCount) }, { "banks_loaded", banksLoaded }, { "banks", banks },
        { "wav_total", static_cast<int>(waves.size()) }, { "wav_loaded", wavLoaded },
        { "vag_tried", vagTried }, { "vag_started", vagStarted }, { "failures", failures }
    };
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

// --- mode: saves -------------------------------------------------------------

constexpr int kSaveSlot = 2;

nlohmann::json GsSummary(const GS& gs)
{
    return { { "gameWorldCur", static_cast<int>(gs.gameWorldCur) }, { "worldLevelCur", static_cast<int>(gs.worldLevelCur) },
        { "clife", gs.clife }, { "ccharm", gs.ccharm }, { "ccoin", gs.ccoin }, { "dt", gs.dt },
        { "cbThis", gs.cbThis }, { "nChecksum", gs.nChecksum } };
}

std::vector<uint8_t> Bytes(const GS& gs)
{
    const auto* p = reinterpret_cast<const uint8_t*>(&gs);
    return std::vector<uint8_t>(p, p + sizeof(GS));
}

int ModeSaveWrite(const Options& options)
{
    BootGame(options);
    nlohmann::json out;
    LEVELINFO* plevel = PlevelinfoFromLevelName(options.Get("--level", sweep_game::kDefaultLevel));
    Stepper stepper;
    if (plevel != nullptr)
    {
        g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        stepper.Step(-1);
        for (int frame = 0; frame < 60; ++frame)
            stepper.Step(frame);
    }
    out["level_loaded"] = g_psw != nullptr && g_transition.m_plevelCurrent == plevel;

    // Distinctive values on top of the live game state from the level.
    g_pgsCur->clife = 7;
    g_pgsCur->ccharm = 2;
    g_pgsCur->ccoin = 42;
    g_pgsCur->dt += 123.25f;
    g_saveData.pgsCurrentSave = &g_saveData.saveData[kSaveSlot];
    out["saved"] = SaveCurrentGameToDisk(&g_saveData);
    const GS& written = g_saveData.saveData[kSaveSlot];
    out["written"] = GsSummary(written);
    const fs::path slotFile = cane::UserDirectory() / "Saves" / ("slot" + std::to_string(kSaveSlot + 1) + ".sav");
    out["slot_file"] = slotFile.string();
    out["slot_file_exists"] = fs::is_regular_file(slotFile);
    const std::vector<uint8_t> bytes = Bytes(written);
    std::ofstream(options.Get("--expected"), std::ios::binary | std::ios::trunc)
        .write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

int ModeSaveRead(const Options& options)
{
    // StartupSaveData (inside Startup) reads every slot from the user folder.
    BootGame(options);
    nlohmann::json out;
    std::vector<uint8_t> expected;
    {
        std::ifstream in(options.Get("--expected"), std::ios::binary);
        expected.assign(std::istreambuf_iterator<char>(in), {});
    }
    const GS& loaded = g_saveData.saveData[kSaveSlot];
    const std::vector<uint8_t> actual = Bytes(loaded);
    out["expected_size"] = expected.size();
    out["loaded"] = GsSummary(loaded);
    out["bytes_equal"] = expected == actual;
    size_t firstDiff = 0;
    while (firstDiff < expected.size() && firstDiff < actual.size() && expected[firstDiff] == actual[firstDiff])
        ++firstDiff;
    out["first_byte_difference"] = firstDiff < expected.size() ? static_cast<long long>(firstDiff) : -1;
    out["attract_save_is_slot"] = g_saveData.pgsAttractSave == &g_saveData.saveData[kSaveSlot];

    // And into the live game state, as the load menu does.
    g_saveData.pgsCurrentSave = &g_saveData.saveData[kSaveSlot];
    out["load_current_save"] = LoadCurrentSave(&g_saveData) != 0;
    out["live_equal"] = Bytes(*g_pgsCur) == expected;
    out["live"] = GsSummary(*g_pgsCur);
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

// --- mode: display -----------------------------------------------------------

struct DisplayCheck
{
    nlohmann::json steps = nlohmann::json::array();
    bool allPassed = true;
};

nlohmann::json CheckRenderState(const char* label, int expectRenderW, int expectRenderH,
    int expectPresentW, int expectPresentH, int glErrors, bool callbackDelivered)
{
    GLfloat viewport[2] = { -1.0f, -1.0f };
    GLfloat lineWidth = -1.0f;
    if (glslCelViewportPx >= 0)
        glGetUniformfv(glCelBorderShader.ID, glslCelViewportPx, viewport);
    if (glslCelLineWidthPx >= 0)
        glGetUniformfv(glCelBorderShader.ID, glslCelLineWidthPx, &lineWidth);
    GLint textureW = 0, textureH = 0;
    glBindTexture(GL_TEXTURE_2D, g_gl.fbc);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureH);
    glBindTexture(GL_TEXTURE_2D, 0);
    const GLenum readError = glGetError();
    int fbW = 0, fbH = 0, winW = 0, winH = 0;
    glfwGetFramebufferSize(g_gl.window, &fbW, &fbH);
    glfwGetWindowSize(g_gl.window, &winW, &winH);

    const float expectedLine = 4.125f * static_cast<float>(g_gl.renderHeight) /
        static_cast<float>((std::max)(g_gl.presentHeight, 1));
    const bool renderOk = g_gl.renderWidth == expectRenderW && g_gl.renderHeight == expectRenderH;
    const bool presentOk = g_gl.presentWidth == expectPresentW && g_gl.presentHeight == expectPresentH;
    const bool textureOk = textureW == g_gl.renderWidth && textureH == g_gl.renderHeight;
    const bool viewportOk = viewport[0] == static_cast<float>(g_gl.renderWidth) &&
        viewport[1] == static_cast<float>(g_gl.renderHeight);
    const bool lineOk = std::fabs(lineWidth - expectedLine) < 1e-4f;
    const bool passed = renderOk && presentOk && textureOk && viewportOk && lineOk &&
        glErrors == 0 && readError == GL_NO_ERROR;
    return {
        { "step", label }, { "passed", passed }, { "window", { winW, winH } }, { "framebuffer", { fbW, fbH } },
        { "render", { g_gl.renderWidth, g_gl.renderHeight } }, { "expected_render", { expectRenderW, expectRenderH } },
        { "present", { g_gl.presentWidth, g_gl.presentHeight } }, { "expected_present", { expectPresentW, expectPresentH } },
        { "scene_texture", { textureW, textureH } }, { "cel_viewport_uniform", { viewport[0], viewport[1] } },
        { "cel_line_width_uniform", lineWidth }, { "expected_line_width", expectedLine },
        { "gl_errors", glErrors }, { "callback_delivered", callbackDelivered }
    };
}

// NSApplicationHideDock | NSApplicationHideMenuBar (AppKit's NSApplication.h).
constexpr unsigned long kHideDockAndMenuBar = (1ul << 1) | (1ul << 3);

// NSApp.presentationOptions: what macOS applies while this app is active.
// This background-only process is never active, so they are read back here
// and never take effect on the user's screen.
unsigned long PresentationOptions()
{
    objc_object* app = reinterpret_cast<objc_object* (*)(objc_object*, objc_selector*)>(objc_msgSend)(
        objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
    if (app == nullptr)
        return ~0ul;
    return reinterpret_cast<unsigned long (*)(objc_object*, objc_selector*)>(objc_msgSend)(
        app, sel_registerName("presentationOptions"));
}

sweep_display::DisplayMode MainDisplayMode()
{
    sweep_display::DisplayMode result;
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(CGMainDisplayID());
    if (mode == nullptr)
        return result;
    result.valid = true;
    result.width = static_cast<int>(CGDisplayModeGetWidth(mode));
    result.height = static_cast<int>(CGDisplayModeGetHeight(mode));
    result.pixelWidth = static_cast<int>(CGDisplayModeGetPixelWidth(mode));
    result.pixelHeight = static_cast<int>(CGDisplayModeGetPixelHeight(mode));
    result.refreshRate = CGDisplayModeGetRefreshRate(mode);
    CGDisplayModeRelease(mode);
    return result;
}

nlohmann::json Describe(const sweep_display::DisplayMode& mode)
{
    return { { "valid", mode.valid }, { "points", { mode.width, mode.height } },
        { "pixels", { mode.pixelWidth, mode.pixelHeight } }, { "refresh", mode.refreshRate } };
}

int ModeDisplay(const Options& options)
{
    BootGame(options);
    const fs::path shots = options.Get("--shots");
    const bool fullscreen = options.Flag("--fullscreen");
    LEVELINFO* plevel = PlevelinfoFromLevelName(options.Get("--level", sweep_game::kDefaultLevel));
    sweep::LevelResult result;
    Stepper stepper;
    stepper.result = &result;
    nlohmann::json out;
    DisplayCheck check;
    int frame = 0;
    if (plevel != nullptr)
    {
        g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
        stepper.Step(-1);
    }
    out["level_loaded"] = g_psw != nullptr && g_transition.m_plevelCurrent == plevel;
    for (int i = 0; i < 30; ++i)
        stepper.Step(frame++);

    auto frames = [&](int count)
    {
        const int before = result.glErrors;
        for (int i = 0; i < count; ++i)
            stepper.Step(frame++);
        return result.glErrors - before;
    };
    auto record = [&](nlohmann::json step, const std::string& shot)
    {
        if (!shot.empty())
            sweep::WritePng(shots / "display" / (shot + ".png"), CaptureScene());
        check.allPassed = check.allPassed && step.value("passed", false);
        check.steps.push_back(std::move(step));
    };
    auto pumpUntilFramebuffer = [&](int w, int h)
    {
        // Resizes reach FrameBufferSizeCallBack through glfwPollEvents.
        for (int i = 0; i < 50; ++i)
        {
            glfwPollEvents();
            if (g_gl.outputWidth == w && g_gl.outputHeight == h)
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    };

    // 1. Window resizes. The display here is 1x, so framebuffer == window.
    const int sizes[][2] = { { 1280, 720 }, { 800, 600 }, { 1600, 900 }, { 640, 480 }, { 1024, 768 }, { 1280, 720 } };
    for (const auto& size : sizes)
    {
        glfwSetWindowSize(g_gl.window, size[0], size[1]);
        int fbW = 0, fbH = 0;
        for (int i = 0; i < 50 && (fbW != size[0] || fbH != size[1]); ++i)
        {
            glfwPollEvents();
            glfwGetFramebufferSize(g_gl.window, &fbW, &fbH);
        }
        bool delivered = pumpUntilFramebuffer(fbW, fbH);
        if (!delivered)
            FrameBufferSizeCallBack(g_gl.window, fbW, fbH);
        const int errors = frames(5);
        char label[64];
        std::snprintf(label, sizeof(label), "resize %dx%d", size[0], size[1]);
        record(CheckRenderState(label, fbW, fbH, fbW, fbH, errors, delivered),
            "resize_" + std::to_string(size[0]) + "x" + std::to_string(size[1]));
    }

    int winW = 0, winH = 0, fbW = 0, fbH = 0;
    glfwGetWindowSize(g_gl.window, &winW, &winH);
    glfwGetFramebufferSize(g_gl.window, &fbW, &fbH);

    // 2. Internal resolution at 200%: scene target twice the presented size,
    // the same split a 2x Retina backing store produces between drawable and
    // window. The pause menu's resolution-scale option takes this path.
    g_resolutionScalePercent = 200;
    const bool applied = ApplyInternalResolutionSettings();
    {
        const int errors = frames(5);
        nlohmann::json step = CheckRenderState("internal resolution 200%", fbW * 2, fbH * 2, fbW, fbH, errors, true);
        step["applied"] = applied;
        record(step, "scale_200");
    }
    g_resolutionScalePercent = 100;
    ApplyInternalResolutionSettings();
    record(CheckRenderState("internal resolution 100% again", fbW, fbH, fbW, fbH, frames(3), true), "");

    // 3. Retina-like callback: GLFW reports a drawable twice the window
    // size. This display has no 2x backing, so the call is synthetic (what
    // GLFW would send on a Retina screen); it checks the game's render state,
    // not the pixels on screen.
    FrameBufferSizeCallBack(g_gl.window, winW * 2, winH * 2);
    {
        nlohmann::json step = CheckRenderState("retina-like framebuffer 2x window (synthetic callback)",
            winW * 2, winH * 2, winW * 2, winH * 2, frames(5), true);
        step["synthetic"] = true;
        record(step, "retina_2x");
    }
    FrameBufferSizeCallBack(g_gl.window, fbW, fbH);
    record(CheckRenderState("framebuffer back to 1x", fbW, fbH, fbW, fbH, frames(3), true), "");

    // 4. Fullscreen and back, through the video-settings path. On macOS
    // Fullscreen is a borderless window over the whole display at its current
    // mode with the menu bar and Dock hidden while the game is active
    // (gl.cpp, ApplyWindowModeSettings). GLFW reports the window and the
    // video mode in points and the framebuffer in pixels (sweep_display.h).
    if (fullscreen)
    {
        // The game's monitor 0 is GLFW's primary monitor, CGMainDisplayID.
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* current = monitor != nullptr ? glfwGetVideoMode(monitor) : nullptr;
        const GLFWvidmode mode = current != nullptr ? *current : GLFWvidmode{};
        const sweep_display::DisplayMode modeBefore = MainDisplayMode();
        const bool toFull = ApplyWindowModeSettings(WindowMode_Fullscreen);
        int w = 0, h = 0, ww = 0, wh = 0;
        for (int i = 0; i < 100; ++i)
        {
            glfwPollEvents();
            glfwGetWindowSize(g_gl.window, &ww, &wh);
            glfwGetFramebufferSize(g_gl.window, &w, &h);
            if (ww == mode.width && wh == mode.height && g_gl.outputWidth == w && g_gl.outputHeight == h)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        const bool delivered = g_gl.outputWidth == w && g_gl.outputHeight == h;
        if (!delivered)
            FrameBufferSizeCallBack(g_gl.window, w, h);
        const sweep_display::DisplayMode modeDuring = MainDisplayMode();
        sweep_display::FullscreenGeometry geometry;
        if (monitor != nullptr)
            glfwGetMonitorPos(monitor, &geometry.monitorX, &geometry.monitorY);
        geometry.modeWidth = mode.width;
        geometry.modeHeight = mode.height;
        glfwGetWindowPos(g_gl.window, &geometry.windowX, &geometry.windowY);
        glfwGetWindowSize(g_gl.window, &geometry.windowWidth, &geometry.windowHeight);
        glfwGetFramebufferSize(g_gl.window, &geometry.framebufferWidth, &geometry.framebufferHeight);
        glfwGetWindowContentScale(g_gl.window, &geometry.scaleX, &geometry.scaleY);
        // Exclusive fullscreen (a window on a monitor) is what lets GLFW
        // switch the display mode.
        const bool exclusive = glfwGetWindowMonitor(g_gl.window) != nullptr;
        const bool decorated = glfwGetWindowAttrib(g_gl.window, GLFW_DECORATED) == GLFW_TRUE;
        const unsigned long presentation = PresentationOptions();
        const bool covers = sweep_display::WindowCoversMonitor(geometry);
        const bool scaled = sweep_display::FramebufferMatchesScale(geometry);
        const bool modeKept = sweep_display::SameMode(modeBefore, modeDuring);
        // The two bits fullscreen sets; other options are not the game's.
        const bool barsHidden = (presentation & kHideDockAndMenuBar) == kHideDockAndMenuBar;

        nlohmann::json step = CheckRenderState("fullscreen", w, h, w, h, frames(10), delivered);
        step["applied"] = toFull;
        step["monitor_mode"] = { mode.width, mode.height, mode.refreshRate };
        step["monitor_pos"] = { geometry.monitorX, geometry.monitorY };
        step["window_pos"] = { geometry.windowX, geometry.windowY };
        step["content_scale"] = { geometry.scaleX, geometry.scaleY };
        step["display_mode_before"] = Describe(modeBefore);
        step["display_mode_during"] = Describe(modeDuring);
        step["window_covers_monitor"] = covers;
        step["framebuffer_matches_scale"] = scaled;
        step["display_mode_unchanged"] = modeKept;
        step["exclusive_monitor"] = exclusive;
        step["decorated"] = decorated;
        step["presentation_options"] = presentation;
        step["menu_bar_and_dock_hidden"] = barsHidden;
        step["passed"] = step["passed"].get<bool>() && toFull && current != nullptr && covers && scaled &&
            modeKept && !exclusive && !decorated && barsHidden;
        record(step, "fullscreen");

        const bool toWindow = ApplyWindowModeSettings(WindowMode_Windowed);
        for (int i = 0; i < 100; ++i)
        {
            glfwPollEvents();
            glfwGetFramebufferSize(g_gl.window, &w, &h);
            if (w == fbW && h == fbH && g_gl.outputWidth == w)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        const bool deliveredBack = g_gl.outputWidth == w && g_gl.outputHeight == h;
        if (!deliveredBack)
            FrameBufferSizeCallBack(g_gl.window, w, h);
        const sweep_display::DisplayMode modeAfter = MainDisplayMode();
        const bool modeKeptAfter = sweep_display::SameMode(modeBefore, modeAfter);
        const unsigned long presentationAfter = PresentationOptions();
        const bool decoratedAfter = glfwGetWindowAttrib(g_gl.window, GLFW_DECORATED) == GLFW_TRUE;
        nlohmann::json back = CheckRenderState("windowed again", w, h, w, h, frames(10), deliveredBack);
        back["applied"] = toWindow;
        back["restored_size"] = w == fbW && h == fbH;
        back["display_mode_after"] = Describe(modeAfter);
        back["display_mode_unchanged"] = modeKeptAfter;
        back["decorated"] = decoratedAfter;
        back["presentation_options"] = presentationAfter;
        back["passed"] = back["passed"].get<bool>() && toWindow && w == fbW && h == fbH && modeKeptAfter &&
            decoratedAfter && (presentationAfter & kHideDockAndMenuBar) == 0;
        record(back, "windowed_again");
    }

    out["steps"] = check.steps;
    out["passed"] = check.allPassed && out["level_loaded"].get<bool>();
    out["content_scale"] = [] { float x = 0, y = 0; glfwGetWindowContentScale(g_gl.window, &x, &y); return std::vector<float>{ x, y }; }();
    WriteJson(options.Get("--result"), out);
    ShutdownGame();
    return 0;
}

}

namespace sweep_game
{
std::vector<std::string> LevelNames()
{
    std::vector<std::string> names;
    for (const LEVELINFO& level : g_levelTable)
        names.push_back(level.levelName);
    return names;
}

int RunChild(const std::vector<std::string>& args)
{
    using namespace sweep_child;
    // args: child <mode> [--key value]...
    if (args.size() < 2)
        return 2;
    const std::string& mode = args[1];
    const Options options = ParseOptions(args, 2);
    if (mode == "levels") return ModeLevels(options);
    if (mode == "hash") return ModeHash(options);
    if (mode == "cutscene") return ModeCutscene(options);
    if (mode == "audio") return ModeAudio(options);
    if (mode == "save-write") return ModeSaveWrite(options);
    if (mode == "save-read") return ModeSaveRead(options);
    if (mode == "display") return ModeDisplay(options);
    if (mode == "van-door") return ModeVanDoor(options);
    if (mode == "hub-tour") return ModeHubTour(options);
    if (mode == "charm-exit") return ModeCharmExit(options);
    if (mode == "behaviour") return ModeBehaviour(options);
    if (mode == "unlock") return ModeUnlock(options);
    std::fprintf(stderr, "unknown child mode %s\n", mode.c_str());
    return 2;
}
}
