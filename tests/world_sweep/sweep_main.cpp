// cane_world_sweep: data-driven checks of the macOS build against the real
// game data, which is never part of the repository. Each check runs the game
// in child processes of this executable (see sweep_child.cpp) so a crash
// fails one level and the sweep carries on in a fresh process.
//
//   cane_world_sweep <levels|cutscenes|audio|saves|determinism|display|
//                     display-fullscreen|van-door|hub-tour|charm-exit|
//                     behaviour|unlock|fuse|all> [options]
//   cane_world_sweep compare <output-dir> <reference-dir> [--tolerance N] [--max-fraction F]
//   cane_world_sweep units   (game-code checks that need no data, sweep_units.cpp)
//
// Options: --data-dir DIR (default $CANE_TEST_DATA_DIR; the folder holding
// Worlds/ and Cutscenes/), --out DIR (default $CANE_SWEEP_OUT, else
// world-sweep in the build folder, beside bin/: build/<preset>/world-sweep,
// where CTest writes too, whatever the current directory), --levels "A;B",
// --frames N, --fmv-seconds N (0 = whole
// movie), --jobs N, --level NAME (determinism/display/saves),
// --reference DIR (levels: compare the screenshots afterwards; advisory only,
// it lists changed images but never fails the run, because the game is not
// deterministic and two identical runs already differ, see determinism).
// behaviour: --must-happen FILE (events that must happen, failures),
// --baseline DIR (traces to compare with, advisory; default
// <out>/baselines/behaviour when it exists), --write-baseline DIR (keep this
// run's traces as the baseline), --frames N (default 900, 15 s),
// --freeze-root OID (a check of the motion check). unlock and fuse:
// --inject-stuck OID (a check of the checks: the object's response is put
// back after its unlock, as if the live change were missed), --inject-lose-flag
// LOCKG (its world flag is cleared before the save, as if the save lost the
// unlock). behaviour,
// unlock, fuse: --known FILE (failures written up for upstream, reported as
// known instead of failing).
// Every option takes a value; an unknown option, one without its value or a
// stray argument is an error (exit 2).
// CANE_SWEEP_SAMPLER_AUDIT=0 turns off the per-draw buffer-sampler audit;
// CANE_SWEEP_GL_TRACE=1 checks glGetError after every GL call and logs the
// call that raised each error (slow).
//
// In the macos-asan build (tests/README.md) AddressSanitizer and UBSan
// reports go to each child's stderr, i.e. its log; every check counts them
// (per level for "levels") and fails on any.
//
// Each child runs in its own --user-dir scratch folder, so files the game
// writes to its working directory (imgui.ini) stay there.
//
// Exit codes: 0 pass, 1 failure, 2 bad command line, 77 skipped (no data
// folder given). A data folder that is given but holds no game data is a
// failure, not a skip.

#include "sweep_behaviour.h"
#include "sweep_core.h"
#include "sweep_game.h"

#include "../../platform/platform.h"
#include "../../Sly1-Release/coin.h"
#include "../../Sly1-Release/fmv.h"

#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

extern char** environ;

namespace
{
namespace fs = std::filesystem;
constexpr int kSkip = 77;

struct Config
{
    fs::path data;
    fs::path out;
    fs::path self;
    std::vector<std::string> levels;
    int frames = sweep::kRequiredFrames;
    double fmvSeconds = 0.0;
    int jobs = 6;
    std::string level = sweep_game::kDefaultLevel;
    fs::path reference;
    int tolerance = 8;
    double maxFraction = 0.001;
    bool framesGiven = false;
    // behaviour: the baseline traces to compare with (advisory), where to
    // write this run's traces as the new baseline, and the events that must
    // happen (failures).
    fs::path baseline;
    fs::path writeBaseline;
    fs::path mustHappen;
    // Checks of the checks: freeze what segments rooted on this oid drive
    // (behaviour), put this object's response back after its unlock
    // (unlock, fuse).
    int freezeRoot = -1;
    int injectStuck = -1;
    // unlock, fuse: clear this lock group's world flag before the save, as if
    // the save had lost its unlock (the reload brings back the broken fuse).
    int injectLoseFlag = -1;
    // behaviour, unlock, fuse: failures already written up for upstream,
    // reported as known instead of failing (world_sweep/known_findings.json).
    fs::path known;
};

std::vector<std::string> Split(const std::string& text, char separator)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= text.size())
    {
        const size_t end = text.find(separator, start);
        const std::string part = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!part.empty())
            parts.push_back(part);
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return parts;
}

bool ValidData(const fs::path& data)
{
    std::error_code error;
    return !data.empty() && fs::is_regular_file(data / "Worlds" / "Splash" / "Splash.brx", error);
}

// The single-child checks below run under CTest's TIMEOUT 900 (CMakeLists.txt,
// the world sweep's test list); a check's worst case must end before it, or
// CTest kills the runner and orphans the child. A stuck child is sampled
// before it is killed (KillStuck: a 3 s sample, SampleProcessToLog gives up
// after 60 s), which adds to its limit.
constexpr double kCtestTimeoutSeconds = 900.0;
constexpr double kStuckChildExtraSeconds = 60.0;
constexpr double WorstCaseSeconds(int children, double limitSeconds)
{
    return children * (limitSeconds + kStuckChildExtraSeconds);
}

void PrintHowToRun()
{
    std::printf(
        "SKIP: no game data. The world sweep reads the extracted game from a folder given at run time\n"
        "and never copies it. To run it:\n"
        "  CANE_TEST_DATA_DIR=/path/to/folder-with-Worlds-and-Cutscenes \\\n"
        "    ctest --test-dir build/macos-release -R cane_world_sweep --output-on-failure\n"
        "or directly: build/macos-release/bin/cane_world_sweep all --data-dir <folder> --out <folder>\n");
}

// --- child processes ---------------------------------------------------------

using ChildResult = sweep::ChildOutcome;
using sweep::Describe;

struct Child
{
    pid_t pid = -1;
    sweep::SleepAwareTimer timer;
    fs::path log;
};

// Before a stuck child is killed, its stacks go to its log.
void KillStuck(const Child& child)
{
    // A failed spawn leaves pid -1, and kill(-1, ...) signals every process the
    // user owns.
    if (child.pid <= 0)
        return;
    sweep::SampleProcessToLog(child.pid, 3, child.log);
    // Only ever our own child.
    kill(child.pid, SIGKILL);
    int status = 0;
    waitpid(child.pid, &status, 0);
}

Child Spawn(const Config& config, const std::vector<std::string>& args, const fs::path& log)
{
    std::vector<std::string> argv = { config.self.string(), "child" };
    argv.insert(argv.end(), args.begin(), args.end());
    std::vector<char*> raw;
    for (std::string& arg : argv)
        raw.push_back(arg.data());
    raw.push_back(nullptr);

    std::vector<std::string> environment;
    for (char** e = environ; *e != nullptr; ++e)
        if (std::string(*e).rfind("SDL_AUDIO_DRIVER=", 0) != 0 &&
            std::string(*e).rfind("SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT=", 0) != 0)
            environment.emplace_back(*e);
    // Background processes cannot open CoreAudio here.
    environment.emplace_back("SDL_AUDIO_DRIVER=dummy");
    // Children must never read the user's controllers (no device is 0000/0000).
    environment.emplace_back("SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT=0x0000/0x0000");
    std::vector<char*> env;
    for (std::string& value : environment)
        env.push_back(value.data());
    env.push_back(nullptr);

    fs::create_directories(log.parent_path());
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);
    posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    // Run in the child's scratch user folder: the game writes imgui.ini to
    // its working directory.
    for (size_t i = 0; i + 1 < args.size(); ++i)
        if (args[i] == "--user-dir")
            posix_spawn_file_actions_addchdir_np(&actions, args[i + 1].c_str());
    Child child;
    child.log = log;
    child.timer = sweep::SleepAwareTimer();
    if (posix_spawn(&child.pid, raw[0], &actions, nullptr, raw.data(), env.data()) != 0)
        child.pid = -1;
    posix_spawn_file_actions_destroy(&actions);
    return child;
}

// Polls a child. `stalled` decides when to give up on it.
template <typename Stalled>
ChildResult Wait(Child child, Stalled stalled)
{
    ChildResult result;
    if (child.pid <= 0)
        return result;
    for (;;)
    {
        int status = 0;
        const pid_t done = waitpid(child.pid, &status, WNOHANG);
        if (done == child.pid)
        {
            result = sweep::OutcomeFromWaitStatus(status);
            result.seconds = child.timer.Awake();
            result.slept = child.timer.Slept();
            return result;
        }
        if (stalled(child))
        {
            KillStuck(child);
            result = sweep::TimedOutOutcome();
            result.seconds = child.timer.Awake();
            result.slept = child.timer.Slept();
            return result;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

ChildResult RunChild(const Config& config, const std::vector<std::string>& args, const fs::path& log, double timeoutSeconds)
{
    const Child child = Spawn(config, args, log);
    return Wait(child, [&](const Child& c)
    {
        return c.timer.Awake() > timeoutSeconds;
    });
}

std::vector<std::string> Common(const Config& config, const fs::path& userDir)
{
    std::error_code error;
    fs::create_directories(userDir, error);
    return { "--data-dir", config.data.string(), "--user-dir", userDir.string() };
}

nlohmann::json ReadJson(const fs::path& path)
{
    std::ifstream in(path);
    nlohmann::json json;
    try { in >> json; } catch (...) { return nlohmann::json(); }
    return json;
}

// --- levels ------------------------------------------------------------------

int Levels(const Config& config)
{
    std::vector<std::string> names = config.levels.empty() ? sweep_game::LevelNames() : config.levels;
    const fs::path results = config.out / "levels.jsonl";
    const fs::path shots = config.out / "shots";
    const fs::path logs = config.out / "logs";
    fs::create_directories(config.out);
    fs::remove(results);
    std::error_code error;
    fs::remove_all(shots, error);
    fs::remove_all(config.out / "user-levels", error);

    std::vector<sweep::LevelResult> rows;
    std::map<std::string, nlohmann::json> extras;
    std::map<std::string, sweep::LevelLog> levelLogs;
    std::vector<std::string> segmentLogs;
    size_t next = 0;
    int segment = 0;
    while (next < names.size())
    {
        const fs::path list = config.out / "children" / ("levels_" + std::to_string(segment) + ".txt");
        fs::create_directories(list.parent_path());
        {
            std::ofstream out(list, std::ios::trunc);
            for (size_t i = next; i < names.size(); ++i)
                out << names[i] << "\n";
        }
        const fs::path log = logs / ("levels_" + std::to_string(segment) + ".log");
        fs::remove(log, error);
        segmentLogs.push_back(log.string());
        std::vector<std::string> args = { "levels" };
        const std::vector<std::string> common = Common(config, config.out / "user-levels");
        args.insert(args.end(), common.begin(), common.end());
        args.insert(args.end(), { "--list", list.string(), "--results", results.string(), "--shots", shots.string(),
            "--frames", std::to_string(config.frames) });

        std::printf("levels: starting child for %zu level(s) from '%s'\n", names.size() - next, names[next].c_str());
        std::fflush(stdout);
        const Child child = Spawn(config, args, log);
        // A level gets 5 minutes of awake time from the last finished level
        // (or the start).
        double lastProgress = sweep::AwakeSeconds();
        size_t lastLines = 0;
        const ChildResult status = Wait(child, [&](const Child&)
        {
            std::ifstream in(results);
            size_t lines = 0;
            for (std::string line; std::getline(in, line);)
                ++lines;
            if (lines != lastLines)
            {
                lastLines = lines;
                lastProgress = sweep::AwakeSeconds();
                std::printf("levels: %zu/%zu done\n", lines, names.size());
                std::fflush(stdout);
            }
            return sweep::AwakeSeconds() - lastProgress > 300.0;
        });

        // Results so far, all segments.
        rows.clear();
        extras.clear();
        {
            std::ifstream in(results);
            for (std::string line; std::getline(in, line);)
            {
                if (line.empty())
                    continue;
                const nlohmann::json json = nlohmann::json::parse(line, nullptr, false);
                if (json.is_discarded())
                    continue;
                rows.push_back(sweep::FromJson(json));
                extras[rows.back().level] = json;
            }
        }
        for (const auto& [level, levelLog] : sweep::SplitLevelLog(sweep::ReadTextFile(log)))
        {
            sweep::LevelLog& merged = levelLogs[level];
            merged.errorLines += levelLog.errorLines;
            merged.errors.insert(merged.errors.end(), levelLog.errors.begin(), levelLog.errors.end());
            merged.driverWarnings += levelLog.driverWarnings;
            merged.samplerAudit += levelLog.samplerAudit;
            merged.sanitizerReports += levelLog.sanitizerReports;
            merged.sanitizerText += levelLog.sanitizerText;
            if (levelLog.signal != 0)
            {
                merged.signal = levelLog.signal;
                merged.backtrace = levelLog.backtrace;
            }
        }

        // Every name in this segment that has a row is done.
        size_t done = next;
        while (done < names.size() && extras.count(names[done]) != 0)
            ++done;
        if (done == names.size())
            break;
        // The child stopped inside names[done]: record it and carry on.
        const sweep::LevelResult failed = sweep::StoppedLevelRow(names[done], status);
        rows.push_back(failed);
        nlohmann::json json = sweep::ToJson(failed);
        std::ofstream(results, std::ios::app) << json.dump() << "\n";
        extras[failed.level] = json;
        next = done + 1;
        ++segment;
    }

    // Attribute [ERROR] lines and backtraces, then write the reports.
    nlohmann::json all = nlohmann::json::array();
    int passed = 0;
    std::ofstream tsv(config.out / "levels.tsv", std::ios::trunc);
    tsv << sweep::TsvHeader() << "\n";
    std::printf("\n%s\n", sweep::TsvHeader().c_str());
    for (sweep::LevelResult& row : rows)
    {
        const sweep::LevelLog& levelLog = levelLogs[row.level];
        row.loggedErrors = levelLog.errorLines;
        row.driverWarnings = levelLog.driverWarnings;
        row.samplerAudit = levelLog.samplerAudit;
        row.sanitizerReports = levelLog.sanitizerReports;
        if (levelLog.signal != 0 && row.signal == 0)
            row.signal = levelLog.signal;
        nlohmann::json json = extras.count(row.level) ? extras[row.level] : nlohmann::json::object();
        const nlohmann::json updated = sweep::ToJson(row);
        for (auto it = updated.begin(); it != updated.end(); ++it)
            json[it.key()] = it.value();
        json["error_lines"] = levelLog.errors;
        json["backtrace"] = levelLog.backtrace;
        json["sanitizer_report"] = levelLog.sanitizerText;
        all.push_back(json);
        passed += sweep::Passed(row) ? 1 : 0;
        tsv << sweep::ToTsv(row) << "\n";
        std::printf("%s\n", sweep::ToTsv(row).c_str());
    }
    // A sanitizer report before the first level (game start-up) fails the run.
    const int startupSanitizer = levelLogs[""].sanitizerReports;
    const bool allPassed = passed == static_cast<int>(names.size()) && rows.size() == names.size() &&
        startupSanitizer == 0;
    nlohmann::json report = { { "levels", all }, { "passed", passed }, { "total", names.size() },
        { "child_logs", segmentLogs }, { "startup_errors", levelLogs[""].errorLines },
        { "startup_sanitizer_reports", startupSanitizer }, { "startup_sanitizer_report", levelLogs[""].sanitizerText },
        { "frames_per_level", config.frames } };

    int exitCode = allPassed ? 0 : 1;
    if (!config.reference.empty())
    {
        const sweep::FolderCompare compare = sweep::CompareFolders(shots, config.reference, config.tolerance, config.maxFraction);
        report["compare"] = { { "reference", config.reference.string() }, { "compared", compare.compared },
            { "changed", compare.changed }, { "missing", compare.missing }, { "added", compare.added } };
        std::printf("compare (advisory): %d compared, %zu changed, %zu missing, %zu new\n", compare.compared,
            compare.changed.size(), compare.missing.size(), compare.added.size());
        for (const std::string& line : compare.changed)
            std::printf("  changed %s\n", line.c_str());
        // Advisory: the game is not deterministic, so changed images are
        // reported for a person to look at and never fail the sweep.
        report["compare"]["advisory"] = true;
    }
    std::ofstream(config.out / "levels.json", std::ios::trunc) << report.dump(2) << "\n";
    if (startupSanitizer != 0)
        std::printf("levels: %d sanitizer report(s) during start-up (see levels.json)\n", startupSanitizer);
    std::printf("\nlevels: %d/%zu passed (frames %d). Reports: %s, %s; screenshots: %s\n", passed, names.size(),
        config.frames, (config.out / "levels.tsv").c_str(), (config.out / "levels.json").c_str(), shots.c_str());
    return exitCode;
}

// --- cutscenes ---------------------------------------------------------------

int Cutscenes(const Config& config)
{
    constexpr int kCount = CUTSCENE_COUNT;
    const fs::path dir = config.out / "cutscenes";
    fs::create_directories(dir);
    struct Job { int index; Child child; fs::path log; fs::path result; };
    std::vector<Job> running;
    std::vector<nlohmann::json> rows(kCount);
    std::vector<ChildResult> statuses(kCount);
    int nextIndex = 0;
    while (nextIndex < kCount || !running.empty())
    {
        while (nextIndex < kCount && static_cast<int>(running.size()) < config.jobs)
        {
            Job job{ nextIndex, {}, config.out / "logs" / ("cutscene_" + std::to_string(nextIndex) + ".log"),
                dir / ("cutscene_" + std::to_string(nextIndex) + ".json") };
            std::error_code error;
            fs::remove(job.log, error);
            fs::remove(job.result, error);
            std::vector<std::string> args = { "cutscene" };
            const std::vector<std::string> common = Common(config, config.out / ("user-cutscene-" + std::to_string(nextIndex)));
            args.insert(args.end(), common.begin(), common.end());
            char seconds[32];
            std::snprintf(seconds, sizeof(seconds), "%g", config.fmvSeconds);
            args.insert(args.end(), { "--index", std::to_string(nextIndex), "--max-seconds", seconds,
                "--result", job.result.string() });
            job.child = Spawn(config, args, job.log);
            running.push_back(job);
            ++nextIndex;
        }
        for (auto it = running.begin(); it != running.end();)
        {
            int status = 0;
            // A failed spawn leaves pid -1: waitpid(-1) would reap a sibling and
            // KillStuck could then signal a reused pid, so finish it here instead.
            const bool spawnFailed = it->child.pid <= 0;
            const pid_t done = spawnFailed ? -1 : waitpid(it->child.pid, &status, WNOHANG);
            // Awake time, so a system sleep never times a movie out.
            const double age = it->child.timer.Awake();
            // A backstop: the child's own watchdog stops an uncapped movie at
            // 1.5 x its length + 60 s (the longest, about 7 minutes, at under
            // 12 minutes).
            const double limit = config.fmvSeconds > 0.0 ? config.fmvSeconds + 120.0 : 900.0;
            if (spawnFailed || done == it->child.pid || age > limit)
            {
                ChildResult r;
                if (spawnFailed)
                {
                    r.exited = true;
                    r.exitCode = -1;
                }
                else if (done != it->child.pid)
                {
                    KillStuck(it->child);
                    r = sweep::TimedOutOutcome();
                }
                else
                    r = sweep::OutcomeFromWaitStatus(status);
                r.seconds = age;
                r.slept = it->child.timer.Slept();
                statuses[it->index] = r;
                nlohmann::json row = ReadJson(it->result);
                if (!row.is_object())
                    row = nlohmann::json::object();
                const std::string text = sweep::ReadTextFile(it->log);
                row["index"] = it->index;
                row["child"] = Describe(r);
                row["error_lines"] = sweep::CountErrorLines(text);
                row["sanitizer_reports"] = sweep::CountSanitizerReports(text);
                const auto split = sweep::SplitLevelLog(text);
                std::vector<std::string> errors;
                for (const auto& [name, levelLog] : split)
                    errors.insert(errors.end(), levelLog.errors.begin(), levelLog.errors.end());
                row["errors"] = errors;
                row["log"] = it->log.string();
                const double media = row.value("media_seconds", 0.0);
                const double elapsed = row.value("elapsed_s", 0.0);
                const bool capped = row.value("capped", false);
                // Uncapped, the movie must have played (nearly) to its end.
                const bool playedThrough = capped ? elapsed >= config.fmvSeconds * 0.9
                                                  : (media > 0.0 && elapsed >= media * 0.9);
                row["played_through"] = playedThrough;
                // "stalled": the child's watchdog had to stop an uncapped movie.
                row["passed"] = r.exited && r.exitCode == 0 && row.value("queued", false) &&
                    row["error_lines"].get<int>() == 0 && row.value("gl_errors", 1) == 0 && playedThrough &&
                    row["sanitizer_reports"].get<int>() == 0 && !row.value("stalled", false);
                rows[it->index] = row;
                std::printf("cutscene %2d %-16s %s elapsed %.1fs media %.1fs errors %d %s\n", it->index,
                    row.value("video", "?").c_str(), Describe(r).c_str(), elapsed, media,
                    row["error_lines"].get<int>(), row["passed"].get<bool>() ? "PASS" : "FAIL");
                std::fflush(stdout);
                it = running.erase(it);
            }
            else
                ++it;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    int passed = 0;
    for (const nlohmann::json& row : rows)
        passed += row.value("passed", false) ? 1 : 0;
    nlohmann::json report = { { "cutscenes", rows }, { "passed", passed }, { "total", kCount },
        { "fmv_seconds", config.fmvSeconds } };
    std::ofstream(config.out / "cutscenes.json", std::ios::trunc) << report.dump(2) << "\n";
    std::printf("cutscenes: %d/%d passed (%s)\n", passed, kCount,
        config.fmvSeconds > 0.0 ? ("capped at " + std::to_string(config.fmvSeconds) + " s").c_str() : "whole movies");
    return passed == kCount ? 0 : 1;
}

// --- single-child checks -------------------------------------------------------

int Audio(const Config& config)
{
    const fs::path result = config.out / "audio.json";
    const fs::path log = config.out / "logs" / "audio.log";
    std::error_code error;
    fs::remove(log, error);
    std::vector<std::string> args = { "audio" };
    const std::vector<std::string> common = Common(config, config.out / "user-audio");
    args.insert(args.end(), common.begin(), common.end());
    args.insert(args.end(), { "--result", result.string() });
    constexpr double kAudioSeconds = 780.0;
    static_assert(WorstCaseSeconds(1, kAudioSeconds) <= kCtestTimeoutSeconds, "audio outlasts its CTest timeout");
    const ChildResult status = RunChild(config, args, log, kAudioSeconds);
    nlohmann::json report = ReadJson(result);
    if (!report.is_object())
        report = nlohmann::json::object();
    const std::string text = sweep::ReadTextFile(log);
    report["child"] = Describe(status);
    report["error_lines"] = sweep::CountErrorLines(text);
    report["sanitizer_reports"] = sweep::CountSanitizerReports(text);
    nlohmann::json errorsByItem = nlohmann::json::object();
    for (const auto& [name, levelLog] : sweep::SplitLevelLog(text))
        if (levelLog.errorLines != 0)
            errorsByItem[name.empty() ? "startup" : name] = levelLog.errors;
    report["errors_by_item"] = errorsByItem;
    const bool passed = status.exited && status.exitCode == 0 &&
        report.value("music_loaded", -1) == report.value("music_total", -2) &&
        report.value("music_started", -1) == report.value("music_total", -2) &&
        report.value("banks_loaded", -1) == report.value("banks_total", -2) &&
        report.value("wav_loaded", -1) == report.value("wav_total", -2) &&
        report.value("vag_started", -1) == report.value("vag_tried", -2) &&
        report["error_lines"].get<int>() == 0 && report["sanitizer_reports"].get<int>() == 0;
    report["passed"] = passed;
    std::ofstream(result, std::ios::trunc) << report.dump(2) << "\n";
    std::printf("audio: music %d/%d loaded, %d started; banks %d/%d; wav %d/%d; vag %d/%d; [ERROR] lines %d; %s\n",
        report.value("music_loaded", 0), report.value("music_total", 0), report.value("music_started", 0),
        report.value("banks_loaded", 0), report.value("banks_total", 0), report.value("wav_loaded", 0),
        report.value("wav_total", 0), report.value("vag_started", 0), report.value("vag_tried", 0),
        report["error_lines"].get<int>(), passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}

int Saves(const Config& config)
{
    const fs::path userDir = config.out / "user-saves";
    std::error_code error;
    fs::remove_all(userDir, error);
    const fs::path expected = config.out / "saves_expected.bin";
    const fs::path writeResult = config.out / "saves_write.json";
    const fs::path readResult = config.out / "saves_read.json";
    std::vector<std::string> args = { "save-write" };
    const std::vector<std::string> common = Common(config, userDir);
    args.insert(args.end(), common.begin(), common.end());
    args.insert(args.end(), { "--level", config.level, "--expected", expected.string(), "--result", writeResult.string() });
    fs::remove(config.out / "logs" / "saves_write.log", error);
    fs::remove(config.out / "logs" / "saves_read.log", error);
    const ChildResult w = RunChild(config, args, config.out / "logs" / "saves_write.log", 300.0);
    args[0] = "save-read";
    for (size_t i = 0; i < args.size(); ++i)
        if (args[i] == "--result")
            args[i + 1] = readResult.string();
    const ChildResult r = RunChild(config, args, config.out / "logs" / "saves_read.log", 300.0);
    const nlohmann::json written = ReadJson(writeResult);
    const nlohmann::json read = ReadJson(readResult);

    // The default location, computed with no override (no directory made).
    const fs::path previous = cane::g_userOverride;
    cane::g_userOverride.clear();
    const fs::path defaultDir = cane::UserDirectory();
    cane::g_userOverride = previous;
    const char* home = std::getenv("HOME");
    const fs::path expectedDefault = fs::path(home != nullptr ? home : "") / "Library" / "Application Support" / "ProjectCane";
    const std::string writeLog = sweep::ReadTextFile(config.out / "logs" / "saves_write.log");
    const std::string readLog = sweep::ReadTextFile(config.out / "logs" / "saves_read.log");
    const int errorLines = sweep::CountErrorLines(writeLog) + sweep::CountErrorLines(readLog);
    const int sanitizerReports = sweep::CountSanitizerReports(writeLog) + sweep::CountSanitizerReports(readLog);

    nlohmann::json report = { { "write", written }, { "read", read }, { "write_child", Describe(w) },
        { "read_child", Describe(r) }, { "default_user_dir", defaultDir.string() },
        { "default_is_application_support", defaultDir == expectedDefault }, { "user_dir_used", userDir.string() },
        { "files_in_user_dir", nlohmann::json::array() }, { "error_lines", errorLines },
        { "sanitizer_reports", sanitizerReports } };
    for (auto it = fs::recursive_directory_iterator(userDir, error); !error && it != fs::recursive_directory_iterator(); it.increment(error))
        if (it->is_regular_file())
            report["files_in_user_dir"].push_back(fs::relative(it->path(), userDir).string());
    const bool passed = w.exited && w.exitCode == 0 && r.exited && r.exitCode == 0 &&
        written.value("saved", false) && written.value("slot_file_exists", false) &&
        read.value("bytes_equal", false) && read.value("load_current_save", false) && read.value("live_equal", false) &&
        defaultDir == expectedDefault && errorLines == 0 && sanitizerReports == 0;
    report["passed"] = passed;
    std::ofstream(config.out / "saves.json", std::ios::trunc) << report.dump(2) << "\n";
    std::printf("saves: write %s, read %s, round trip %s, live %s, default dir %s (%s); %s\n",
        Describe(w).c_str(), Describe(r).c_str(), read.value("bytes_equal", false) ? "equal" : "DIFFERENT",
        read.value("live_equal", false) ? "equal" : "DIFFERENT", defaultDir.c_str(),
        defaultDir == expectedDefault ? "Application Support" : "UNEXPECTED", passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}

int Determinism(const Config& config)
{
    std::vector<std::string> names;
    sweep::FrameHashes runs[2];
    nlohmann::json results[2];
    ChildResult statuses[2];
    int sanitizerReports = 0;
    for (int run = 0; run < 2; ++run)
    {
        const fs::path userDir = config.out / ("user-determinism-" + std::to_string(run));
        std::error_code error;
        fs::remove_all(userDir, error);
        const fs::path hashes = config.out / ("determinism_" + std::to_string(run) + ".bin");
        const fs::path result = config.out / ("determinism_" + std::to_string(run) + ".json");
        const fs::path log = config.out / "logs" / ("determinism_" + std::to_string(run) + ".log");
        fs::remove(log, error);
        std::vector<std::string> args = { "hash" };
        const std::vector<std::string> common = Common(config, userDir);
        args.insert(args.end(), common.begin(), common.end());
        args.insert(args.end(), { "--level", config.level, "--frames", std::to_string(config.frames),
            "--hashes", hashes.string(), "--result", result.string() });
        // Two runs, a few seconds each at the default 300 frames.
        constexpr double kRunSeconds = 360.0;
        static_assert(WorstCaseSeconds(2, kRunSeconds) <= kCtestTimeoutSeconds, "determinism outlasts its CTest timeout");
        statuses[run] = RunChild(config, args, log, kRunSeconds);
        sweep::ReadFrameHashes(hashes, runs[run]);
        sanitizerReports += sweep::CountSanitizerReports(sweep::ReadTextFile(log));
        results[run] = ReadJson(result);
    }
    const sweep::Divergence d = sweep::FirstDivergence(runs[0], runs[1]);
    nlohmann::json report = { { "level", config.level }, { "frames", config.frames },
        { "run_children", { Describe(statuses[0]), Describe(statuses[1]) } },
        { "frames_recorded", { runs[0].size(), runs[1].size() } }, { "diverged", d.diverged },
        { "frames_compared", d.framesCompared },
        { "nonfixed_dt_frames", { results[0].value("nonfixed_dt_frames", -1), results[1].value("nonfixed_dt_frames", -1) } } };
    if (d.diverged)
    {
        report["first_divergent_frame"] = d.frame;
        report["first_divergent_object_index"] = d.object;
        report["object_count_mismatch"] = d.objectCountMismatch;
        const nlohmann::json objects = results[0].value("object_list", nlohmann::json::array());
        if (d.object < objects.size())
            report["first_divergent_object"] = objects[d.object];
        // How widespread: diverged objects per frame from the first divergence.
        nlohmann::json spread = nlohmann::json::array();
        for (size_t f = d.frame; f < (std::min)(runs[0].size(), runs[1].size()) && spread.size() < 10; f += 30)
        {
            size_t differing = 0;
            const size_t n = (std::min)(runs[0][f].size(), runs[1][f].size());
            for (size_t o = 0; o < n; ++o)
                differing += runs[0][f][o] != runs[1][f][o] ? 1 : 0;
            spread.push_back({ { "frame", f }, { "objects_differing", differing }, { "objects", n } });
        }
        report["spread"] = spread;
    }
    const bool ran = statuses[0].exited && statuses[0].exitCode == 0 && statuses[1].exited && statuses[1].exitCode == 0 &&
        runs[0].size() == static_cast<size_t>(config.frames) && runs[1].size() == static_cast<size_t>(config.frames);
    report["ran"] = ran;
    report["sanitizer_reports"] = sanitizerReports;
    std::ofstream(config.out / "determinism.json", std::ios::trunc) << report.dump(2) << "\n";
    if (d.diverged)
        std::printf("determinism: '%s' %d frames: runs DIVERGE at frame %zu, object %zu (%s)\n", config.level.c_str(),
            config.frames, d.frame, d.object, report.value("first_divergent_object", nlohmann::json()).dump().c_str());
    else
        std::printf("determinism: '%s' %d frames: runs match (%zu frames compared)\n", config.level.c_str(),
            config.frames, d.framesCompared);
    if (sanitizerReports != 0)
        std::printf("determinism: %d sanitizer report(s) in the run logs\n", sanitizerReports);
    // A baseline for the float-mode wave: divergence is recorded, not failed.
    return ran && sanitizerReports == 0 ? 0 : 1;
}

int Display(const Config& config, bool fullscreen)
{
    const std::string name = fullscreen ? "display_fullscreen" : "display";
    const fs::path result = config.out / (name + ".json");
    const fs::path log = config.out / "logs" / (name + ".log");
    std::error_code error;
    fs::remove(log, error);
    std::vector<std::string> args = { "display" };
    const std::vector<std::string> common = Common(config, config.out / ("user-" + name));
    args.insert(args.end(), common.begin(), common.end());
    args.insert(args.end(), { "--level", config.level, "--shots", (config.out / "shots-display").string(),
        "--result", result.string() });
    if (fullscreen)
        args.push_back("--fullscreen");
    const ChildResult status = RunChild(config, args, log, 300.0);
    nlohmann::json report = ReadJson(result);
    if (!report.is_object())
        report = nlohmann::json::object();
    report["child"] = Describe(status);
    const std::string text = sweep::ReadTextFile(log);
    report["error_lines"] = sweep::CountErrorLines(text);
    report["sanitizer_reports"] = sweep::CountSanitizerReports(text);
    const bool passed = status.exited && status.exitCode == 0 && report.value("passed", false) &&
        report["error_lines"].get<int>() == 0 && report["sanitizer_reports"].get<int>() == 0;
    report["passed"] = passed;
    std::ofstream(result, std::ios::trunc) << report.dump(2) << "\n";
    for (const nlohmann::json& step : report.value("steps", nlohmann::json::array()))
        std::printf("  %-55s %s render %s present %s cel viewport %s width %.3f gl %d\n",
            step.value("step", "").c_str(), step.value("passed", false) ? "ok  " : "FAIL",
            step["render"].dump().c_str(), step["present"].dump().c_str(),
            step["cel_viewport_uniform"].dump().c_str(), step.value("cel_line_width_uniform", 0.0),
            step.value("gl_errors", -1));
    std::printf("%s: %s, [ERROR] lines %d, sanitizer reports %d: %s\n", name.c_str(), Describe(status).c_str(),
        report["error_lines"].get<int>(), report["sanitizer_reports"].get<int>(), passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}

// Log totals every single-child check below fails on.
struct ChildLog
{
    int errorLines = 0;
    int sanitizerReports = 0;
    int driverWarnings = 0;
    int samplerAudit = 0;
    nlohmann::json errors = nlohmann::json::array();
    // The [ERROR] lines by level ("startup" before the first).
    nlohmann::json errorsByLevel = nlohmann::json::object();
};

ChildLog ReadChildLog(const fs::path& log)
{
    ChildLog totals;
    const std::string text = sweep::ReadTextFile(log);
    totals.sanitizerReports = sweep::CountSanitizerReports(text);
    for (const auto& [name, levelLog] : sweep::SplitLevelLog(text))
    {
        totals.errorLines += levelLog.errorLines;
        totals.driverWarnings += levelLog.driverWarnings;
        totals.samplerAudit += levelLog.samplerAudit;
        for (const std::string& error : levelLog.errors)
            totals.errors.push_back(error);
        if (levelLog.errorLines != 0)
            totals.errorsByLevel[name.empty() ? "startup" : name] = levelLog.errors;
    }
    return totals;
}

void AddLogReasons(const ChildLog& log, std::vector<std::string>& reasons)
{
    if (log.errorLines != 0)
        reasons.push_back(std::to_string(log.errorLines) + " [ERROR] lines");
    if (log.sanitizerReports != 0)
        reasons.push_back(std::to_string(log.sanitizerReports) + " sanitizer reports");
    if (log.driverWarnings != 0)
        reasons.push_back(std::to_string(log.driverWarnings) + " GL driver warnings");
    if (log.samplerAudit != 0)
        reasons.push_back(std::to_string(log.samplerAudit) + " samplers on empty units (sampler audit)");
}

// A Stealthy Approach's van door (sweep_child.cpp, ModeVanDoor): the door
// opens with no input, Sly plays on outside the van, and Prowling the
// Grounds loads and plays after it, all without GL errors, [ERROR] lines,
// sanitizer reports or replaced sidebags.
int VanDoor(const Config& config)
{
    const fs::path result = config.out / "van_door.json";
    const fs::path log = config.out / "logs" / "van_door.log";
    const fs::path shots = config.out / "shots-van-door";
    std::error_code error;
    fs::remove(log, error);
    fs::remove(result, error);
    fs::remove_all(shots, error);
    std::vector<std::string> args = { "van-door" };
    const std::vector<std::string> common = Common(config, config.out / "user-van-door");
    args.insert(args.end(), common.begin(), common.end());
    args.insert(args.end(), { "--shots", shots.string(), "--result", result.string() });
    const ChildResult status = RunChild(config, args, log, 600.0);
    nlohmann::json report = ReadJson(result);
    if (!report.is_object())
        report = nlohmann::json::object();
    const ChildLog childLog = ReadChildLog(log);
    report["child"] = Describe(status);
    report["log"] = log.string();
    report["error_lines"] = childLog.errorLines;
    report["sanitizer_reports"] = childLog.sanitizerReports;
    report["errors_by_level"] = childLog.errorsByLevel;
    report["driver_warnings"] = childLog.driverWarnings;
    report["sampler_audit"] = childLog.samplerAudit;
    const int driverWarnings = childLog.driverWarnings;
    const int samplerAudit = childLog.samplerAudit;

    const nlohmann::json stealthy = report.value("stealthy", nlohmann::json::object());
    const nlohmann::json next = report.value("next", nlohmann::json::object());
    std::vector<std::string> reasons;
    if (!(status.exited && status.exitCode == 0))
        reasons.push_back("child " + Describe(status));
    if (!stealthy.value("loaded", false))
        reasons.push_back("A Stealthy Approach did not load");
    else if (!report.value("door_opened", false))
        reasons.push_back("the van door never opened (state machine 1863 state " +
            std::to_string(report.value("door_sm_state_after_wait", -1)) + ")");
    else if (report.value("frames_after_open", 0) < 600)
        reasons.push_back("only " + std::to_string(report.value("frames_after_open", 0)) + " frames after the door opened");
    if (stealthy.value("gl_errors", 0) != 0)
        reasons.push_back(std::to_string(stealthy.value("gl_errors", 0)) + " GL errors in A Stealthy Approach (" +
            std::to_string(report.value("tour_gl_errors", 0)) + " during the clue tour)");
    if (report.value("door_opened", false) && report.value("tour_cane_ribbon_frames", 0) == 0)
        reasons.push_back("the clue tour never drew a cane ribbon");
    if (!next.value("loaded", false))
        reasons.push_back(next.value("level", std::string("next level")) + " did not load after it");
    if (next.value("gl_errors", 0) != 0)
        reasons.push_back(std::to_string(next.value("gl_errors", 0)) + " GL errors in " + next.value("level", std::string()));
    if (driverWarnings != 0)
        reasons.push_back(std::to_string(driverWarnings) + " GL driver warnings");
    if (samplerAudit != 0)
        reasons.push_back(std::to_string(samplerAudit) + " samplers on empty units (sampler audit)");
    for (const nlohmann::json* level : { &stealthy, &next })
        if (level->value("nan_inf", 0) + level->value("absurd", 0) != 0)
            reasons.push_back("bad transforms in " + level->value("level", std::string()));
    const int rebuilds = stealthy.value("sidebag_rebuilds", 0) + next.value("sidebag_rebuilds", 0);
    if (rebuilds != 0)
        reasons.push_back(std::to_string(rebuilds) + " valid sidebags replaced (custom options lost)");
    if (report["error_lines"].get<int>() != 0)
        reasons.push_back(std::to_string(report["error_lines"].get<int>()) + " [ERROR] lines");
    if (report["sanitizer_reports"].get<int>() != 0)
        reasons.push_back(std::to_string(report["sanitizer_reports"].get<int>()) + " sanitizer reports");
    const bool passed = reasons.empty();
    report["failure_reasons"] = reasons;
    report["passed"] = passed;
    std::ofstream(result, std::ios::trunc) << report.dump(2) << "\n";
    std::printf("van-door: %s; door %s at frame %d, %d frames after; GL errors %d before / %d after the door opened, "
        "%d in %s; clues collected %d/%d; [ERROR] lines %d; sidebags replaced %d; driver warnings %d; "
        "sampler audit %d; %s\n",
        Describe(status).c_str(), report.value("door_opened", false) ? "opened" : "NOT opened",
        report.value("open_frame", -1), report.value("frames_after_open", 0), report.value("gl_errors_before_open", 0),
        report.value("gl_errors_after_open", 0), next.value("gl_errors", 0), next.value("level", std::string("?")).c_str(),
        report.value("clues_collected", -1), report.value("clues_in_level", -1),
        report["error_lines"].get<int>(), rebuilds, driverWarnings, samplerAudit, passed ? "PASS" : "FAIL");
    std::printf("  clue tour: %d GL errors, %d frames with a cane ribbon\n", report.value("tour_gl_errors", 0),
        report.value("tour_cane_ribbon_frames", 0));
    for (const std::string& reason : reasons)
        std::printf("  %s\n", reason.c_str());
    std::printf("van-door: report %s, screenshots %s\n", result.c_str(), shots.c_str());
    return passed ? 0 : 1;
}

// Prowling the Grounds' dialog triggers (sweep_child.cpp, ModeHubTour): no GL
// errors, driver warnings or empty sampler units while Bentley's TV dialogs
// play.
int HubTour(const Config& config)
{
    const fs::path result = config.out / "hub_tour.json";
    const fs::path log = config.out / "logs" / "hub_tour.log";
    std::error_code error;
    fs::remove(log, error);
    fs::remove(result, error);
    std::vector<std::string> args = { "hub-tour" };
    const std::vector<std::string> common = Common(config, config.out / "user-hub-tour");
    args.insert(args.end(), common.begin(), common.end());
    args.insert(args.end(), { "--result", result.string() });
    const ChildResult status = RunChild(config, args, log, 600.0);
    nlohmann::json report = ReadJson(result);
    if (!report.is_object())
        report = nlohmann::json::object();
    const ChildLog childLog = ReadChildLog(log);
    const nlohmann::json level = report.value("level", nlohmann::json::object());
    std::vector<std::string> reasons;
    if (!(status.exited && status.exitCode == 0))
        reasons.push_back("child " + Describe(status));
    if (!level.value("loaded", false))
        reasons.push_back(level.value("level", std::string("level")) + " did not load");
    if (report.value("stops", 0) == 0)
        reasons.push_back("no dialog triggers visited");
    else if (report.value("cane_ribbon_frames", 0) == 0)
        reasons.push_back("the tour never drew a cane ribbon");
    if (level.value("gl_errors", 0) != 0)
        reasons.push_back(std::to_string(level.value("gl_errors", 0)) + " GL errors");
    if (level.value("sidebag_rebuilds", 0) != 0)
        reasons.push_back(std::to_string(level.value("sidebag_rebuilds", 0)) + " valid sidebags replaced");
    AddLogReasons(childLog, reasons);
    report["child"] = Describe(status);
    report["log"] = log.string();
    report["driver_warnings"] = childLog.driverWarnings;
    report["sampler_audit"] = childLog.samplerAudit;
    report["error_lines"] = childLog.errors;
    report["failure_reasons"] = reasons;
    report["passed"] = reasons.empty();
    std::ofstream(result, std::ios::trunc) << report.dump(2) << "\n";
    std::printf("hub-tour: %s; %d stops, %d frames, %d with a cane ribbon; GL errors %d; driver warnings %d; "
        "sampler audit %d; %s\n",
        Describe(status).c_str(), report.value("stops", 0), level.value("frames", 0),
        report.value("cane_ribbon_frames", 0), level.value("gl_errors", 0),
        childLog.driverWarnings, childLog.samplerAudit, reasons.empty() ? "PASS" : "FAIL");
    for (const std::string& reason : reasons)
        std::printf("  %s\n", reason.c_str());
    return reasons.empty() ? 0 : 1;
}

// A charm swirling or sticking when a level door's exit fires inside the
// object update (sweep_child.cpp, ModeCharmExit), and the three controls.
// Every case must finish without a crash; in the two crash cases the charm
// must end up falling (the default PZI disables collection).
int CharmExit(const Config& config)
{
    constexpr const char* cases[] = { "in-update", "in-stick", "no-charm", "after-swirl", "outside" };
    // Each case takes a few seconds; together they must end before CTest's
    // timeout.
    constexpr double kCaseSeconds = 100.0;
    static_assert(WorstCaseSeconds(static_cast<int>(std::size(cases)), kCaseSeconds) <= kCtestTimeoutSeconds,
        "charm-exit outlasts its CTest timeout");
    nlohmann::json rows = nlohmann::json::array();
    int passed = 0;
    for (const char* name : cases)
    {
        const fs::path result = config.out / ("charm_exit_" + std::string(name) + ".json");
        const fs::path log = config.out / "logs" / ("charm_exit_" + std::string(name) + ".log");
        std::error_code error;
        fs::remove(log, error);
        fs::remove(result, error);
        std::vector<std::string> args = { "charm-exit" };
        const std::vector<std::string> common = Common(config, config.out / ("user-charm-exit-" + std::string(name)));
        args.insert(args.end(), common.begin(), common.end());
        args.insert(args.end(), { "--case", name, "--result", result.string() });
        const ChildResult status = RunChild(config, args, log, kCaseSeconds);
        nlohmann::json row = ReadJson(result);
        if (!row.is_object())
            row = nlohmann::json::object();
        const ChildLog childLog = ReadChildLog(log);
        const nlohmann::json level = row.value("level", nlohmann::json::object());
        std::vector<std::string> reasons;
        if (!(status.exited && status.exitCode == 0))
            reasons.push_back("child " + Describe(status) + (status.signal != 0 ? " (crashed)" : ""));
        if (row.empty())
            reasons.push_back("no result written");
        else if (!level.value("loaded", false))
            reasons.push_back("level did not load");
        if (!row.empty() && !row.value("exit_found", false))
            reasons.push_back("exit to Treasure in the Depths not found");
        else if (!row.empty() && !row.value("exit_triggered", false))
            reasons.push_back("the exit never fired");
        if (!row.empty() && std::string(name) != "no-charm" && !row.value("charm_created", false))
            reasons.push_back("no swirling charm after CreateSwCharm");
        if (!row.empty() && (std::string(name) == "in-update" || std::string(name) == "in-stick"))
        {
            const int expected = std::string(name) == "in-update" ? DPRIZES_Swirl : DPRIZES_Stick;
            if (row.value("charm_state_at_exit", -1) != expected)
                reasons.push_back("charm was not in state " + std::to_string(expected) + " when the exit fired (state " +
                    std::to_string(row.value("charm_state_at_exit", -1)) + ")");
            if (row.value("player_after_exit", true))
                reasons.push_back("the exit did not take Sly off the player list");
            // With the player gone the charm has nothing to fly to: it must
            // not jump (e.g. towards the world origin) around the exit.
            double maxStep = 0.0;
            const nlohmann::json positions = row.value("charm_positions", nlohmann::json::array());
            for (size_t i = 1; i < positions.size(); ++i)
            {
                double d2 = 0.0;
                for (int k = 0; k < 3; ++k)
                {
                    const double d = positions[i]["pos"][k].get<double>() - positions[i - 1]["pos"][k].get<double>();
                    d2 += d * d;
                }
                maxStep = (std::max)(maxStep, std::sqrt(d2));
            }
            row["charm_max_step_around_exit"] = maxStep;
            if (positions.size() < 2)
                reasons.push_back("no charm positions recorded around the exit");
            else if (maxStep > 100.0)
                reasons.push_back("charm jumped " + std::to_string(static_cast<int>(maxStep)) + " units in one frame around the exit");
            bool fell = false;
            for (const nlohmann::json& change : row.value("charm_states", nlohmann::json::array()))
                fell |= change.value("frame", -1) > row.value("exit_at", 0) && change.value("state", -1) == DPRIZES_Fall;
            if (!fell)
                reasons.push_back("charm never fell after the exit");
        }
        if (level.value("gl_errors", 0) != 0)
            reasons.push_back(std::to_string(level.value("gl_errors", 0)) + " GL errors");
        AddLogReasons(childLog, reasons);
        row["child"] = Describe(status);
        row["log"] = log.string();
        row["failure_reasons"] = reasons;
        row["passed"] = reasons.empty();
        passed += reasons.empty() ? 1 : 0;
        std::printf("charm-exit %-11s %s; charm states %s; %s\n", name, Describe(status).c_str(),
            row.value("charm_states", nlohmann::json::array()).dump().c_str(), reasons.empty() ? "PASS" : "FAIL");
        for (const std::string& reason : reasons)
            std::printf("  %s\n", reason.c_str());
        rows.push_back(row);
    }
    const int total = static_cast<int>(std::size(cases));
    std::ofstream(config.out / "charm_exit.json", std::ios::trunc)
        << nlohmann::json({ { "cases", rows }, { "passed", passed }, { "total", total } }).dump(2) << "\n";
    std::printf("charm-exit: %d/%d cases passed\n", passed, total);
    return passed == total ? 0 : 1;
}

// --- behaviour checks (sweep_behaviour_child.cpp, sweep_behaviour.cpp) ---------

// Runs `mode` over every level in children that each take the rest of the
// list, so a crash costs one level; returns each level's result line (a
// stand-in row for a level a child died in) and its log totals.
struct LevelRun
{
    std::map<std::string, nlohmann::json> rows;
    std::map<std::string, sweep::LevelLog> logs;
    std::vector<std::string> childLogs;
};

LevelRun RunLevelChildren(const Config& config, const std::string& mode, const std::vector<std::string>& extra,
    const std::string& prefix, const std::vector<std::string>& names, double secondsPerLevel)
{
    LevelRun run;
    const fs::path results = config.out / (prefix + ".jsonl");
    std::error_code error;
    fs::remove(results, error);
    // A fresh user folder: saves from an earlier run never carry over.
    fs::remove_all(config.out / ("user-" + prefix), error);
    size_t next = 0;
    int segment = 0;
    while (next < names.size())
    {
        const fs::path list = config.out / "children" / (prefix + "_" + std::to_string(segment) + ".txt");
        fs::create_directories(list.parent_path());
        {
            std::ofstream out(list, std::ios::trunc);
            for (size_t i = next; i < names.size(); ++i)
                out << names[i] << "\n";
        }
        const fs::path log = config.out / "logs" / (prefix + "_" + std::to_string(segment) + ".log");
        fs::remove(log, error);
        run.childLogs.push_back(log.string());
        std::vector<std::string> args = { mode };
        const std::vector<std::string> common = Common(config, config.out / ("user-" + prefix));
        args.insert(args.end(), common.begin(), common.end());
        args.insert(args.end(), { "--list", list.string(), "--results", results.string() });
        args.insert(args.end(), extra.begin(), extra.end());
        std::printf("%s: starting child for %zu level(s) from '%s'\n", prefix.c_str(), names.size() - next, names[next].c_str());
        const Child child = Spawn(config, args, log);
        double lastProgress = sweep::AwakeSeconds();
        size_t lastLines = 0;
        const ChildResult status = Wait(child, [&](const Child&)
        {
            std::ifstream in(results);
            size_t lines = 0;
            for (std::string line; std::getline(in, line);)
                ++lines;
            if (lines != lastLines)
            {
                lastLines = lines;
                lastProgress = sweep::AwakeSeconds();
                std::printf("%s: %zu/%zu done\n", prefix.c_str(), lines, names.size());
            }
            return sweep::AwakeSeconds() - lastProgress > secondsPerLevel;
        });
        run.rows.clear();
        {
            std::ifstream in(results);
            for (std::string line; std::getline(in, line);)
            {
                const nlohmann::json json = nlohmann::json::parse(line, nullptr, false);
                if (json.is_object() && json.contains("level"))
                    run.rows[json["level"].get<std::string>()] = json;
            }
        }
        for (const auto& [level, levelLog] : sweep::SplitLevelLog(sweep::ReadTextFile(log)))
        {
            sweep::LevelLog& merged = run.logs[level];
            merged.errorLines += levelLog.errorLines;
            merged.errors.insert(merged.errors.end(), levelLog.errors.begin(), levelLog.errors.end());
            merged.driverWarnings += levelLog.driverWarnings;
            merged.samplerAudit += levelLog.samplerAudit;
            merged.sanitizerReports += levelLog.sanitizerReports;
            merged.sanitizerText += levelLog.sanitizerText;
            if (levelLog.signal != 0)
            {
                merged.signal = levelLog.signal;
                merged.backtrace = levelLog.backtrace;
            }
        }
        size_t done = next;
        while (done < names.size() && run.rows.count(names[done]) != 0)
            ++done;
        if (done == names.size())
            break;
        nlohmann::json failed = { { "level", names[done] }, { "loaded", false },
            { "child", "child " + Describe(status) + " during this level" } };
        std::ofstream(results, std::ios::app) << failed.dump() << "\n";
        run.rows[names[done]] = failed;
        next = done + 1;
        ++segment;
    }
    return run;
}

// Failures every level row shares: the child died in it, it did not load,
// its log has [ERROR] lines, sanitizer reports, driver warnings or empty
// sampler units, or a sidebag was replaced.
std::vector<std::string> CommonReasons(const nlohmann::json& row, const sweep::LevelLog& log)
{
    std::vector<std::string> reasons;
    if (row.contains("child"))
        reasons.push_back(row["child"].get<std::string>());
    else if (!row.value("loaded", false))
        reasons.push_back("did not load");
    if (log.signal != 0)
        reasons.push_back("signal " + std::to_string(log.signal));
    if (log.errorLines != 0)
        reasons.push_back(std::to_string(log.errorLines) + " [ERROR] lines");
    if (log.sanitizerReports != 0)
        reasons.push_back(std::to_string(log.sanitizerReports) + " sanitizer reports");
    if (log.driverWarnings != 0)
        reasons.push_back(std::to_string(log.driverWarnings) + " GL driver warnings");
    if (log.samplerAudit != 0)
        reasons.push_back(std::to_string(log.samplerAudit) + " samplers on empty units");
    if (row.value("sidebag_rebuilds", 0) != 0)
        reasons.push_back(std::to_string(row.value("sidebag_rebuilds", 0)) + " valid sidebags replaced");
    return reasons;
}

// The known findings, read and checked before a run starts so a bad file
// fails at once. False (and a message) when the file is given but unusable.
bool LoadKnown(const Config& config, const char* check, std::vector<sweep::KnownFinding>& known)
{
    known.clear();
    if (config.known.empty())
        return true;
    std::ifstream in(config.known);
    const nlohmann::json json = nlohmann::json::parse(in, nullptr, false);
    const std::vector<std::string> problems = json.is_discarded()
        ? std::vector<std::string>{ "not JSON (or not there)" } : sweep::KnownFindingProblems(json);
    for (const std::string& problem : problems)
        std::printf("%s: known findings %s: %s\n", check, config.known.c_str(), problem.c_str());
    if (!problems.empty())
        return false;
    known = sweep::KnownFindingsFromJson(json);
    return true;
}

// Moves the reasons that are known findings out of `reasons`; returns them
// with their write-ups and records which entries matched in `used`.
nlohmann::json TakeKnown(const std::vector<sweep::KnownFinding>& known, const std::string& check,
    const std::string& level, std::vector<std::string>& reasons, std::set<size_t>& used)
{
    nlohmann::json taken = nlohmann::json::array();
    std::vector<std::string> left;
    for (const std::string& reason : reasons)
    {
        const std::string writeup = sweep::KnownWriteup(known, check, level, reason, &used);
        if (writeup.empty())
            left.push_back(reason);
        else
            taken.push_back({ { "reason", reason }, { "writeup", writeup } });
    }
    reasons.swap(left);
    return taken;
}

// Known entries for levels that ran but matched nothing: listed (advisory)
// so a fixed or reworded finding does not keep a stale entry.
nlohmann::json ReportStale(const std::vector<sweep::KnownFinding>& known, const std::string& check,
    const std::vector<std::string>& ran, const std::set<size_t>& used)
{
    const std::vector<std::string> stale = sweep::StaleKnown(known, check, ran, used);
    for (const std::string& line : stale)
        std::printf("%s: stale known finding (advisory): %s\n", check.c_str(), line.c_str());
    return stale;
}

std::vector<std::string> Strings(const nlohmann::json& list)
{
    std::vector<std::string> out;
    for (const nlohmann::json& item : list)
        out.push_back(item.is_string() ? item.get<std::string>() : item.dump());
    return out;
}

// Checks 1 and 2: each level plays 15 s with no input while the child
// records its structural trace and the motion of every applied segment.
// Fails on a segment whose driven object never moved while its goal did
// ("applied, target static"), on a must-happen event that never happened,
// and on the usual crash/log/sidebag reasons. The baseline difference, the
// segments that resolve no object and the stalled state machine
// transitions are reported only.
int Behaviour(const Config& config)
{
    const std::vector<std::string> names = config.levels.empty() ? sweep_game::LevelNames() : config.levels;
    const int frames = config.framesGiven ? config.frames : 900;
    const fs::path traces = config.out / "traces";
    std::error_code error;
    fs::remove_all(traces, error);
    std::vector<sweep::MustHappen> must;
    if (!config.mustHappen.empty())
    {
        std::ifstream in(config.mustHappen);
        const nlohmann::json json = nlohmann::json::parse(in, nullptr, false);
        if (json.is_discarded())
        {
            std::printf("behaviour: cannot read %s\n", config.mustHappen.c_str());
            return 1;
        }
        // A list that would check nothing fails before the run.
        const std::vector<std::string> problems = sweep::MustHappenProblems(json, sweep_game::LevelNames());
        for (const std::string& problem : problems)
            std::printf("behaviour: must-happen %s: %s\n", config.mustHappen.c_str(), problem.c_str());
        if (!problems.empty())
            return 1;
        must = sweep::MustHappenFromJson(json);
    }
    fs::path baseline = config.baseline;
    if (baseline.empty() && fs::is_directory(config.out / "baselines" / "behaviour", error))
        baseline = config.out / "baselines" / "behaviour";
    std::vector<sweep::KnownFinding> known;
    if (!LoadKnown(config, "behaviour", known))
        return 1;
    std::set<size_t> knownUsed;
    std::vector<std::string> loadedLevels;
    int knownTotal = 0;
    std::vector<std::string> extra = { "--traces", traces.string(), "--frames", std::to_string(frames) };
    if (config.freezeRoot >= 0)
        extra.insert(extra.end(), { "--freeze-root", std::to_string(config.freezeRoot) });
    const LevelRun run = RunLevelChildren(config, "behaviour", extra, "behaviour", names, 300.0);

    nlohmann::json levels = nlohmann::json::array();
    int passed = 0, staticTotal = 0, missingTotal = 0, addedTotal = 0, removedTotal = 0;
    std::printf("\n%-28s %-6s %7s %7s %6s %6s %6s %8s %8s  %s\n", "level", "result", "events", "tracks", "static",
        "unres", "stall", "+base", "-base", "reasons");
    for (const std::string& name : names)
    {
        nlohmann::json row = run.rows.count(name) ? run.rows.at(name) : nlohmann::json{ { "level", name } };
        const sweep::LevelLog log = run.logs.count(name) ? run.logs.at(name) : sweep::LevelLog{};
        std::vector<std::string> reasons = CommonReasons(row, log);
        const nlohmann::json statics = row.value("static_targets", nlohmann::json::array());
        for (const nlohmann::json& s : statics)
            reasons.push_back("applied, target static: " + s.value("track", std::string()) + " (frames " +
                std::to_string(s.value("first_frame", -1)) + "-" + std::to_string(s.value("last_frame", -1)) + ")");
        staticTotal += static_cast<int>(statics.size());
        sweep::Trace trace;
        const fs::path tracePath = row.value("trace", std::string());
        if (!tracePath.empty())
        {
            std::ifstream in(tracePath);
            const nlohmann::json json = nlohmann::json::parse(in, nullptr, false);
            if (!json.is_discarded())
                trace = sweep::TraceFromJson(json);
        }
        trace.level = name;
        if (row.value("loaded", false))
            loadedLevels.push_back(name);
        if (row.value("loaded", false))
            for (const std::string& missing : sweep::MissingMustHappen(trace, must))
            {
                reasons.push_back("must happen: " + missing);
                ++missingTotal;
            }
        int added = -1, removed = -1;
        if (!baseline.empty() && !tracePath.empty())
        {
            std::ifstream in(baseline / tracePath.filename());
            const nlohmann::json json = nlohmann::json::parse(in, nullptr, false);
            if (!json.is_discarded())
            {
                const sweep::TraceDiff diff = sweep::DiffTraces(trace, sweep::TraceFromJson(json));
                added = static_cast<int>(diff.added.size());
                removed = static_cast<int>(diff.removed.size());
                addedTotal += added;
                removedTotal += removed;
                row["baseline_added"] = diff.added;
                row["baseline_removed"] = diff.removed;
            }
        }
        if (!config.writeBaseline.empty() && !tracePath.empty() && row.value("loaded", false))
        {
            fs::create_directories(config.writeBaseline, error);
            fs::copy_file(tracePath, config.writeBaseline / tracePath.filename(), fs::copy_options::overwrite_existing, error);
        }
        row["known_findings"] = TakeKnown(known, "behaviour", name, reasons, knownUsed);
        knownTotal += static_cast<int>(row["known_findings"].size());
        row["errors"] = log.errors;
        row["sanitizer_report"] = log.sanitizerText;
        row["backtrace"] = log.backtrace;
        row["failure_reasons"] = reasons;
        row["passed"] = reasons.empty();
        passed += reasons.empty() ? 1 : 0;
        std::printf("%-28.28s %-6s %7d %7d %6zu %6zu %6zu %8s %8s  %s\n", name.c_str(), reasons.empty() ? "PASS" : "FAIL",
            row.value("events", 0), row.value("tracks_judged", 0), statics.size(),
            row.value("unresolved_segments", nlohmann::json::array()).size(),
            row.value("stalled_transitions", nlohmann::json::array()).size(),
            added < 0 ? "-" : std::to_string(added).c_str(), removed < 0 ? "-" : std::to_string(removed).c_str(),
            reasons.empty() ? "" : reasons.front().c_str());
        for (size_t i = 1; i < reasons.size() && i < 6; ++i)
            std::printf("%-28s %s\n", "", reasons[i].c_str());
        levels.push_back(row);
    }
    // Must-happen entries nobody evaluated (their level did not run or did
    // not load) fail the run: they would otherwise pass silently.
    const std::vector<std::string> unevaluated = sweep::UnevaluatedMustHappen(must, loadedLevels);
    for (const std::string& line : unevaluated)
        std::printf("behaviour: must happen: %s\n", line.c_str());
    const nlohmann::json stale = ReportStale(known, "behaviour", loadedLevels, knownUsed);
    const int startupSanitizer = run.logs.count("") ? run.logs.at("").sanitizerReports : 0;
    const bool ok = passed == static_cast<int>(names.size()) && startupSanitizer == 0 && unevaluated.empty();
    nlohmann::json report = { { "levels", levels }, { "passed", passed }, { "total", names.size() },
        { "must_happen_unevaluated", unevaluated }, { "stale_known_findings", stale },
        { "frames_per_level", frames }, { "must_happen", config.mustHappen.string() }, { "baseline", baseline.string() },
        { "baseline_written", config.writeBaseline.string() }, { "child_logs", run.childLogs },
        { "static_targets", staticTotal }, { "must_happen_missing", missingTotal },
        { "baseline_added", addedTotal }, { "baseline_removed", removedTotal },
        { "startup_sanitizer_reports", startupSanitizer }, { "freeze_root", config.freezeRoot },
        { "known_findings", knownTotal }, { "known_file", config.known.string() } };
    std::ofstream(config.out / "behaviour.json", std::ios::trunc) << report.dump(2) << "\n";
    std::printf("\nbehaviour: %d/%zu levels passed (%d frames each); static targets %d, must-happen missing %d, "
        "not evaluated %zu; %d known findings; baseline %s (advisory: %d added, %d removed); %s; report %s\n", passed,
        names.size(), frames, staticTotal, missingTotal, unevaluated.size(), knownTotal,
        baseline.empty() ? "none" : baseline.c_str(), addedTotal, removedTotal, ok ? "PASS" : "FAIL",
        (config.out / "behaviour.json").c_str());
    if (!config.writeBaseline.empty())
        std::printf("behaviour: traces kept as the baseline in %s\n", config.writeBaseline.c_str());
    return ok ? 0 : 1;
}

// Checks 3 and 4 on every level: unlocks, swings at what they made
// breakable, clue pickups, then save, reload and compare. Fails on a
// breakable object that conclusive swings did not break, on a live/reload
// mismatch that sweep::ReloadMismatchFails calls a failure, on a failed
// save or reload, and on the usual crash/log/sidebag reasons.
int Unlock(const Config& config)
{
    const std::vector<std::string> names = config.levels.empty() ? sweep_game::LevelNames() : config.levels;
    std::vector<sweep::KnownFinding> known;
    if (!LoadKnown(config, "unlock", known))
        return 1;
    std::set<size_t> knownUsed;
    std::vector<std::string> loadedLevels;
    std::vector<std::string> extra;
    if (config.injectStuck >= 0)
        extra.insert(extra.end(), { "--inject-stuck", std::to_string(config.injectStuck) });
    if (config.injectLoseFlag >= 0)
        extra.insert(extra.end(), { "--inject-lose-flag", std::to_string(config.injectLoseFlag) });
    const LevelRun run = RunLevelChildren(config, "unlock", extra, "unlock", names, 400.0);
    nlohmann::json levels = nlohmann::json::array();
    int passed = 0, unlocks = 0, breakables = 0, mismatches = 0, advisory = 0, knownTotal = 0;
    sweep::SwingCoverage swings;
    // How each lock group opened (see Unlock in sweep_behaviour_child.cpp):
    // "forced" and "failed" mean the player's path was not covered.
    std::map<std::string, int> hows;
    std::printf("\n%-28s %-6s %7s %6s %6s %6s %6s  %s\n", "level", "result", "unlocks", "brk", "clues", "fail",
        "advis", "reasons");
    for (const std::string& name : names)
    {
        nlohmann::json row = run.rows.count(name) ? run.rows.at(name) : nlohmann::json{ { "level", name } };
        const sweep::LevelLog log = run.logs.count(name) ? run.logs.at(name) : sweep::LevelLog{};
        std::vector<std::string> reasons = CommonReasons(row, log);
        for (const std::string& finding : Strings(row.value("breakable_findings", nlohmann::json::array())))
            reasons.push_back("breakable: " + finding);
        const nlohmann::json failing = row.value("reload_mismatches", nlohmann::json::array());
        for (const nlohmann::json& m : failing)
            reasons.push_back("live vs reload: " + m.value("key", std::string()) + " " + m.value("field", std::string()) +
                ": live " + m.value("live", std::string()) + ", reload " + m.value("reload", std::string()) + " (" +
                m.value("story", std::string()) + ")");
        const nlohmann::json reload = row.value("reload", nlohmann::json::object());
        if (row.value("loaded", false) && !(reload.value("saved", false) && reload.value("save_loaded", false) &&
            reload.value("reloaded", false)))
            reasons.push_back("save and reload failed " + reload.dump());
        const int levelUnlocks = static_cast<int>(row.value("unlocks", nlohmann::json::array()).size());
        const int levelBreakables = static_cast<int>(row.value("breakables", nlohmann::json::array()).size());
        for (const nlohmann::json& breakable : row.value("breakables", nlohmann::json::array()))
            sweep::AddSwingCoverage(swings, sweep::BreakableFromJson(breakable));
        unlocks += levelUnlocks;
        breakables += levelBreakables;
        mismatches += static_cast<int>(failing.size());
        advisory += static_cast<int>(row.value("reload_advisory", nlohmann::json::array()).size());
        if (row.value("loaded", false))
            loadedLevels.push_back(name);
        nlohmann::json unlockNotes = nlohmann::json::array();
        for (const nlohmann::json& u : row.value("unlocks", nlohmann::json::array()))
        {
            const std::string how = u.value("how", std::string("?"));
            ++hows[how];
            if (how == "failed" || how == "forced")
                unlockNotes.push_back("lock group " + std::to_string(u.value("lockg", -1)) + " " + how +
                    ": state " + std::to_string(u.value("state_before", -1)) + " -> " +
                    std::to_string(u.value("state_after", -1)) + (u.value("approached", false) ? "" : ", no place to stand beside it"));
        }
        row["unlock_notes"] = unlockNotes;
        row["known_findings"] = TakeKnown(known, "unlock", name, reasons, knownUsed);
        knownTotal += static_cast<int>(row["known_findings"].size());
        row["errors"] = log.errors;
        row["sanitizer_report"] = log.sanitizerText;
        row["backtrace"] = log.backtrace;
        row["failure_reasons"] = reasons;
        row["passed"] = reasons.empty();
        passed += reasons.empty() ? 1 : 0;
        std::printf("%-28.28s %-6s %7d %6d %6d %6zu %6zu  %s\n", name.c_str(), reasons.empty() ? "PASS" : "FAIL",
            levelUnlocks, levelBreakables, row.value("clues_touched", 0), failing.size(),
            row.value("reload_advisory", nlohmann::json::array()).size(), reasons.empty() ? "" : reasons.front().c_str());
        for (size_t i = 1; i < reasons.size() && i < 6; ++i)
            std::printf("%-28s %s\n", "", reasons[i].c_str());
        levels.push_back(row);
    }
    const nlohmann::json stale = ReportStale(known, "unlock", loadedLevels, knownUsed);
    const int startupSanitizer = run.logs.count("") ? run.logs.at("").sanitizerReports : 0;
    const bool ok = passed == static_cast<int>(names.size()) && startupSanitizer == 0;
    std::string howText;
    for (const auto& [how, count] : hows)
        howText += (howText.empty() ? "" : ", ") + how + " " + std::to_string(count);
    nlohmann::json report = { { "levels", levels }, { "passed", passed }, { "total", names.size() },
        { "unlock_outcomes", hows }, { "stale_known_findings", stale }, { "inject_lose_flag", config.injectLoseFlag },
        { "lock_groups", unlocks }, { "breakables_changed", breakables }, { "reload_mismatches", mismatches },
        { "reload_advisory", advisory }, { "child_logs", run.childLogs }, { "inject_stuck", config.injectStuck },
        { "startup_sanitizer_reports", startupSanitizer }, { "known_findings", knownTotal },
        { "known_file", config.known.string() }, { "swing_coverage", sweep::ToJson(swings) } };
    std::ofstream(config.out / "unlock.json", std::ios::trunc) << report.dump(2) << "\n";
    std::printf("\nunlock: %d/%zu passed; %d lock groups (%s), %d responses changed, %d failing (%d of them known) "
        "and %d advisory live/reload differences; report %s\n", passed, names.size(), unlocks,
        howText.empty() ? "none" : howText.c_str(), breakables, mismatches, knownTotal, advisory,
        (config.out / "unlock.json").c_str());
    // Coverage of the breakable check: only a judged object could fail it.
    std::printf("unlock: swings: %d of %d objects that became breakable judged (%d broke, %d swung at), "
        "%d conclusive swings\n", swings.judged, swings.becameBreakable, swings.broke, swings.swungAt,
        swings.conclusiveSwings);
    return ok ? 0 : 1;
}

// The user's fuse sequence (sweep_behaviour_child.cpp, ScenarioFuse).
int Fuse(const Config& config)
{
    std::vector<sweep::KnownFinding> known;
    if (!LoadKnown(config, "fuse", known))
        return 1;
    const fs::path result = config.out / "fuse.json";
    const fs::path log = config.out / "logs" / "fuse.log";
    std::error_code error;
    fs::remove(log, error);
    fs::remove(result, error);
    std::vector<std::string> args = { "unlock", "--scenario", "fuse" };
    fs::remove_all(config.out / "user-fuse", error);
    const std::vector<std::string> common = Common(config, config.out / "user-fuse");
    args.insert(args.end(), common.begin(), common.end());
    args.insert(args.end(), { "--results", result.string() });
    if (config.injectStuck >= 0)
        args.insert(args.end(), { "--inject-stuck", std::to_string(config.injectStuck) });
    if (config.injectLoseFlag >= 0)
        args.insert(args.end(), { "--inject-lose-flag", std::to_string(config.injectLoseFlag) });
    const ChildResult status = RunChild(config, args, log, 600.0);
    nlohmann::json report = ReadJson(result);
    if (!report.is_object())
        report = nlohmann::json::object();
    const ChildLog childLog = ReadChildLog(log);
    std::vector<std::string> reasons;
    if (!(status.exited && status.exitCode == 0))
        reasons.push_back("child " + Describe(status));
    if (report.contains("error"))
        reasons.push_back(report["error"].get<std::string>());
    if (!report.value("arrived", false))
        reasons.push_back("never arrived in Prowling the Grounds");
    if (!report.value("fuse_found", false) || !report.value("breaker_found", false))
        reasons.push_back("fuse or breaker not found");
    for (const std::string& finding : Strings(report.value("findings", nlohmann::json::array())))
        reasons.push_back(finding);
    if (!report.value("reload", nlohmann::json::object()).value("reloaded", false))
        reasons.push_back("the level was not reloaded");
    if (report.value("sidebag_rebuild_list", nlohmann::json::array()).size() != 0)
        reasons.push_back("valid sidebags replaced");
    AddLogReasons(childLog, reasons);
    std::set<size_t> knownUsed;
    report["known_findings"] = TakeKnown(known, "fuse", "Prowling the Grounds", reasons, knownUsed);
    report["stale_known_findings"] = ReportStale(known, "fuse", { "Prowling the Grounds" }, knownUsed);
    report["child"] = Describe(status);
    report["log"] = log.string();
    report["failure_reasons"] = reasons;
    report["passed"] = reasons.empty();
    std::ofstream(result, std::ios::trunc) << report.dump(2) << "\n";
    const nlohmann::json fuse = report.value("fuse", nlohmann::json::object());
    std::printf("fuse: %s; steps %s; breaker unlock %s; fuse response %d -> %d; broke %s after %d swings; "
        "fuse after reload %s; %s\n", Describe(status).c_str(), report.value("steps", nlohmann::json::array()).dump().c_str(),
        report.value("unlock", nlohmann::json::object()).value("how", std::string("?")).c_str(),
        fuse.value("sweep_before", -1), fuse.value("sweep_after", -1), fuse.value("broke", false) ? "yes" : "no",
        fuse.value("swings", 0),
        report.value("reload", nlohmann::json::object()).value("fuse_in_world", true) ? "PRESENT" : "gone",
        reasons.empty() ? "PASS" : "FAIL");
    for (const std::string& reason : reasons)
        std::printf("  %s\n", reason.c_str());
    return reasons.empty() ? 0 : 1;
}

int Compare(int argc, char** argv)
{
    if (argc < 4)
    {
        std::fprintf(stderr, "usage: cane_world_sweep compare <output-dir> <reference-dir> [--tolerance N] [--max-fraction F]\n");
        return 2;
    }
    int tolerance = 8;
    double maxFraction = 0.001;
    for (int i = 4; i < argc; ++i)
    {
        const std::string key = argv[i];
        if ((key != "--tolerance" && key != "--max-fraction") || i + 1 >= argc)
        {
            std::fprintf(stderr, "compare: %s %s\n", key.c_str(),
                key == "--tolerance" || key == "--max-fraction" ? "needs a value" : "is not an option");
            return 2;
        }
        if (key == "--tolerance") tolerance = std::atoi(argv[++i]);
        else maxFraction = std::atof(argv[++i]);
    }
    const sweep::FolderCompare result = sweep::CompareFolders(argv[2], argv[3], tolerance, maxFraction);
    for (const std::string& line : result.changed) std::printf("changed  %s\n", line.c_str());
    for (const std::string& line : result.missing) std::printf("missing  %s\n", line.c_str());
    for (const std::string& line : result.added) std::printf("new      %s\n", line.c_str());
    std::printf("compare: %d compared, %zu changed, %zu missing, %zu new (tolerance %d, max fraction %g): %s\n",
        result.compared, result.changed.size(), result.missing.size(), result.added.size(), tolerance, maxFraction,
        result.Passed() ? "PASS" : "FAIL");
    return result.Passed() ? 0 : 1;
}

// Every check by name; "all" runs them in this order.
struct NamedCheck
{
    const char* name;
    int (*run)(const Config&);
};
constexpr NamedCheck kChecks[] = {
    { "levels", Levels },
    { "cutscenes", Cutscenes },
    { "audio", Audio },
    { "saves", Saves },
    { "determinism", Determinism },
    { "display", [](const Config& config) { return Display(config, false); } },
    { "display-fullscreen", [](const Config& config) { return Display(config, true); } },
    { "van-door", VanDoor },
    { "hub-tour", HubTour },
    { "charm-exit", CharmExit },
    { "behaviour", Behaviour },
    { "unlock", Unlock },
    { "fuse", Fuse },
};

void PrintUsage()
{
    std::string names;
    for (const NamedCheck& check : kChecks)
        names += std::string(check.name) + "|";
    std::fprintf(stderr, "usage: cane_world_sweep <%sall> [options]\n"
        "       cane_world_sweep compare <output-dir> <reference-dir> [--tolerance N] [--max-fraction F]\n"
        "       cane_world_sweep units\n"
        "(options: see the top of tests/world_sweep/sweep_main.cpp)\n", names.c_str());
}

// Reads `--key value` pairs into `config`. False (and a message) on an
// unknown option, an option without its value, or a stray argument.
bool ParseOptions(const std::vector<std::string>& args, Config& config)
{
    for (size_t i = 1; i < args.size(); i += 2)
    {
        const std::string& key = args[i];
        if (key.rfind("--", 0) != 0)
        {
            std::fprintf(stderr, "cane_world_sweep: unexpected argument '%s'\n", key.c_str());
            return false;
        }
        static const std::set<std::string> known = { "--data-dir", "--out", "--levels", "--frames", "--fmv-seconds",
            "--jobs", "--level", "--reference", "--tolerance", "--max-fraction", "--baseline", "--write-baseline",
            "--must-happen", "--freeze-root", "--inject-stuck", "--inject-lose-flag", "--known" };
        if (known.count(key) == 0)
        {
            std::fprintf(stderr, "cane_world_sweep: unknown option %s\n", key.c_str());
            return false;
        }
        if (i + 1 >= args.size())
        {
            std::fprintf(stderr, "cane_world_sweep: %s needs a value\n", key.c_str());
            return false;
        }
        const std::string& value = args[i + 1];
        if (key == "--data-dir") config.data = value;
        else if (key == "--out") config.out = value;
        else if (key == "--levels") config.levels = Split(value, ';');
        else if (key == "--frames") { config.frames = std::atoi(value.c_str()); config.framesGiven = true; }
        else if (key == "--fmv-seconds") config.fmvSeconds = std::atof(value.c_str());
        else if (key == "--jobs") config.jobs = (std::max)(1, std::atoi(value.c_str()));
        else if (key == "--level") config.level = value;
        else if (key == "--reference") config.reference = value;
        else if (key == "--tolerance") config.tolerance = std::atoi(value.c_str());
        else if (key == "--max-fraction") config.maxFraction = std::atof(value.c_str());
        else if (key == "--baseline") config.baseline = value;
        else if (key == "--write-baseline") config.writeBaseline = value;
        else if (key == "--must-happen") config.mustHappen = value;
        else if (key == "--freeze-root") config.freezeRoot = std::atoi(value.c_str());
        else if (key == "--inject-stuck") config.injectStuck = std::atoi(value.c_str());
        else if (key == "--inject-lose-flag") config.injectLoseFlag = std::atoi(value.c_str());
        else if (key == "--known") config.known = value;
    }
    return true;
}
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    std::vector<std::string> args(argv + 1, argv + argc);
    if (!args.empty() && args[0] == "child")
        return sweep_game::RunChild(args);
    if (!args.empty() && args[0] == "compare")
        return Compare(argc, argv);
    // No data, display or Startup needed.
    if (!args.empty() && args[0] == "units")
        return sweep_game::RunUnits();
    const std::string command = args.empty() ? std::string() : args[0];
    const NamedCheck* named = nullptr;
    for (const NamedCheck& check : kChecks)
        if (command == check.name)
            named = &check;
    if (named == nullptr && command != "all")
    {
        if (!command.empty())
            std::fprintf(stderr, "cane_world_sweep: unknown command %s\n", command.c_str());
        PrintUsage();
        return 2;
    }

    Config config;
    config.self = cane::ExecutableDirectory() / "cane_world_sweep";
    // An empty variable counts as not set.
    if (const char* data = std::getenv("CANE_TEST_DATA_DIR"); data != nullptr && *data != '\0')
        config.data = data;
    // Beside bin/, in the build folder (CTest's --out is the same folder).
    const char* outVariable = std::getenv("CANE_SWEEP_OUT");
    config.out = outVariable != nullptr && *outVariable != '\0' ? fs::path(outVariable)
                                                                : cane::ExecutableDirectory().parent_path() / "world-sweep";
    if (!ParseOptions(args, config))
    {
        PrintUsage();
        return 2;
    }
    // No data folder given: skip. One given that holds no game data is a
    // mistake in the setup, and must not pass as a skip.
    if (config.data.empty())
    {
        PrintHowToRun();
        return kSkip;
    }
    if (!ValidData(config.data))
    {
        std::printf("FAIL: the game data folder '%s' (--data-dir or CANE_TEST_DATA_DIR) has no "
            "Worlds/Splash/Splash.brx. Give the folder that holds the extracted Worlds/ and Cutscenes/.\n",
            config.data.c_str());
        return 1;
    }
    config.data = fs::absolute(config.data);
    config.out = fs::absolute(config.out);
    fs::create_directories(config.out / "logs");

    if (named != nullptr)
        return named->run(config);
    int failures = 0;
    for (const NamedCheck& check : kChecks)
        failures += check.run(config) != 0 ? 1 : 0;
    return failures == 0 ? 0 : 1;
}
