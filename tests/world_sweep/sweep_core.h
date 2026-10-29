#pragma once
// Game-independent helpers for the world sweep (tests/world_sweep): image
// compare and PNG I/O, transform checks, frame hashes, log attribution and
// the per-level report rows. Nothing here touches GL or the game objects, so
// the selftest exercises it without data or a display.

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace sweep
{
struct Image
{
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba;
};

struct ImageDiff
{
    bool sizeMismatch = false;
    uint64_t changedPixels = 0;
    uint64_t totalPixels = 0;
    int maxChannelDelta = 0;
    double ChangedFraction() const;
};

// A pixel changed when any of R, G, B differs by more than `tolerance`.
// Alpha is ignored.
ImageDiff CompareImages(const Image& a, const Image& b, int tolerance);
// Size mismatch, or more than `maxFraction` of the pixels changed.
bool ImageChanged(const ImageDiff& diff, double maxFraction);
double MeanLuma(const Image& image);
void FlipRows(Image& image);
bool WritePng(const std::filesystem::path& path, const Image& image);
bool ReadPng(const std::filesystem::path& path, Image& image);

struct FolderCompare
{
    int compared = 0;
    std::vector<std::string> changed; // "relative/path.png: N px (F%), max delta D"
    std::vector<std::string> missing; // in the reference, not in the output
    std::vector<std::string> added;   // in the output, not in the reference
    bool Passed() const { return changed.empty() && missing.empty(); }
};
// Compares every .png below `reference` with the same relative path below
// `output`.
FolderCompare CompareFolders(const std::filesystem::path& output,
    const std::filesystem::path& reference, int tolerance, double maxFraction);

struct TransformScan
{
    int nonFinite = 0;
    int absurd = 0;
    int parked = 0;
    float maxAbs = 0.0f;
};
void ScanValues(TransformScan& scan, const float* values, size_t count, float absurdLimit);

// Checks objects' world transforms frame by frame. NaN and Inf fail, and so
// does any value above `absurdLimit`, with one exception: a component whose
// bits are exactly +FLT_MAX or -FLT_MAX (0x7f7fffff / 0xff7fffff) since the
// object's first check, at the checker's first frame, with every other
// component of the transform finite, is "parked" (level data can place an
// object there). The same value appearing later, or on an object first seen
// later, is absurd.
class TransformChecker
{
public:
    explicit TransformChecker(float absurdLimit) : m_absurdLimit(absurdLimit) {}
    TransformScan Check(uint64_t object, int frame, const float* values, size_t count);

private:
    float m_absurdLimit;
    bool m_started = false;
    int m_firstFrame = 0;
    std::map<uint64_t, uint64_t> m_parkedMask; // components parked since the first frame
};

constexpr uint64_t kFnvOffset = 14695981039346656037ull;
uint64_t Fnv1a(const void* data, size_t size, uint64_t hash = kFnvOffset);

// [frame][object]
using FrameHashes = std::vector<std::vector<uint64_t>>;
struct Divergence
{
    bool diverged = false;
    size_t frame = 0;
    size_t object = 0;
    bool objectCountMismatch = false;
    size_t framesCompared = 0;
};
Divergence FirstDivergence(const FrameHashes& a, const FrameHashes& b);
bool WriteFrameHashes(const std::filesystem::path& path, const FrameHashes& hashes);
bool ReadFrameHashes(const std::filesystem::path& path, FrameHashes& hashes);

// A child writes "=== LEVEL <name> ===" before each level and
// "=== SIGNAL <n> ===" plus a backtrace when it crashes. Lines before the
// first marker go to the "" entry.
struct LevelLog
{
    int errorLines = 0;
    std::vector<std::string> errors;
    int signal = 0;
    std::string backtrace;
    // Apple GL driver "UNSUPPORTED (log once)" lines.
    int driverWarnings = 0;
    // "[sampler-audit]" lines (the audit is on unless CANE_SWEEP_SAMPLER_AUDIT=0).
    int samplerAudit = 0;
    // AddressSanitizer / UndefinedBehaviorSanitizer reports (macos-asan
    // build): one per headline ("ERROR: AddressSanitizer: ...",
    // "file:line:col: runtime error: ..."), with each report's text, stack
    // and SUMMARY line included, kept in sanitizerText.
    int sanitizerReports = 0;
    std::string sanitizerText;
};
std::map<std::string, LevelLog> SplitLevelLog(const std::string& text);
int CountErrorLines(const std::string& text);
int CountSanitizerReports(const std::string& text);
std::string ReadTextFile(const std::filesystem::path& path);

struct FrameStats
{
    int frames = 0;
    double totalMs = 0.0;
    double maxMs = 0.0;
    int maxFrame = -1;
    void Add(double ms, int frame);
    double AvgMs() const;
};

// Child time limits run on awake time. CLOCK_UPTIME_RAW stops while the Mac
// sleeps (a maintenance sleep can last many minutes), so sleep never times a
// child out; CLOCK_MONOTONIC keeps counting through it, and the difference
// is the time slept.
double AwakeSeconds();
double TotalSeconds();
struct SleepAwareTimer
{
    double awakeStart = AwakeSeconds();
    double totalStart = TotalSeconds();
    double Awake() const { return AwakeSeconds() - awakeStart; }
    double Slept() const;
};
// "" for under 5 s of sleep, else a note for the report.
std::string SleepNote(double sleptSeconds);
// The cutscene child's own watchdog: the cap when one is given, else the
// movie's length x 1.5 + 60 s; 0 (no watchdog) when neither is known.
double CutsceneWatchdogSeconds(double mediaSeconds, double maxSeconds);
// Runs `sample <pid> <seconds>` and appends its report to `log` between
// "=== SAMPLE pid <pid> ===" and "=== END SAMPLE ===", giving up on the
// sampler after `limitSeconds`. Used before a stuck child is killed.
bool SampleProcessToLog(int pid, int seconds, const std::filesystem::path& log, double limitSeconds = 60.0);

constexpr int kRequiredFrames = 300;
constexpr int kMinimumViews = 3;

struct LevelResult
{
    std::string level;
    bool loaded = false;
    double loadMs = 0.0;
    int frames = 0;
    FrameStats frameStats;
    int nonFixedDtFrames = 0;
    int glErrors = 0;
    unsigned firstGlError = 0;
    int firstGlErrorFrame = -1;
    int loggedErrors = 0;
    int nonFinite = 0;
    int absurd = 0;
    float maxAbsCoordinate = 0.0f;
    // Non-finite or absurd values on objects that are not in the world
    // (FIsLoInWorld false): reported, not failed.
    int outOfWorldBad = 0;
    // World-transform components parked at exactly +/-FLT_MAX since frame 0
    // (TransformChecker): reported, not failed.
    int parked = 0;
    // Apple driver warnings and sampler-audit hits: failures.
    int driverWarnings = 0;
    int samplerAudit = 0;
    // Sanitizer reports in the level's log: failures.
    int sanitizerReports = 0;
    // Sidebags the game allocated that EnsureBasicSidebag then replaced with
    // an empty one, losing the object's custom options (sidebag_probe.cpp):
    // failures.
    int sidebagRebuilds = 0;
    int objects = 0;
    int warps = 0;
    int views = 0;
    int cutscenesCut = 0;
    std::string transitionRequested;
    // How a child that stopped inside this level ended: a crash signal, the
    // runner's timeout, or a non-zero exit code (never stored as a signal).
    int signal = 0;
    bool timedOut = false;
    int exitCode = 0;
    std::string note;
};
bool Passed(const LevelResult& result);

// How a child process the runner started ended (sweep_main.cpp).
struct ChildOutcome
{
    bool exited = false;   // returned from main or called exit
    int exitCode = -1;
    int signal = 0;        // the signal that ended it (the crash handler exits with 128 + signal)
    bool timedOut = false; // the runner stopped it for taking too long
    // Awake time: limits never count system sleep (SleepAwareTimer).
    double seconds = 0.0;
    double slept = 0.0;
};
// From a waitpid status.
ChildOutcome OutcomeFromWaitStatus(int status);
// A child the runner stopped (SIGKILL) for running past its limit.
ChildOutcome TimedOutOutcome();
// "exit 0", "signal 11", "timed out", plus a note when the system slept.
std::string Describe(const ChildOutcome& outcome);
// The row for a level a child stopped in before it wrote one.
LevelResult StoppedLevelRow(const std::string& level, const ChildOutcome& outcome);
std::vector<std::string> FailureReasons(const LevelResult& result);
std::string TsvHeader();
std::string ToTsv(const LevelResult& result);
nlohmann::json ToJson(const LevelResult& result);
LevelResult FromJson(const nlohmann::json& json);
}
