// Data-free checks for the world sweep's helpers: image compare, transform
// scan, frame hashing and divergence search, log attribution and the report
// rows. These run without game data, a display or the game objects.

#include "sweep_core.h"
#include "sweep_behaviour.h"

#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cmath>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <set>
#include <string>
#include <vector>

extern char** environ;

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

sweep::Image Solid(int width, int height, uint8_t value)
{
    sweep::Image image;
    image.width = width;
    image.height = height;
    image.rgba.assign(static_cast<size_t>(width) * height * 4, value);
    return image;
}

void TestImageCompare()
{
    const sweep::Image a = Solid(4, 4, 100);
    sweep::Image b = a;
    sweep::ImageDiff same = sweep::CompareImages(a, b, 0);
    CHECK(!same.sizeMismatch);
    CHECK(same.totalPixels == 16);
    CHECK(same.changedPixels == 0);
    CHECK(!sweep::ImageChanged(same, 0.0));

    // One pixel off by 5 in one channel: within tolerance 5, outside 4.
    b.rgba[4 * 5 + 1] = 105;
    CHECK(sweep::CompareImages(a, b, 5).changedPixels == 0);
    sweep::ImageDiff one = sweep::CompareImages(a, b, 4);
    CHECK(one.changedPixels == 1);
    CHECK(one.maxChannelDelta == 5);
    CHECK(std::fabs(one.ChangedFraction() - 1.0 / 16.0) < 1e-12);
    CHECK(sweep::ImageChanged(one, 0.05));
    CHECK(!sweep::ImageChanged(one, 0.07));

    // Alpha is ignored: the scene target's alpha is not part of the picture.
    sweep::Image c = a;
    c.rgba[3] = 0;
    CHECK(sweep::CompareImages(a, c, 0).changedPixels == 0);

    const sweep::ImageDiff sized = sweep::CompareImages(a, Solid(4, 5, 100), 0);
    CHECK(sized.sizeMismatch);
    CHECK(sweep::ImageChanged(sized, 1.0));
}

void TestImageStats()
{
    sweep::Image black = Solid(2, 2, 0);
    CHECK(sweep::MeanLuma(black) == 0.0);
    sweep::Image white = Solid(2, 2, 255);
    CHECK(std::fabs(sweep::MeanLuma(white) - 255.0) < 1e-9);
}

void TestPngRoundTrip()
{
    const std::filesystem::path dir = std::filesystem::temp_directory_path() / "cane_sweep_selftest";
    std::filesystem::create_directories(dir);
    sweep::Image image = Solid(3, 2, 0);
    for (size_t i = 0; i < image.rgba.size(); ++i)
        image.rgba[i] = static_cast<uint8_t>(i * 7);
    for (size_t i = 3; i < image.rgba.size(); i += 4)
        image.rgba[i] = 255;
    const std::filesystem::path path = dir / "roundtrip.png";
    CHECK(sweep::WritePng(path, image));
    sweep::Image read;
    CHECK(sweep::ReadPng(path, read));
    CHECK(read.width == 3 && read.height == 2);
    CHECK(read.rgba == image.rgba);

    // Rows from glReadPixels are bottom-up; FlipRows makes them top-down.
    sweep::Image rows = Solid(1, 2, 0);
    rows.rgba[0] = 1;
    rows.rgba[4] = 2;
    sweep::FlipRows(rows);
    CHECK(rows.rgba[0] == 2 && rows.rgba[4] == 1);
    std::filesystem::remove_all(dir);
}

void TestCompareFolders()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cane_sweep_selftest_cmp";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "out" / "Level A");
    std::filesystem::create_directories(root / "ref" / "Level A");
    const sweep::Image base = Solid(4, 4, 50);
    sweep::Image changed = base;
    for (size_t i = 0; i < 16; ++i)
        changed.rgba[i * 4] = 200;
    CHECK(sweep::WritePng(root / "out" / "Level A" / "same.png", base));
    CHECK(sweep::WritePng(root / "ref" / "Level A" / "same.png", base));
    CHECK(sweep::WritePng(root / "out" / "Level A" / "changed.png", changed));
    CHECK(sweep::WritePng(root / "ref" / "Level A" / "changed.png", base));
    CHECK(sweep::WritePng(root / "ref" / "Level A" / "missing.png", base));
    CHECK(sweep::WritePng(root / "out" / "Level A" / "new.png", base));

    const sweep::FolderCompare result = sweep::CompareFolders(root / "out", root / "ref", 8, 0.001);
    CHECK(result.compared == 2);
    CHECK(result.changed.size() == 1 && result.changed[0].find("changed.png") != std::string::npos);
    CHECK(result.missing.size() == 1 && result.missing[0].find("missing.png") != std::string::npos);
    CHECK(result.added.size() == 1 && result.added[0].find("new.png") != std::string::npos);
    CHECK(!result.Passed());
    std::filesystem::remove_all(root);
}

void TestTransformScan()
{
    sweep::TransformScan scan;
    const float good[] = { 1.0f, -2.0f, 3.0f };
    sweep::ScanValues(scan, good, 3, 1.0e6f);
    CHECK(scan.nonFinite == 0 && scan.absurd == 0);
    CHECK(scan.maxAbs == 3.0f);

    const float bad[] = { std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(), 5.0e6f, -5.0e6f };
    sweep::ScanValues(scan, bad, 4, 1.0e6f);
    CHECK(scan.nonFinite == 2);
    CHECK(scan.absurd == 2);
    CHECK(scan.maxAbs == 5.0e6f);
}

float Bits(uint32_t bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void TestParkedTransforms()
{
    const float kMax = Bits(0x7f7fffffu);
    const float kMin = Bits(0xff7fffffu);
    CHECK(kMax == std::numeric_limits<float>::max());
    CHECK(kMin == -std::numeric_limits<float>::max());
    auto world = [](float z) { std::vector<float> v(12, 0.5f); v[2] = z; return v; };

    sweep::TransformChecker checker(1.0e7f);
    // Object 1: parked at +FLT_MAX from frame 0, and still there later.
    std::vector<float> parked = world(kMax);
    sweep::TransformScan s0 = checker.Check(1, 0, parked.data(), parked.size());
    CHECK(s0.parked == 1 && s0.absurd == 0 && s0.nonFinite == 0);
    CHECK(s0.maxAbs == 0.5f);
    sweep::TransformScan s1 = checker.Check(1, 1, parked.data(), parked.size());
    CHECK(s1.parked == 1 && s1.absurd == 0);
    // -FLT_MAX from frame 0 parks as well.
    std::vector<float> parkedLow = world(kMin);
    CHECK(checker.Check(2, 0, parkedLow.data(), parkedLow.size()).parked == 1);

    // Object 3: finite at frame 0, FLT_MAX later: absurd, not parked.
    std::vector<float> fine = world(10.0f);
    CHECK(checker.Check(3, 0, fine.data(), fine.size()).absurd == 0);
    sweep::TransformScan late = checker.Check(3, 5, parked.data(), parked.size());
    CHECK(late.parked == 0 && late.absurd == 1);

    // Object 4 first appears mid-run already at FLT_MAX: absurd.
    sweep::TransformScan spawned = checker.Check(4, 7, parked.data(), parked.size());
    CHECK(spawned.parked == 0 && spawned.absurd == 1);

    // NaN, Inf and 1e30 always fail, at frame 0 or later.
    sweep::TransformChecker fresh(1.0e7f);
    std::vector<float> nan = world(std::numeric_limits<float>::quiet_NaN());
    std::vector<float> inf = world(std::numeric_limits<float>::infinity());
    std::vector<float> big = world(1.0e30f);
    CHECK(fresh.Check(10, 0, nan.data(), nan.size()).nonFinite == 1);
    CHECK(fresh.Check(11, 0, inf.data(), inf.size()).nonFinite == 1);
    sweep::TransformScan bigScan = fresh.Check(12, 0, big.data(), big.size());
    CHECK(bigScan.absurd == 1 && bigScan.parked == 0);
    CHECK(fresh.Check(10, 3, nan.data(), nan.size()).nonFinite == 1);

    // A parked component only counts while the rest of the transform is
    // finite: with a NaN beside it, both fail.
    std::vector<float> mixed = world(kMax);
    sweep::TransformChecker third(1.0e7f);
    CHECK(third.Check(20, 0, mixed.data(), mixed.size()).parked == 1);
    mixed[5] = std::numeric_limits<float>::quiet_NaN();
    sweep::TransformScan broken = third.Check(20, 1, mixed.data(), mixed.size());
    CHECK(broken.nonFinite == 1 && broken.absurd == 1 && broken.parked == 0);
    // Parked from frame 0 requires the rest finite at frame 0 too.
    sweep::TransformChecker fourth(1.0e7f);
    CHECK(fourth.Check(30, 0, mixed.data(), mixed.size()).parked == 0);
    std::vector<float> healed = world(kMax);
    CHECK(fourth.Check(30, 1, healed.data(), healed.size()).absurd == 1);
}

void TestHashAndDivergence()
{
    const float x[] = { 1.0f, 2.0f };
    const float y[] = { 1.0f, 2.0000002f };
    CHECK(sweep::Fnv1a(x, sizeof(x)) == sweep::Fnv1a(x, sizeof(x)));
    CHECK(sweep::Fnv1a(x, sizeof(x)) != sweep::Fnv1a(y, sizeof(y)));

    // frames x objects
    sweep::FrameHashes a = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
    sweep::FrameHashes b = a;
    sweep::Divergence none = sweep::FirstDivergence(a, b);
    CHECK(!none.diverged);
    CHECK(none.framesCompared == 3);

    b[1][2] = 60;
    b[2][0] = 70;
    sweep::Divergence d = sweep::FirstDivergence(a, b);
    CHECK(d.diverged);
    CHECK(d.frame == 1 && d.object == 2);

    // An object-count mismatch at a frame is a divergence at that frame.
    sweep::FrameHashes c = a;
    c[2].push_back(10);
    sweep::Divergence counts = sweep::FirstDivergence(a, c);
    CHECK(counts.diverged && counts.frame == 2 && counts.objectCountMismatch);

    // A shorter run diverges where it ends.
    sweep::FrameHashes shorter = { { 1, 2, 3 } };
    sweep::Divergence len = sweep::FirstDivergence(a, shorter);
    CHECK(len.diverged && len.frame == 1);

    // Binary round trip of the per-frame hash file.
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "cane_sweep_hashes.bin";
    CHECK(sweep::WriteFrameHashes(path, a));
    sweep::FrameHashes back;
    CHECK(sweep::ReadFrameHashes(path, back));
    CHECK(back == a);
    std::filesystem::remove(path);
}

void TestLogSplit()
{
    const std::string log =
        "Content package: x\n"
        "[12:00:00.000] [ERROR] [Graphics] before any level\n"
        "=== LEVEL Splash ===\n"
        "World Loaded Successfully\n"
        "=== LEVEL The Hideout ===\n"
        "[12:00:01.000] [ERROR] [Load] one\n"
        "[12:00:02.000] [ERROR] [Music] two\n"
        "UNSUPPORTED (log once): POSSIBLE ISSUE: unit 0 GLD_TEXTURE_INDEX_BUFFER is unloadable\n"
        "[sampler-audit] glDrawArrays: program 13 uniform 'edgeTex' unit 12 has no buffer texture\n"
        "=== LEVEL Paris, France ===\n"
        "=== SIGNAL 11 ===\n"
        "0 cane_world_sweep 0x1 frame0\n"
        "1 cane_world_sweep 0x2 frame1\n";
    const std::map<std::string, sweep::LevelLog> split = sweep::SplitLevelLog(log);
    CHECK(split.count("") == 1 && split.at("").errorLines == 1);
    CHECK(split.count("Splash") == 1 && split.at("Splash").errorLines == 0);
    CHECK(split.at("The Hideout").errorLines == 2);
    CHECK(split.at("The Hideout").errors.size() == 2);
    CHECK(split.at("The Hideout").driverWarnings == 1);
    CHECK(split.at("The Hideout").samplerAudit == 1);
    CHECK(split.at("Splash").driverWarnings == 0 && split.at("Splash").samplerAudit == 0);
    CHECK(split.at("Paris, France").signal == 11);
    CHECK(split.at("Paris, France").backtrace.find("frame1") != std::string::npos);
    CHECK(split.at("Splash").signal == 0);
    CHECK(sweep::CountErrorLines(log) == 3);
}

void TestSanitizerReports()
{
    // UBSan (halt_on_error=0) keeps running after each report; ASan with
    // -fsanitize-recover=address and halt_on_error=0 does too. Each report has
    // one headline line; its stack and SUMMARY line belong to it.
    const std::string log =
        "=== LEVEL Splash ===\n"
        "World Loaded Successfully\n"
        "=== LEVEL The Hideout ===\n"
        "/src/Sly1-Release/sw.cpp:812:14: runtime error: load of misaligned address 0x000102 for type 'int'\n"
        "    #0 0x1000 in LoadSwFromBrx(SW*, CBinaryInputStream*) sw.cpp:812\n"
        "SUMMARY: UndefinedBehaviorSanitizer: misaligned-pointer-use /src/Sly1-Release/sw.cpp:812:14\n"
        "=================================================================\n"
        "==4242==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6020 at pc 0x1 bp 0x2 sp 0x3\n"
        "READ of size 4 at 0x6020 thread T0\n"
        "    #0 0x2000 in ReadThing thing.cpp:7\n"
        "SUMMARY: AddressSanitizer: heap-buffer-overflow thing.cpp:7 in ReadThing\n"
        "=== LEVEL Paris, France ===\n"
        "==4243==ERROR: UndefinedBehaviorSanitizer: SEGV on unknown address 0x000000000000\n"
        "Not a report: the word runtime error without the colon form\n";
    const std::map<std::string, sweep::LevelLog> split = sweep::SplitLevelLog(log);
    CHECK(split.at("Splash").sanitizerReports == 0);
    CHECK(split.at("The Hideout").sanitizerReports == 2);
    CHECK(split.at("Paris, France").sanitizerReports == 1);
    // The whole report text stays with its level, stack included.
    const std::string& hideout = split.at("The Hideout").sanitizerText;
    CHECK(hideout.find("sw.cpp:812:14: runtime error") != std::string::npos);
    CHECK(hideout.find("in LoadSwFromBrx") != std::string::npos);
    CHECK(hideout.find("SUMMARY: AddressSanitizer: heap-buffer-overflow") != std::string::npos);
    CHECK(hideout.find("World Loaded") == std::string::npos);
    CHECK(split.at("Splash").sanitizerText.empty());
    // Sanitizer reports are not [ERROR] lines, but they are counted for the
    // checks that read a whole log.
    CHECK(split.at("The Hideout").errorLines == 0);
    CHECK(sweep::CountSanitizerReports(log) == 3);
    CHECK(sweep::CountSanitizerReports("[ERROR] nothing else\n") == 0);

    sweep::LevelResult r;
    r.level = "The Hideout";
    r.loaded = true;
    r.frames = sweep::kRequiredFrames;
    CHECK(sweep::Passed(r));
    r.sanitizerReports = 2;
    CHECK(!sweep::Passed(r));
    CHECK(sweep::ToTsv(r).find("2 sanitizer reports") != std::string::npos);
    CHECK(sweep::FromJson(sweep::ToJson(r)).sanitizerReports == 2);
}

void TestChildTimers()
{
    // Awake time never runs backwards and, outside system sleep, matches the
    // total time closely.
    const double a0 = sweep::AwakeSeconds();
    const double a1 = sweep::AwakeSeconds();
    CHECK(a1 >= a0 && a0 > 0.0);
    const sweep::SleepAwareTimer timer;
    CHECK(timer.Awake() >= 0.0 && timer.Awake() < 5.0);
    CHECK(timer.Slept() >= 0.0 && timer.Slept() < 1.0);

    // A run that spanned a system sleep says so; short gaps are noise.
    CHECK(sweep::SleepNote(0.0).empty());
    CHECK(sweep::SleepNote(4.9).empty());
    CHECK(sweep::SleepNote(900.0).find("system sleep") != std::string::npos);
    CHECK(sweep::SleepNote(900.0).find("900") != std::string::npos);

    // The cutscene child's own watchdog: the cap when given, else 1.5 x the
    // movie plus 60 s, none when neither is known.
    CHECK(sweep::CutsceneWatchdogSeconds(430.0, 0.0) == 705.0);
    CHECK(std::fabs(sweep::CutsceneWatchdogSeconds(13.6, 0.0) - 80.4) < 1e-9);
    CHECK(sweep::CutsceneWatchdogSeconds(100.0, 30.0) == 30.0);
    CHECK(sweep::CutsceneWatchdogSeconds(0.0, 0.0) == 0.0);
}

// Whether /usr/bin/sample can sample `pid` here at all (a sandbox, or a
// missing permission to examine other processes, denies it): runs it
// directly and looks for its report.
bool SampleWorks(pid_t pid)
{
    const std::filesystem::path report = std::filesystem::temp_directory_path() / "cane_sweep_sample_probe.txt";
    std::error_code error;
    std::filesystem::remove(report, error);
    std::string tool = "/usr/bin/sample", pidText = std::to_string(pid), seconds = "1", flag = "-file";
    std::string reportText = report.string();
    char* argv[] = { tool.data(), pidText.data(), seconds.data(), flag.data(), reportText.data(), nullptr };
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
    pid_t sampler = -1;
    const bool spawned = posix_spawn(&sampler, argv[0], &actions, nullptr, argv, environ) == 0;
    posix_spawn_file_actions_destroy(&actions);
    if (!spawned || sampler <= 0)
        return false;
    int status = 0;
    waitpid(sampler, &status, 0);
    const bool works = std::filesystem::file_size(report, error) > 0 && !error;
    std::filesystem::remove(report, error);
    return works;
}

void TestSampleProcess()
{
    // A stuck child is sampled before it is killed: the stack goes to its log.
    pid_t pid = -1;
    char sleepPath[] = "/bin/sleep";
    char seconds[] = "30";
    char* argv[] = { sleepPath, seconds, nullptr };
    const bool spawned = posix_spawn(&pid, sleepPath, nullptr, nullptr, argv, environ) == 0;
    CHECK(spawned);
    // Never fall through to kill(-1, ...), which signals every process the user owns.
    if (!spawned || pid <= 0)
        return;
    auto stop = [pid]
    {
        kill(pid, SIGKILL);
        int status = 0;
        waitpid(pid, &status, 0);
    };
    const std::filesystem::path log = std::filesystem::temp_directory_path() / "cane_sweep_sample.log";
    std::filesystem::remove(log);
    std::FILE* f = std::fopen(log.c_str(), "w");
    CHECK(f != nullptr);
    if (f == nullptr)
    {
        stop();
        return;
    }
    std::fputs("child output\n", f);
    std::fclose(f);
    const bool sampled = sweep::SampleProcessToLog(pid, 1, log);
    // Where sample itself is denied this check cannot run: a skip, not a
    // failure (the sweep's children then lose only their stacks).
    if (!sampled && !SampleWorks(pid))
    {
        std::printf("skipped: /usr/bin/sample cannot sample a process here (denied); stuck-child sampling not checked\n");
        stop();
        std::filesystem::remove(log);
        return;
    }
    CHECK(sampled);
    stop();
    const std::string text = sweep::ReadTextFile(log);
    CHECK(text.rfind("child output\n", 0) == 0);
    CHECK(text.find("=== SAMPLE pid " + std::to_string(pid)) != std::string::npos);
    CHECK(text.find("Analysis of sampling") != std::string::npos);
    std::filesystem::remove(log);
}

void TestFrameStats()
{
    sweep::FrameStats stats;
    CHECK(stats.AvgMs() == 0.0);
    stats.Add(10.0, 4);
    stats.Add(30.0, 9);
    stats.Add(20.0, 12);
    CHECK(stats.frames == 3);
    CHECK(stats.AvgMs() == 20.0);
    CHECK(stats.maxMs == 30.0);
    CHECK(stats.maxFrame == 9);
}

void TestLevelResultRows()
{
    sweep::LevelResult r;
    r.level = "Paris, France";
    r.loaded = true;
    r.loadMs = 1234.5;
    r.frames = 300;
    r.frameStats.Add(4.0, 0);
    r.frameStats.Add(6.0, 1);
    r.outOfWorldBad = 12;
    r.parked = 18;
    r.views = 4;
    CHECK(sweep::Passed(r));
    CHECK(sweep::FailureReasons(r).empty());

    const std::string header = sweep::TsvHeader();
    const std::string row = sweep::ToTsv(r);
    // Same column count, tab separated, level first.
    auto columns = [](const std::string& s) { size_t n = 1; for (char ch : s) n += ch == '\t'; return n; };
    CHECK(columns(header) == columns(row));
    CHECK(row.rfind("Paris, France\t", 0) == 0);
    CHECK(row.find("\tPASS") != std::string::npos);

    const sweep::LevelResult back = sweep::FromJson(sweep::ToJson(r));
    CHECK(back.level == r.level && back.frames == 300 && back.views == 4);
    CHECK(back.frameStats.maxMs == 6.0 && back.frameStats.maxFrame == 1 && back.loaded);
    CHECK(back.outOfWorldBad == 12);
    CHECK(back.parked == 18);
    // Bad values on objects outside the world and parked components are
    // reported, not failed.
    CHECK(sweep::Passed(r));

    sweep::LevelResult f = r;
    f.glErrors = 2;
    CHECK(!sweep::Passed(f));
    f = r;
    f.loggedErrors = 1;
    CHECK(!sweep::Passed(f));
    f = r;
    f.nonFinite = 1;
    CHECK(!sweep::Passed(f));
    f = r;
    f.absurd = 1;
    CHECK(!sweep::Passed(f));
    f = r;
    f.driverWarnings = 1;
    CHECK(!sweep::Passed(f));
    f = r;
    f.samplerAudit = 2;
    CHECK(!sweep::Passed(f));
    // A valid sidebag replaced by EnsureBasicSidebag (custom options lost).
    f = r;
    f.sidebagRebuilds = 3;
    CHECK(!sweep::Passed(f));
    CHECK(sweep::ToTsv(f).find("3 valid sidebags replaced") != std::string::npos);
    CHECK(sweep::FromJson(sweep::ToJson(f)).sidebagRebuilds == 3);
    CHECK(columns(header) == columns(sweep::ToTsv(f)));
    f = r;
    f.signal = 11;
    CHECK(!sweep::Passed(f));
    CHECK(sweep::ToTsv(f).find("\tFAIL") != std::string::npos);
    f = r;
    f.loaded = false;
    CHECK(!sweep::Passed(f));
    f = r;
    f.frames = 299;
    CHECK(!sweep::Passed(f));
    // Fewer than the minimum views is reported but is not a failure on its own.
    f = r;
    f.views = 1;
    CHECK(sweep::Passed(f));
}

// --- behaviour checks (sweep_behaviour.h) ---------------------------------------

sweep::ActSample Sample(float t, float goalX, float posX, bool drives = true)
{
    sweep::ActSample s;
    s.tLocal = t;
    s.goalPos = { goalX, 0.0f, 0.0f };
    s.pos = { posX, 0.0f, 0.0f };
    s.goalRot = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    s.rot = s.goalRot;
    s.hasPosCurve = true;
    s.drivesPos = drives;
    s.drivesRot = drives;
    return s;
}

// How a child ended, and the row the runner writes for a level a child
// stopped in: a timeout is reported as a timeout, and an exit code is never
// stored as a signal.
void TestChildOutcome()
{
    auto has = [](const std::vector<std::string>& reasons, const std::string& text)
    {
        for (const std::string& reason : reasons)
            if (reason.find(text) != std::string::npos)
                return true;
        return false;
    };

    const sweep::ChildOutcome clean = sweep::OutcomeFromWaitStatus(W_EXITCODE(0, 0));
    CHECK(clean.exited && clean.exitCode == 0 && clean.signal == 0 && !clean.timedOut);
    CHECK(sweep::Describe(clean) == "exit 0");
    // The crash handler exits with 128 + signal.
    const sweep::ChildOutcome handled = sweep::OutcomeFromWaitStatus(W_EXITCODE(128 + SIGSEGV, 0));
    CHECK(handled.signal == SIGSEGV && sweep::Describe(handled) == "signal 11");
    const sweep::ChildOutcome killed = sweep::OutcomeFromWaitStatus(W_EXITCODE(0, SIGBUS));
    CHECK(!killed.exited && killed.signal == SIGBUS);
    const sweep::ChildOutcome exit3 = sweep::OutcomeFromWaitStatus(W_EXITCODE(3, 0));
    CHECK(exit3.exited && exit3.exitCode == 3 && exit3.signal == 0);

    // Timed out: the runner's SIGKILL is not a crash.
    const sweep::ChildOutcome hung = sweep::TimedOutOutcome();
    CHECK(hung.timedOut && hung.signal == 0);
    CHECK(sweep::Describe(hung) == "timed out");
    const sweep::LevelResult hungRow = sweep::StoppedLevelRow("Paris, France", hung);
    const std::vector<std::string> hungReasons = sweep::FailureReasons(hungRow);
    CHECK(hungRow.timedOut && hungRow.signal == 0 && hungRow.exitCode == 0);
    CHECK(has(hungReasons, "timed out"));
    CHECK(!has(hungReasons, "signal"));
    CHECK(!has(hungReasons, "did not load"));
    CHECK(sweep::ToTsv(hungRow).find("signal 9") == std::string::npos);
    CHECK(sweep::FromJson(sweep::ToJson(hungRow)).timedOut);
    CHECK(!sweep::Passed(hungRow));

    // An exit code stays an exit code.
    const sweep::LevelResult exitRow = sweep::StoppedLevelRow("Paris, France", exit3);
    CHECK(exitRow.signal == 0 && exitRow.exitCode == 3 && !exitRow.timedOut);
    CHECK(has(sweep::FailureReasons(exitRow), "exited with code 3"));
    CHECK(!has(sweep::FailureReasons(exitRow), "signal"));
    CHECK(sweep::FromJson(sweep::ToJson(exitRow)).exitCode == 3);

    // A crash is a crash.
    const sweep::LevelResult crashRow = sweep::StoppedLevelRow("Paris, France", handled);
    CHECK(crashRow.signal == SIGSEGV && crashRow.exitCode == 0 && !crashRow.timedOut);
    CHECK(has(sweep::FailureReasons(crashRow), "crashed with signal 11"));
    CHECK(!has(sweep::FailureReasons(crashRow), "did not load"));

    // The TSV keeps one column per value.
    auto columns = [](const std::string& s) { size_t n = 1; for (char ch : s) n += ch == '\t'; return n; };
    CHECK(columns(sweep::TsvHeader()) == columns(sweep::ToTsv(hungRow)));
    CHECK(sweep::ToTsv(hungRow).find('@') == std::string::npos);
}

void TestMotionJudge()
{
    const sweep::MotionThresholds th;
    // The goal travels 30 units over 20 frames and the object follows.
    sweep::ActTrack follows;
    for (int f = 0; f < 20; ++f)
        follows.Add(f, Sample(f / 60.0f, 1.5f * f, 1.5f * f));
    CHECK(sweep::JudgeActTrack(follows, th) == sweep::MotionVerdict::Moved);
    CHECK(follows.Frames() == 20 && follows.FirstFrame() == 0 && follows.LastFrame() == 19);
    CHECK(std::fabs(follows.GoalPosTravel() - 28.5f) < 1e-4f);

    // The goal travels, the object stays put: the van door with its options wiped.
    sweep::ActTrack stuck;
    for (int f = 0; f < 20; ++f)
        stuck.Add(100 + f, Sample(f / 60.0f, 1.5f * f, 7.0f));
    CHECK(sweep::JudgeActTrack(stuck, th) == sweep::MotionVerdict::Static);
    CHECK(stuck.PosTravel() == 0.0f);

    // Lagging is not static: any travel above the still threshold counts.
    sweep::ActTrack lag;
    for (int f = 0; f < 20; ++f)
        lag.Add(f, Sample(f / 60.0f, 1.5f * f, f < 15 ? 0.0f : 0.5f));
    CHECK(sweep::JudgeActTrack(lag, th) == sweep::MotionVerdict::Moved);

    // A goal that never moves (a hold pose) says nothing about the object.
    sweep::ActTrack hold;
    for (int f = 0; f < 20; ++f)
        hold.Add(f, Sample(f / 60.0f, 3.0f, 3.0f));
    CHECK(sweep::JudgeActTrack(hold, th) == sweep::MotionVerdict::Moved);

    // Too few frames for a verdict.
    sweep::ActTrack brief;
    for (int f = 0; f < 5; ++f)
        brief.Add(f, Sample(f / 60.0f, 10.0f * f, 0.0f));
    CHECK(sweep::JudgeActTrack(brief, th) == sweep::MotionVerdict::TooShort);

    // Another action drives the object: frames where this one does not
    // drive it are ignored, so the static object is not this segment's fault.
    sweep::ActTrack other;
    for (int f = 0; f < 20; ++f)
        other.Add(f, Sample(f / 60.0f, 1.5f * f, 0.0f, false));
    CHECK(sweep::JudgeActTrack(other, th) == sweep::MotionVerdict::NotDriving);

    // An object the game does not update (outside the world, like Bentley's
    // TV speaker before it is shown, or frozen): its segment's goal moves,
    // the object does not, and that is not a fault.
    sweep::ActTrack hidden;
    for (int f = 0; f < 20; ++f)
    {
        sweep::ActSample s = Sample(f / 60.0f, 1.5f * f, 0.0f);
        s.updated = false;
        hidden.Add(f, s);
    }
    CHECK(sweep::JudgeActTrack(hidden, th) == sweep::MotionVerdict::NotDriving);

    // Rotation only: the goal turns, the object does not.
    sweep::ActTrack turn;
    for (int f = 0; f < 20; ++f)
    {
        sweep::ActSample s = Sample(f / 60.0f, 0.0f, 0.0f);
        s.hasPosCurve = false;
        s.hasRotCurve = true;
        s.goalRot[0] = std::cos(0.05f * f);
        s.goalRot[1] = std::sin(0.05f * f);
        turn.Add(f, s);
    }
    CHECK(sweep::JudgeActTrack(turn, th) == sweep::MotionVerdict::Static);
    CHECK(turn.GoalRotTravel() > 0.02f && turn.RotTravel() == 0.0f);
    CHECK(std::string(sweep::MotionVerdictName(sweep::MotionVerdict::Static)) == "static");
}

void TestSegmentAndTransition()
{
    const sweep::MotionThresholds th;
    sweep::SegmentTrack none;
    for (int f = 0; f < 20; ++f)
        none.Add(f / 60.0f, 2, 0);
    CHECK(sweep::SegmentUnresolved(none, th));
    sweep::SegmentTrack resolved = none;
    resolved.Add(0.5f, 2, 1);
    CHECK(!sweep::SegmentUnresolved(resolved, th));
    // Sound-only segments (no curves) and segments whose time never moves
    // are not judged.
    sweep::SegmentTrack sound;
    for (int f = 0; f < 20; ++f)
        sound.Add(f / 60.0f, 0, 0);
    CHECK(!sweep::SegmentUnresolved(sound, th));
    sweep::SegmentTrack frozen;
    for (int f = 0; f < 20; ++f)
        frozen.Add(0.0f, 2, 0);
    CHECK(!sweep::SegmentUnresolved(frozen, th));

    sweep::TransitionTrack moving;
    for (int f = 0; f < 200; ++f)
        moving.Add(f / 60.0f, true);
    CHECK(!sweep::TransitionStalled(moving, 120));
    sweep::TransitionTrack stalled;
    for (int f = 0; f < 200; ++f)
        stalled.Add(f < 10 ? f / 60.0f : 0.25f, true);
    CHECK(stalled.stalledFrames >= 189);
    CHECK(sweep::TransitionStalled(stalled, 120));
    // Held on purpose (speed 0 or paused) is not stalled.
    sweep::TransitionTrack held;
    for (int f = 0; f < 200; ++f)
        held.Add(0.0f, false);
    CHECK(!sweep::TransitionStalled(held, 120));
}

void TestTraceDiff()
{
    sweep::Trace baseline;
    baseline.level = "A Stealthy Approach";
    baseline.frames = 900;
    baseline.events = { { 10, "lo+", "1860:41" }, { 240, "sm", "1863@1862:1871>911" }, { 300, "aseg+", "1880@1859" },
        { 301, "aseg+", "1880@1859" } };
    sweep::Trace run = baseline;
    // Same keys at other frames and counts: no difference.
    run.events = { { 12, "lo+", "1860:41" }, { 260, "sm", "1863@1862:1871>911" }, { 290, "aseg+", "1880@1859" } };
    sweep::TraceDiff same = sweep::DiffTraces(run, baseline);
    CHECK(same.added.empty() && same.removed.empty());
    CHECK(sweep::TraceKeys(baseline).at("aseg+ 1880@1859") == 2);

    // The van with its options wiped: the button never comes back, the door
    // never opens, and the door's closed state repeats.
    run.events = { { 300, "aseg+", "1880@1859" }, { 400, "sm", "1863@1862:1871>1871" } };
    sweep::TraceDiff van = sweep::DiffTraces(run, baseline);
    CHECK(van.removed.size() == 2);
    CHECK(van.added.size() == 1 && van.added[0] == "sm 1863@1862:1871>1871");

    const sweep::Trace back = sweep::TraceFromJson(sweep::ToJson(baseline));
    CHECK(back.level == baseline.level && back.frames == 900 && back.events.size() == 4);
    CHECK(back.events[1].kind == "sm" && back.events[1].key == "1863@1862:1871>911" && back.events[1].frame == 240);

    CHECK(sweep::GlobMatch("1863@*:*>911", "1863@1862:1871>911"));
    CHECK(!sweep::GlobMatch("1863@*:*>911", "1863@1862:1871>912"));
    CHECK(sweep::GlobMatch("*", ""));
    CHECK(sweep::GlobMatch("a*b*c", "aXbYc"));
    CHECK(!sweep::GlobMatch("a*b*c", "aXbY"));
    CHECK(sweep::GlobMatch("exact", "exact") && !sweep::GlobMatch("exact", "exactly"));

    const nlohmann::json list = nlohmann::json::parse(R"([
        { "level": "A Stealthy Approach", "kind": "sm", "key": "1863@*:*>911", "by_frame": 900, "why": "van door opens" },
        { "level": "A Stealthy Approach", "kind": "lo+", "key": "1860:*", "why": "door button back" },
        { "level": "Prowling the Grounds", "kind": "sm", "key": "556@1843:558>559", "why": "other level" }
    ])");
    const std::vector<sweep::MustHappen> must = sweep::MustHappenFromJson(list);
    CHECK(must.size() == 3 && must[0].byFrame == 900 && must[1].byFrame == -1);
    CHECK(sweep::MissingMustHappen(baseline, must).empty());
    const std::vector<std::string> missing = sweep::MissingMustHappen(run, must);
    CHECK(missing.size() == 2);
    CHECK(!missing.empty() && missing[0].find("van door opens") != std::string::npos);
    // Too late counts as missing.
    sweep::Trace late = baseline;
    late.events[1].frame = 1200;
    CHECK(sweep::MissingMustHappen(late, must).size() == 1);

    // The committed file's object form, and a list that would check nothing:
    // no events, a level that is not one of the game's (a typo), an entry
    // without a kind or key, an entry whose level did not run.
    const std::vector<std::string> levels = { "A Stealthy Approach", "Prowling the Grounds" };
    const nlohmann::json file = nlohmann::json::parse(R"({ "comment": ["c"], "events": [
        { "level": "A Stealthy Approach", "kind": "sm", "key": "1863@1862:1871>911", "by_frame": 900, "why": "door" }
    ] })");
    CHECK(sweep::MustHappenFromJson(file).size() == 1 && sweep::MustHappenFromJson(file)[0].byFrame == 900);
    CHECK(sweep::MustHappenProblems(file, levels).empty());
    CHECK(sweep::MustHappenProblems(nlohmann::json::parse(R"({ "comment": ["c"] })"), levels).size() == 1);
    CHECK(sweep::MustHappenProblems(nlohmann::json::parse(R"({ "events": [] })"), levels).size() == 1);
    CHECK(sweep::MustHappenProblems(nlohmann::json::parse(R"({ "events": [
        { "level": "A Stealthy Aproach", "kind": "sm", "key": "k" } ] })"), levels).size() == 1);
    CHECK(sweep::MustHappenProblems(nlohmann::json::parse(R"({ "events": [
        { "level": "A Stealthy Approach", "kind": "sm" } ] })"), levels).size() == 1);
    CHECK(sweep::MustHappenProblems(nlohmann::json::parse(R"([ 1 ])"), levels).size() == 1);
    CHECK(sweep::UnevaluatedMustHappen(must, { "A Stealthy Approach" }).size() == 1);
    CHECK(sweep::UnevaluatedMustHappen(must, { "A Stealthy Approach", "Prowling the Grounds" }).empty());
}

void TestBreakables()
{
    CHECK(!sweep::CaneBreaks(3));  // stop + bounce: the locked fuse
    CHECK(sweep::CaneBreaks(17));  // stop + hit: the unlocked fuse
    CHECK(sweep::CaneBreaks(16));
    CHECK(!sweep::CaneBreaks(18)); // bounce wins over hit
    CHECK(!sweep::CaneBreaks(0));

    sweep::BreakableRecord fuse;
    fuse.oid = 1487;
    fuse.cid = 33;
    fuse.trigger = "lockg 1843";
    fuse.sweepBefore = 3;
    fuse.sweepAfter = 17;
    fuse.hitTried = true;
    fuse.broke = true;
    fuse.conclusiveSwings = 1;
    CHECK(sweep::JudgeBreakables({ fuse }).empty());
    // Unlocked, and two swings from beside it (Sly standing, close) did not
    // break it: a finding.
    sweep::BreakableRecord bounced = fuse;
    bounced.broke = false;
    bounced.conclusiveSwings = 2;
    CHECK(sweep::JudgeBreakables({ bounced }).size() == 1);
    CHECK(sweep::BreakableNotes({ bounced }).empty());
    // One conclusive miss is not enough (the cane can miss): a note.
    sweep::BreakableRecord once = bounced;
    once.conclusiveSwings = 1;
    CHECK(sweep::JudgeBreakables({ once }).empty());
    CHECK(sweep::BreakableNotes({ once }).size() == 1);
    // The live fuse that stayed at bounce after the unlock is not "flipped"
    // here; the live/reload comparison names it. An object a switch makes
    // unbreakable (a guarded window) is a design change, noted, not failed.
    sweep::BreakableRecord lost = fuse;
    lost.sweepBefore = 16;
    lost.sweepAfter = 7;
    lost.hitTried = false;
    lost.broke = false;
    CHECK(sweep::JudgeBreakables({ lost }).empty());
    CHECK(sweep::BreakableNotes({ lost }).size() == 1);
    // Became breakable while out of the world, so no swing was tried: noted.
    sweep::BreakableRecord untried = fuse;
    untried.hitTried = false;
    untried.broke = false;
    untried.conclusiveSwings = 0;
    CHECK(sweep::JudgeBreakables({ untried }).empty());
    CHECK(sweep::BreakableNotes({ untried }).size() == 1);
    CHECK(sweep::ToJson(fuse)["oid"] == 1487);
    // Swings that never had Sly standing close beside it (he fell, or was
    // zapped) prove nothing: advisory, not a finding.
    sweep::BreakableRecord fell = bounced;
    fell.conclusiveSwings = 0;
    CHECK(sweep::JudgeBreakables({ fell }).empty());
    CHECK(sweep::BreakableNotes({ fell }).size() == 1);
    CHECK(sweep::ToJson(fell)["conclusive_swings"] == 0);

    // The unlock summary's swing coverage, from the rows the children write.
    const sweep::BreakableRecord back = sweep::BreakableFromJson(sweep::ToJson(bounced));
    CHECK(back.oid == 1487 && back.sweepBefore == 3 && back.sweepAfter == 17 && back.hitTried && !back.broke &&
        back.conclusiveSwings == 2 && back.trigger == "lockg 1843");
    sweep::SwingCoverage coverage;
    for (const sweep::BreakableRecord& r : { fuse, bounced, once, lost, untried, fell })
        sweep::AddSwingCoverage(coverage, sweep::BreakableFromJson(sweep::ToJson(r)));
    // fuse, bounced, once, untried and fell became breakable (lost stopped
    // being); four were swung at; fuse (broke) and bounced (two conclusive
    // misses) are judged; once and fell got too few swings that count.
    CHECK(coverage.becameBreakable == 5);
    CHECK(coverage.swungAt == 4);
    CHECK(coverage.judged == 2);
    CHECK(coverage.broke == 1);
    CHECK(coverage.conclusiveSwings == 1 + 2 + 1 + 0);
    CHECK(sweep::ToJson(coverage)["judged"] == 2);
}

void TestLiveReload()
{
    sweep::Snapshot fresh;
    fresh.objects["1487:33 'fuse'"] = { 1, 1, 3, 0, "" };
    fresh.objects["1843:93 'breaker'"] = { 1, 1, 0, -1, "556:558" };
    fresh.objects["2001:46 'clue'"] = { 1, 1, 0, -1, "" };
    fresh.flags["fws"] = "0x1";

    // The working game: the unlock makes the fuse breakable, the swing breaks
    // it, the clue is collected; the reload agrees on everything.
    sweep::Snapshot live = fresh;
    live.objects["1487:33 'fuse'"] = { 0, 1, 17, 1, "" };
    live.objects["1843:93 'breaker'"].sm = "556:559";
    live.objects["2001:46 'clue'"].inWorld = 0;
    live.flags["fws"] = "0x3";
    sweep::Snapshot reload = live;
    reload.objects["1487:33 'fuse'"] = { 0, 1, 3, 0, "" };
    // A broken BRK's response and broken flag are not compared once it is
    // gone in both: only presence is.
    CHECK(sweep::CompareLiveReload(fresh, live, reload).empty());

    // The user's fuse: the unlock ran but the response stayed at bounce, so
    // the swing bounced and the fuse is still there; the reload removes it.
    sweep::Snapshot bug = live;
    bug.objects["1487:33 'fuse'"] = { 1, 1, 3, 0, "" };
    const std::vector<sweep::ReloadMismatch> found = sweep::CompareLiveReload(fresh, bug, reload);
    CHECK(found.size() == 1);
    if (!found.empty())
    {
        CHECK(found[0].field == "presence");
        CHECK(found[0].live == "in world, response 3 (bounce)");
        CHECK(found[0].reload == "not in world");
        CHECK(found[0].story.find("reload removes") != std::string::npos);
        CHECK(sweep::ToJson(found[0])["field"] == "presence");
    }

    // A clue collected live that comes back on reload, a response and a
    // state machine that differ, and a flag the save lost.
    sweep::Snapshot lost = reload;
    lost.objects["2001:46 'clue'"].inWorld = 1;
    lost.objects["1843:93 'breaker'"].sm = "556:558";
    lost.flags["fws"] = "0x1";
    const std::vector<sweep::ReloadMismatch> more = sweep::CompareLiveReload(fresh, live, lost);
    CHECK(more.size() == 3);
    bool sawClue = false, sawSm = false, sawFlag = false;
    for (const sweep::ReloadMismatch& m : more)
    {
        sawClue |= m.field == "presence" && m.key == "2001:46 'clue'" && m.story.find("comes back") != std::string::npos;
        sawSm |= m.field == "sm" && m.live == "556:559" && m.reload == "556:558";
        sawFlag |= m.field == "flag" && m.key == "fws";
    }
    CHECK(sawClue && sawSm && sawFlag);

    // Both present with different responses: fails for an object the
    // sweep drove (its unlock changed the response), advisory for one that
    // changes by itself with level time (the reload is earlier in it).
    sweep::Snapshot response = live;
    response.objects["1487:33 'fuse'"] = { 1, 1, 17, 0, "" };
    sweep::Snapshot responseReload = response;
    responseReload.objects["1487:33 'fuse'"].sweep = 3;
    const std::vector<sweep::ReloadMismatch> r = sweep::CompareLiveReload(fresh, response, responseReload);
    CHECK(r.size() == 1 && !r.empty() && r[0].field == "response");
    sweep::ReloadContext undriven;
    CHECK(!sweep::ReloadMismatchFails(r.at(0), undriven));
    sweep::ReloadContext driven;
    driven.drivenKeys.insert("1487:33 'fuse'");
    CHECK(sweep::ReloadMismatchFails(r.at(0), driven));

    // Keys only in one snapshot count as not loaded there.
    sweep::Snapshot extra = live;
    extra.objects["9999:88 'coin'"] = { 1, 1, 0, -1, "" };
    CHECK(sweep::CompareLiveReload(fresh, extra, live).size() == 1);

    // Which mismatches fail. The fuse that stays unbreakable live while the
    // reload removes it fails; a breakable one the swing never reached is a
    // pending break (advisory).
    sweep::ReloadContext ctx;
    CHECK(sweep::ReloadMismatchFails(found.at(0), ctx));
    sweep::Snapshot pending = live;
    pending.objects["1487:33 'fuse'"] = { 1, 1, 17, 0, "" };
    const std::vector<sweep::ReloadMismatch> p = sweep::CompareLiveReload(fresh, pending, reload);
    CHECK(p.size() == 1 && !sweep::ReloadMismatchFails(p.at(0), ctx));
    // A collected clue that comes back, and a lost flag, fail.
    for (const sweep::ReloadMismatch& m : more)
        if (m.field != "sm")
            CHECK(sweep::ReloadMismatchFails(m, ctx));
    // A lock group's state differs: fails only when Sly walking up to it on
    // the reloaded level does not bring it back to the live state.
    for (const sweep::ReloadMismatch& m : more)
        if (m.field == "sm")
        {
            CHECK(!sweep::ReloadMismatchFails(m, ctx));
            ctx.reapproached[m.key] = "556:559";
            CHECK(!sweep::ReloadMismatchFails(m, ctx));
            ctx.reapproached[m.key] = "556:558";
            CHECK(sweep::ReloadMismatchFails(m, ctx));
        }
    // An ordinary breakable broken live comes back on reload (the game does
    // not save broken crates): advisory. A clue that comes back fails.
    sweep::Snapshot crate = live;
    crate.objects["3001:32 'crate'"] = { 0, 1, -1, -1, "" };
    sweep::Snapshot crateReload = live;
    crateReload.objects["3001:32 'crate'"] = { 1, 1, 17, 0, "" };
    const std::vector<sweep::ReloadMismatch> c = sweep::CompareLiveReload(fresh, crate, crateReload);
    CHECK(c.size() == 1 && c.at(0).breakable && !sweep::ReloadMismatchFails(c.at(0), ctx));
    CHECK(c.at(0).story.find("comes back") != std::string::npos);

    // Item: the save loses a break. The sweep broke the fuse live (its
    // unlock was saved), and the reload brings it back: fails. Undriven (a
    // crate, above) it is advisory.
    sweep::ReloadContext drove;
    drove.drivenKeys.insert("1487:33 'fuse'");
    sweep::Snapshot fuseBack = reload;
    fuseBack.objects["1487:33 'fuse'"] = { 1, 1, 3, 0, "" };
    const std::vector<sweep::ReloadMismatch> fb = sweep::CompareLiveReload(fresh, live, fuseBack);
    CHECK(fb.size() == 1 && fb.at(0).field == "presence" && fb.at(0).breakable);
    CHECK(sweep::ReloadMismatchFails(fb.at(0), drove));
    CHECK(!sweep::ReloadMismatchFails(fb.at(0), ctx));

    // Item: present live, gone after the reload. For an object the sweep
    // drove (a missed live transition) it fails, whatever its class; for one
    // it did not drive it is advisory; a driven breakable still breakable
    // live is a pending break (advisory).
    sweep::Snapshot button = live;
    button.objects["1445:40"] = { 1, 1, 1, -1, "" };
    sweep::Snapshot buttonReload = live;
    buttonReload.objects["1445:40"] = { 0, 1, -1, -1, "" };
    const std::vector<sweep::ReloadMismatch> bt = sweep::CompareLiveReload(fresh, button, buttonReload);
    CHECK(bt.size() == 1 && !bt.at(0).breakable);
    CHECK(!sweep::ReloadMismatchFails(bt.at(0), ctx));
    sweep::ReloadContext droveButton;
    droveButton.drivenKeys.insert("1445:40");
    CHECK(sweep::ReloadMismatchFails(bt.at(0), droveButton));
    CHECK(!sweep::ReloadMismatchFails(p.at(0), drove)); // pending break of a driven fuse

    // A lock group (not a breakable) that is open live and gone after the
    // reload: the reload shows the end state of the opened group. Advisory.
    sweep::Snapshot group = live;
    group.objects["1444:93"] = { 1, 1, 0, -1, "556:559" };
    sweep::Snapshot groupReload = live;
    groupReload.objects["1444:93"] = { 0, 1, -1, -1, "" };
    const std::vector<sweep::ReloadMismatch> g = sweep::CompareLiveReload(fresh, group, groupReload);
    CHECK(g.size() == 1 && !g.at(0).breakable && !sweep::ReloadMismatchFails(g.at(0), ctx));

    // A flag the save kept but the level's own load set back (its script
    // starts the level with it): advisory. One the save itself lost fails.
    sweep::ReloadMismatch flag{ "ldata 1339", "flag", "0", "1", "0", "", 0, 0, false, false };
    CHECK(sweep::ReloadMismatchFails(flag, ctx));
    sweep::ReloadContext saved;
    saved.savedFlags["ldata 1339"] = "1";
    CHECK(!sweep::ReloadMismatchFails(flag, saved));
    saved.savedFlags["ldata 1339"] = "0";
    CHECK(sweep::ReloadMismatchFails(flag, saved));

    // Lock pieces (LOCK) of a group: the reload removing some is the opened
    // group's end state (advisory); pieces live play removed that come back
    // fail, unless their group is open in both (an advisory key).
    sweep::Snapshot locks = live;
    locks.objects["1371:92"] = { 10, 10, 0, -1, "" };
    sweep::Snapshot locksReload = live;
    locksReload.objects["1371:92"] = { 4, 10, 0, -1, "" };
    const std::vector<sweep::ReloadMismatch> l = sweep::CompareLiveReload(fresh, locks, locksReload);
    CHECK(l.size() == 1 && !sweep::ReloadMismatchFails(l.at(0), ctx));
    const std::vector<sweep::ReloadMismatch> back = sweep::CompareLiveReload(fresh, locksReload, locks);
    CHECK(back.size() == 1 && sweep::ReloadMismatchFails(back.at(0), ctx));
    ctx.advisoryKeys.insert("1371:92");
    CHECK(!sweep::ReloadMismatchFails(back.at(0), ctx));

    // Known findings (filed upstream) are reported, not failed,
    // like the sanitizer suppressions. An entry names the whole key, the
    // field and both values (so the direction), and matches only that.
    const nlohmann::json knownJson = nlohmann::json::parse(R"J({ "comment": ["c"], "findings": [
        { "check": "unlock", "level": "Muggshot's Turf", "key": "1719:40", "field": "response",
          "live": "1", "reload": "5", "writeup": "theclub654/ProjectCane#63" },
        { "check": "unlock", "level": "Paris, France", "key": "1397:23", "field": "presence",
          "live": "not in world", "reload": "in world, response 3 (bounce)", "writeup": "theclub654/ProjectCane#64" },
        { "check": "fuse", "level": "Prowling the Grounds", "reason": "the fuse broke twice", "writeup": "not filed" }
    ] })J");
    CHECK(sweep::KnownFindingProblems(knownJson).empty());
    const std::vector<sweep::KnownFinding> known = sweep::KnownFindingsFromJson(knownJson);
    CHECK(known.size() == 3);
    std::set<size_t> used;
    CHECK(sweep::KnownWriteup(known, "unlock", "Muggshot's Turf",
        "live vs reload: 1719:40 response: live 1, reload 5 (response differs)", &used) == "theclub654/ProjectCane#63");
    CHECK(used.count(0) == 1);
    // Not a substring match: another key, a key that only starts the same,
    // the other direction, another check or level.
    CHECK(sweep::KnownWriteup(known, "unlock", "Muggshot's Turf",
        "live vs reload: 1979:40 response: live 1, reload 5 (response differs)", &used).empty());
    CHECK(sweep::KnownWriteup(known, "unlock", "Muggshot's Turf",
        "live vs reload: 11719:40 response: live 1, reload 5 (x)", &used).empty());
    CHECK(sweep::KnownWriteup(known, "unlock", "Muggshot's Turf",
        "live vs reload: 1719:40 response: live 5, reload 1 (x)", &used).empty());
    CHECK(sweep::KnownWriteup(known, "behaviour", "Muggshot's Turf",
        "live vs reload: 1719:40 response: live 1, reload 5 (x)", &used).empty());
    CHECK(sweep::KnownWriteup(known, "unlock", "Paris, France",
        "live vs reload: 1719:40 response: live 1, reload 5 (x)", &used).empty());
    CHECK(sweep::KnownWriteup(known, "fuse", "Prowling the Grounds", "the fuse broke twice", &used) == "not filed");
    CHECK(sweep::KnownWriteup(known, "fuse", "Prowling the Grounds", "the fuse broke twice, again", &used).empty());
    // Entries for levels that ran but matched nothing are stale (advisory).
    const std::vector<std::string> stale = sweep::StaleKnown(known, "unlock", { "Muggshot's Turf", "Paris, France" }, used);
    CHECK(stale.size() == 1 && stale.at(0).find("1397:23") != std::string::npos);
    CHECK(sweep::StaleKnown(known, "unlock", { "Muggshot's Turf" }, used).empty());
    // Malformed files are refused before a run.
    CHECK(!sweep::KnownFindingProblems(nlohmann::json::parse(R"({ "comment": "no findings" })")).empty());
    CHECK(!sweep::KnownFindingProblems(nlohmann::json::parse(
        R"({ "findings": [ { "check": "unlock", "level": "L", "key": "1:2", "writeup": "W" } ] })")).empty());
    CHECK(!sweep::KnownFindingProblems(nlohmann::json::parse(
        R"({ "findings": [ { "check": "unlock", "level": "L", "reason": "r" } ] })")).empty());

    const sweep::Snapshot copy = sweep::SnapshotFromJson(sweep::ToJson(live));
    CHECK(copy.objects.size() == live.objects.size() && copy.flags.at("fws") == "0x3");
    CHECK(copy.objects.at("1843:93 'breaker'").sm == "556:559" && copy.objects.at("1487:33 'fuse'").sweep == 17);
}

}

int main()
{
    TestImageCompare();
    TestImageStats();
    TestPngRoundTrip();
    TestCompareFolders();
    TestTransformScan();
    TestParkedTransforms();
    TestHashAndDivergence();
    TestLogSplit();
    TestSanitizerReports();
    TestChildTimers();
    TestSampleProcess();
    TestFrameStats();
    TestLevelResultRows();
    TestChildOutcome();
    TestMotionJudge();
    TestSegmentAndTransition();
    TestTraceDiff();
    TestBreakables();
    TestLiveReload();
    std::printf("%d/%d checks passed\n", g_checks - g_failures, g_checks);
    return g_failures == 0 ? 0 : 1;
}
