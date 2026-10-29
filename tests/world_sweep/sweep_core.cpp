#include "sweep_core.h"

#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <csignal>
#include <thread>

extern char** environ;
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>

// Static copies: the game objects already export the stb_image_write
// functions (debug.cpp), and nothing in the build provides stb_image.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

namespace sweep
{
double ImageDiff::ChangedFraction() const
{
    return totalPixels == 0 ? 0.0 : static_cast<double>(changedPixels) / static_cast<double>(totalPixels);
}

ImageDiff CompareImages(const Image& a, const Image& b, int tolerance)
{
    ImageDiff diff;
    if (a.width != b.width || a.height != b.height ||
        a.rgba.size() != b.rgba.size())
    {
        diff.sizeMismatch = true;
        return diff;
    }
    diff.totalPixels = static_cast<uint64_t>(a.width) * static_cast<uint64_t>(a.height);
    for (size_t pixel = 0; pixel < diff.totalPixels; ++pixel)
    {
        int worst = 0;
        for (size_t channel = 0; channel < 3; ++channel)
        {
            const size_t index = pixel * 4 + channel;
            worst = (std::max)(worst, std::abs(static_cast<int>(a.rgba[index]) - static_cast<int>(b.rgba[index])));
        }
        diff.maxChannelDelta = (std::max)(diff.maxChannelDelta, worst);
        if (worst > tolerance)
            ++diff.changedPixels;
    }
    return diff;
}

bool ImageChanged(const ImageDiff& diff, double maxFraction)
{
    return diff.sizeMismatch || diff.ChangedFraction() > maxFraction;
}

double MeanLuma(const Image& image)
{
    const size_t pixels = static_cast<size_t>(image.width) * static_cast<size_t>(image.height);
    if (pixels == 0 || image.rgba.size() < pixels * 4)
        return 0.0;
    double sum = 0.0;
    for (size_t pixel = 0; pixel < pixels; ++pixel)
    {
        const uint8_t* p = &image.rgba[pixel * 4];
        sum += 0.2126 * p[0] + 0.7152 * p[1] + 0.0722 * p[2];
    }
    return sum / static_cast<double>(pixels);
}

void FlipRows(Image& image)
{
    const size_t stride = static_cast<size_t>(image.width) * 4;
    for (int top = 0, bottom = image.height - 1; top < bottom; ++top, --bottom)
        std::swap_ranges(image.rgba.begin() + top * stride, image.rgba.begin() + (top + 1) * stride,
            image.rgba.begin() + bottom * stride);
}

bool WritePng(const std::filesystem::path& path, const Image& image)
{
    if (image.width <= 0 || image.height <= 0 ||
        image.rgba.size() != static_cast<size_t>(image.width) * image.height * 4)
        return false;
    std::error_code error;
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path(), error);
    return stbi_write_png(path.string().c_str(), image.width, image.height, 4,
        image.rgba.data(), image.width * 4) != 0;
}

bool ReadPng(const std::filesystem::path& path, Image& image)
{
    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
    if (data == nullptr)
        return false;
    image.width = width;
    image.height = height;
    image.rgba.assign(data, data + static_cast<size_t>(width) * height * 4);
    stbi_image_free(data);
    return true;
}

namespace
{
std::set<std::string> PngsBelow(const std::filesystem::path& root)
{
    std::set<std::string> result;
    std::error_code error;
    if (!std::filesystem::is_directory(root, error))
        return result;
    for (auto it = std::filesystem::recursive_directory_iterator(root, error);
         !error && it != std::filesystem::recursive_directory_iterator(); it.increment(error))
    {
        if (it->is_regular_file() && it->path().extension() == ".png")
            result.insert(std::filesystem::relative(it->path(), root).generic_string());
    }
    return result;
}
}

FolderCompare CompareFolders(const std::filesystem::path& output,
    const std::filesystem::path& reference, int tolerance, double maxFraction)
{
    FolderCompare result;
    const std::set<std::string> outputs = PngsBelow(output);
    const std::set<std::string> references = PngsBelow(reference);
    for (const std::string& name : references)
    {
        if (outputs.count(name) == 0)
        {
            result.missing.push_back(name);
            continue;
        }
        Image a, b;
        if (!ReadPng(output / name, a) || !ReadPng(reference / name, b))
        {
            result.changed.push_back(name + ": unreadable");
            continue;
        }
        ++result.compared;
        const ImageDiff diff = CompareImages(a, b, tolerance);
        if (!ImageChanged(diff, maxFraction))
            continue;
        char line[256];
        if (diff.sizeMismatch)
            std::snprintf(line, sizeof(line), ": size %dx%d vs reference %dx%d", a.width, a.height, b.width, b.height);
        else
            std::snprintf(line, sizeof(line), ": %llu px (%.4f%%), max delta %d",
                static_cast<unsigned long long>(diff.changedPixels), diff.ChangedFraction() * 100.0,
                diff.maxChannelDelta);
        result.changed.push_back(name + line);
    }
    for (const std::string& name : outputs)
        if (references.count(name) == 0)
            result.added.push_back(name);
    return result;
}

void ScanValues(TransformScan& scan, const float* values, size_t count, float absurdLimit)
{
    for (size_t i = 0; i < count; ++i)
    {
        const float value = values[i];
        if (!std::isfinite(value))
        {
            ++scan.nonFinite;
            continue;
        }
        const float magnitude = std::fabs(value);
        scan.maxAbs = (std::max)(scan.maxAbs, magnitude);
        if (magnitude > absurdLimit)
            ++scan.absurd;
    }
}

TransformScan TransformChecker::Check(uint64_t object, int frame, const float* values, size_t count)
{
    if (!m_started)
    {
        m_started = true;
        m_firstFrame = frame;
    }
    uint64_t maxMask = 0;
    bool othersFinite = true;
    for (size_t i = 0; i < count; ++i)
    {
        uint32_t bits = 0;
        std::memcpy(&bits, &values[i], sizeof(bits));
        if (bits == 0x7f7fffffu || bits == 0xff7fffffu)
            maxMask |= uint64_t{ 1 } << (i & 63);
        else if (!std::isfinite(values[i]))
            othersFinite = false;
    }
    const auto known = m_parkedMask.find(object);
    if (known == m_parkedMask.end())
        m_parkedMask[object] = frame == m_firstFrame && othersFinite ? maxMask : 0;
    const uint64_t parkedMask = m_parkedMask[object];

    TransformScan scan;
    for (size_t i = 0; i < count; ++i)
    {
        const uint64_t bit = uint64_t{ 1 } << (i & 63);
        if ((maxMask & bit) != 0 && (parkedMask & bit) != 0 && othersFinite)
        {
            ++scan.parked;
            continue;
        }
        ScanValues(scan, &values[i], 1, m_absurdLimit);
    }
    return scan;
}

uint64_t Fnv1a(const void* data, size_t size, uint64_t hash)
{
    const auto* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i)
    {
        hash ^= bytes[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

Divergence FirstDivergence(const FrameHashes& a, const FrameHashes& b)
{
    Divergence result;
    const size_t frames = (std::min)(a.size(), b.size());
    for (size_t frame = 0; frame < frames; ++frame)
    {
        result.framesCompared = frame + 1;
        const std::vector<uint64_t>& x = a[frame];
        const std::vector<uint64_t>& y = b[frame];
        const size_t objects = (std::min)(x.size(), y.size());
        for (size_t object = 0; object < objects; ++object)
        {
            if (x[object] != y[object])
            {
                result.diverged = true;
                result.frame = frame;
                result.object = object;
                return result;
            }
        }
        if (x.size() != y.size())
        {
            result.diverged = true;
            result.frame = frame;
            result.object = objects;
            result.objectCountMismatch = true;
            return result;
        }
    }
    if (a.size() != b.size())
    {
        result.diverged = true;
        result.frame = frames;
    }
    return result;
}

bool WriteFrameHashes(const std::filesystem::path& path, const FrameHashes& hashes)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    const uint64_t frames = hashes.size();
    out.write(reinterpret_cast<const char*>(&frames), sizeof(frames));
    for (const std::vector<uint64_t>& frame : hashes)
    {
        const uint64_t count = frame.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        out.write(reinterpret_cast<const char*>(frame.data()), static_cast<std::streamsize>(count * sizeof(uint64_t)));
    }
    return static_cast<bool>(out);
}

bool ReadFrameHashes(const std::filesystem::path& path, FrameHashes& hashes)
{
    std::ifstream in(path, std::ios::binary);
    uint64_t frames = 0;
    if (!in.read(reinterpret_cast<char*>(&frames), sizeof(frames)) || frames > (1u << 24))
        return false;
    hashes.assign(static_cast<size_t>(frames), {});
    for (std::vector<uint64_t>& frame : hashes)
    {
        uint64_t count = 0;
        if (!in.read(reinterpret_cast<char*>(&count), sizeof(count)) || count > (1u << 24))
            return false;
        frame.resize(static_cast<size_t>(count));
        if (!in.read(reinterpret_cast<char*>(frame.data()), static_cast<std::streamsize>(count * sizeof(uint64_t))))
            return false;
    }
    return true;
}

namespace
{
bool IsErrorLine(const std::string& line)
{
    return line.find("[ERROR]") != std::string::npos;
}

// The first line of an ASan or UBSan report. UBSan's own lines are
// "<file>:<line>:<col>: runtime error: <what>" (or "<unknown>: runtime
// error: ..."); fatal ones and ASan's start "==<pid>==ERROR: <Sanitizer>: ".
bool IsSanitizerHeadline(const std::string& line)
{
    return line.find(": runtime error: ") != std::string::npos ||
        line.find("==ERROR: AddressSanitizer: ") != std::string::npos ||
        line.find("==ERROR: UndefinedBehaviorSanitizer: ") != std::string::npos;
}

bool Marker(const std::string& line, const char* prefix, std::string& value)
{
    const std::string head = prefix;
    const std::string tail = " ===";
    if (line.size() < head.size() + tail.size() || line.compare(0, head.size(), head) != 0 ||
        line.compare(line.size() - tail.size(), tail.size(), tail) != 0)
        return false;
    value = line.substr(head.size(), line.size() - head.size() - tail.size());
    return true;
}
}

std::map<std::string, LevelLog> SplitLevelLog(const std::string& text)
{
    std::map<std::string, LevelLog> result;
    std::string current;
    result[current];
    bool inBacktrace = false;
    bool inSanitizer = false;
    // Caps one level's kept sanitizer text; the count is never capped.
    constexpr size_t kMaxSanitizerText = 256 * 1024;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        std::string value;
        if (Marker(line, "=== LEVEL ", value))
        {
            current = value;
            result[current];
            inBacktrace = false;
            inSanitizer = false;
            continue;
        }
        if (Marker(line, "=== SIGNAL ", value))
        {
            result[current].signal = std::atoi(value.c_str());
            inBacktrace = true;
            continue;
        }
        LevelLog& log = result[current];
        if (inBacktrace)
            log.backtrace += line + "\n";
        if (line.rfind("UNSUPPORTED", 0) == 0)
            ++log.driverWarnings;
        if (line.rfind("[sampler-audit]", 0) == 0 && line.find(" unit ") != std::string::npos)
            ++log.samplerAudit;
        if (IsErrorLine(line))
        {
            ++log.errorLines;
            if (log.errors.size() < 20)
                log.errors.push_back(line);
        }
        if (IsSanitizerHeadline(line))
        {
            ++log.sanitizerReports;
            inSanitizer = true;
        }
        if (inSanitizer)
        {
            if (log.sanitizerText.size() < kMaxSanitizerText)
                log.sanitizerText += line + "\n";
            // The SUMMARY line ends a report.
            if (line.rfind("SUMMARY: ", 0) == 0)
                inSanitizer = false;
        }
    }
    return result;
}

int CountErrorLines(const std::string& text)
{
    int count = 0;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
        count += IsErrorLine(line) ? 1 : 0;
    return count;
}

int CountSanitizerReports(const std::string& text)
{
    int count = 0;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
        count += IsSanitizerHeadline(line) ? 1 : 0;
    return count;
}

std::string ReadTextFile(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
}

double AwakeSeconds()
{
    return static_cast<double>(clock_gettime_nsec_np(CLOCK_UPTIME_RAW)) / 1e9;
}

double TotalSeconds()
{
    return static_cast<double>(clock_gettime_nsec_np(CLOCK_MONOTONIC)) / 1e9;
}

double SleepAwareTimer::Slept() const
{
    return (std::max)(0.0, (TotalSeconds() - totalStart) - Awake());
}

std::string SleepNote(double sleptSeconds)
{
    if (sleptSeconds < 5.0)
        return {};
    char text[96];
    std::snprintf(text, sizeof(text), "interrupted by system sleep (%.0f s asleep)", sleptSeconds);
    return text;
}

double CutsceneWatchdogSeconds(double mediaSeconds, double maxSeconds)
{
    if (maxSeconds > 0.0)
        return maxSeconds;
    return mediaSeconds > 0.0 ? mediaSeconds * 1.5 + 60.0 : 0.0;
}

bool SampleProcessToLog(int pid, int seconds, const std::filesystem::path& log, double limitSeconds)
{
    if (pid <= 0)
        return false;
    std::error_code error;
    const std::filesystem::path report = std::filesystem::temp_directory_path() /
        ("cane_sweep_sample_" + std::to_string(getpid()) + "_" + std::to_string(pid) + ".txt");
    std::filesystem::remove(report, error);
    std::string tool = "/usr/bin/sample";
    std::string pidText = std::to_string(pid);
    std::string secondsText = std::to_string((std::max)(1, seconds));
    std::string fileFlag = "-file";
    std::string reportText = report.string();
    char* argv[] = { tool.data(), pidText.data(), secondsText.data(), fileFlag.data(), reportText.data(), nullptr };
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
    pid_t sampler = -1;
    const bool spawned = posix_spawn(&sampler, argv[0], &actions, nullptr, argv, environ) == 0;
    posix_spawn_file_actions_destroy(&actions);
    if (!spawned)
        return false;
    const double deadline = AwakeSeconds() + limitSeconds;
    int status = 0;
    while (waitpid(sampler, &status, WNOHANG) != sampler)
    {
        if (AwakeSeconds() > deadline)
        {
            // Only ever the sampler this function started.
            kill(sampler, SIGKILL);
            waitpid(sampler, &status, 0);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    const std::string text = ReadTextFile(report);
    std::filesystem::remove(report, error);
    if (text.empty())
        return false;
    std::ofstream out(log, std::ios::app);
    out << "\n=== SAMPLE pid " << pid << " ===\n" << text << "\n=== END SAMPLE ===\n";
    return static_cast<bool>(out);
}

void FrameStats::Add(double ms, int frame)
{
    ++frames;
    totalMs += ms;
    if (ms > maxMs || maxFrame < 0)
    {
        maxMs = ms;
        maxFrame = frame;
    }
}

double FrameStats::AvgMs() const
{
    return frames == 0 ? 0.0 : totalMs / frames;
}

std::vector<std::string> FailureReasons(const LevelResult& r)
{
    std::vector<std::string> reasons;
    // A child that stopped inside the level says how; whether the level
    // loaded first is then unknown, so "did not load" is not claimed.
    const bool stopped = r.timedOut || r.signal != 0 || r.exitCode != 0;
    if (r.timedOut)
        reasons.push_back("timed out");
    if (r.signal != 0)
        reasons.push_back("crashed with signal " + std::to_string(r.signal));
    if (r.exitCode != 0)
        reasons.push_back("child exited with code " + std::to_string(r.exitCode));
    if (!r.loaded)
    {
        if (!stopped)
            reasons.push_back("did not load");
    }
    else if (r.frames < kRequiredFrames)
        reasons.push_back("ran " + std::to_string(r.frames) + " frames");
    if (r.glErrors != 0)
        reasons.push_back(std::to_string(r.glErrors) + " GL errors");
    if (r.loggedErrors != 0)
        reasons.push_back(std::to_string(r.loggedErrors) + " [ERROR] lines");
    if (r.nonFinite != 0)
        reasons.push_back(std::to_string(r.nonFinite) + " non-finite transform values");
    if (r.absurd != 0)
        reasons.push_back(std::to_string(r.absurd) + " absurd transform values");
    if (r.driverWarnings != 0)
        reasons.push_back(std::to_string(r.driverWarnings) + " GL driver warnings");
    if (r.samplerAudit != 0)
        reasons.push_back(std::to_string(r.samplerAudit) + " buffer samplers on empty units");
    if (r.sanitizerReports != 0)
        reasons.push_back(std::to_string(r.sanitizerReports) + " sanitizer reports");
    if (r.sidebagRebuilds != 0)
        reasons.push_back(std::to_string(r.sidebagRebuilds) + " valid sidebags replaced (custom options lost)");
    return reasons;
}

bool Passed(const LevelResult& r)
{
    return FailureReasons(r).empty();
}

ChildOutcome OutcomeFromWaitStatus(int status)
{
    ChildOutcome outcome;
    if (WIFEXITED(status))
    {
        outcome.exited = true;
        outcome.exitCode = WEXITSTATUS(status);
        // The crash handler exits with 128 + signal.
        if (outcome.exitCode > 128 && outcome.exitCode < 160)
            outcome.signal = outcome.exitCode - 128;
    }
    else if (WIFSIGNALED(status))
        outcome.signal = WTERMSIG(status);
    return outcome;
}

ChildOutcome TimedOutOutcome()
{
    // The SIGKILL that stops it is the runner's, not a crash: no signal.
    ChildOutcome outcome;
    outcome.timedOut = true;
    return outcome;
}

std::string Describe(const ChildOutcome& r)
{
    std::string text = r.timedOut ? "timed out" : r.signal != 0 ? "signal " + std::to_string(r.signal)
                                                                : "exit " + std::to_string(r.exitCode);
    // Informational: sleep never counts toward a limit.
    const std::string sleep = SleepNote(r.slept);
    return sleep.empty() ? text : text + ", " + sleep;
}

LevelResult StoppedLevelRow(const std::string& level, const ChildOutcome& outcome)
{
    LevelResult failed;
    failed.level = level;
    failed.timedOut = outcome.timedOut;
    failed.signal = outcome.timedOut ? 0 : outcome.signal;
    failed.exitCode = outcome.timedOut || outcome.signal != 0 || !outcome.exited ? 0 : outcome.exitCode;
    failed.note = "child " + Describe(outcome) + " during this level";
    return failed;
}

std::string TsvHeader()
{
    return "level\tloaded\tload_ms\tframes\tavg_frame_ms\tmax_frame_ms\tmax_frame\tnonfixed_dt\tgl_errors\t"
           "first_gl_error\tfirst_gl_error_frame\tlogged_errors\tnan_inf\tabsurd\tmax_abs\tout_of_world_bad\tparked\t"
           "driver_warnings\tsampler_audit\tsidebag_rebuilds\tobjects\twarps\tviews\t"
           "cutscenes_cut\ttransition_requested\tsignal\ttimed_out\texit_code\tresult\tnote";
}

std::string ToTsv(const LevelResult& r)
{
    char numbers[512];
    std::snprintf(numbers, sizeof(numbers),
        "%d\t%.1f\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t0x%X\t%d\t%d\t%d\t%d\t%g\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
        r.loaded ? 1 : 0, r.loadMs, r.frames, r.frameStats.AvgMs(), r.frameStats.maxMs, r.frameStats.maxFrame,
        r.nonFixedDtFrames, r.glErrors, r.firstGlError, r.firstGlErrorFrame, r.loggedErrors,
        r.nonFinite, r.absurd, static_cast<double>(r.maxAbsCoordinate), r.outOfWorldBad, r.parked, r.driverWarnings, r.samplerAudit, r.sidebagRebuilds, r.objects, r.warps, r.views,
        r.cutscenesCut);
    std::string note = r.note;
    for (const std::string& reason : FailureReasons(r))
        note += (note.empty() ? "" : "; ") + reason;
    std::replace(note.begin(), note.end(), '\t', ' ');
    std::replace(note.begin(), note.end(), '\n', ' ');
    return r.level + "\t" + numbers + "\t" + (r.transitionRequested.empty() ? "-" : r.transitionRequested) +
        "\t" + std::to_string(r.signal) + "\t" + (r.timedOut ? "1" : "0") + "\t" + std::to_string(r.exitCode) + "\t" +
        (Passed(r) ? "PASS" : "FAIL") + "\t" + note;
}

nlohmann::json ToJson(const LevelResult& r)
{
    return {
        { "level", r.level }, { "loaded", r.loaded }, { "load_ms", r.loadMs }, { "frames", r.frames },
        { "frame_count", r.frameStats.frames }, { "frame_total_ms", r.frameStats.totalMs },
        { "avg_frame_ms", r.frameStats.AvgMs() }, { "max_frame_ms", r.frameStats.maxMs },
        { "max_frame_index", r.frameStats.maxFrame }, { "out_of_world_bad", r.outOfWorldBad }, { "parked", r.parked },
        { "driver_warnings", r.driverWarnings }, { "sampler_audit", r.samplerAudit },
        { "sanitizer_reports", r.sanitizerReports }, { "sidebag_rebuilds", r.sidebagRebuilds },
        { "nonfixed_dt_frames", r.nonFixedDtFrames }, { "gl_errors", r.glErrors },
        { "first_gl_error", r.firstGlError }, { "first_gl_error_frame", r.firstGlErrorFrame },
        { "logged_errors", r.loggedErrors }, { "nan_inf", r.nonFinite }, { "absurd", r.absurd },
        { "max_abs_coordinate", r.maxAbsCoordinate }, { "objects", r.objects }, { "warps", r.warps },
        { "views", r.views }, { "cutscenes_cut", r.cutscenesCut },
        { "transition_requested", r.transitionRequested }, { "signal", r.signal }, { "timed_out", r.timedOut },
        { "exit_code", r.exitCode }, { "note", r.note },
        { "passed", Passed(r) }, { "failure_reasons", FailureReasons(r) }
    };
}

LevelResult FromJson(const nlohmann::json& j)
{
    LevelResult r;
    r.level = j.value("level", "");
    r.loaded = j.value("loaded", false);
    r.loadMs = j.value("load_ms", 0.0);
    r.frames = j.value("frames", 0);
    r.frameStats.frames = j.value("frame_count", 0);
    r.frameStats.totalMs = j.value("frame_total_ms", 0.0);
    r.frameStats.maxMs = j.value("max_frame_ms", 0.0);
    r.frameStats.maxFrame = j.value("max_frame_index", -1);
    r.outOfWorldBad = j.value("out_of_world_bad", 0);
    r.parked = j.value("parked", 0);
    r.driverWarnings = j.value("driver_warnings", 0);
    r.samplerAudit = j.value("sampler_audit", 0);
    r.sanitizerReports = j.value("sanitizer_reports", 0);
    r.sidebagRebuilds = j.value("sidebag_rebuilds", 0);
    r.nonFixedDtFrames = j.value("nonfixed_dt_frames", 0);
    r.glErrors = j.value("gl_errors", 0);
    r.firstGlError = j.value("first_gl_error", 0u);
    r.firstGlErrorFrame = j.value("first_gl_error_frame", -1);
    r.loggedErrors = j.value("logged_errors", 0);
    r.nonFinite = j.value("nan_inf", 0);
    r.absurd = j.value("absurd", 0);
    r.maxAbsCoordinate = j.value("max_abs_coordinate", 0.0f);
    r.objects = j.value("objects", 0);
    r.warps = j.value("warps", 0);
    r.views = j.value("views", 0);
    r.cutscenesCut = j.value("cutscenes_cut", 0);
    r.transitionRequested = j.value("transition_requested", "");
    r.signal = j.value("signal", 0);
    r.timedOut = j.value("timed_out", false);
    r.exitCode = j.value("exit_code", 0);
    r.note = j.value("note", "");
    return r;
}
}
