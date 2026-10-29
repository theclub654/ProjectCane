#pragma once
// Game-independent judging for the world sweep's behaviour checks
// (sweep_behaviour_child.cpp records what the game did; this file decides
// what is wrong with it). Nothing here touches the game, so the selftest
// covers every rule without data:
//
// - motion: an animation segment (ASEG) is applied and its channel's goal
//   moves, but the object it drives never does ("applied, target static");
//   a segment whose channels have curves resolves no object at all; a state
//   machine transition whose animation never advances.
// - trace: a structural record of a level's first seconds (state machine
//   transitions, segments applied and retracted, objects added to and
//   removed from the world, custom option writes, hit responses, world
//   state) compared as sets of keys, never frame by frame, against a local
//   baseline (advisory) and a list of events that must happen (failures).
// - breakables: an object whose cane response was changed at run time (an
//   unlock) must break when Sly swings at it.
// - live vs reload: the persisted-relevant state of a level after in-session
//   changes must match the same level saved and loaded again.

#include <nlohmann/json.hpp>

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace sweep
{
// --- motion -------------------------------------------------------------------

// One frame of one channel of an applied segment: where the channel's
// curves want the object (local position and rotation) and where it is.
struct ActSample
{
    float tLocal = 0.0f;
    std::array<float, 3> goalPos{};
    std::array<float, 9> goalRot{};
    std::array<float, 3> pos{};
    std::array<float, 9> rot{};
    // The channel has a position / rotation curve, and this segment is the
    // object's driving action for it (ALO::pactPos / pactRot). An object
    // the game does not update (outside the world, or frozen: UpdateAsega
    // leaves a segment with a frozen target where it is) is never judged.
    bool updated = true;
    bool hasPosCurve = false;
    bool hasRotCurve = false;
    bool drivesPos = false;
    bool drivesRot = false;
};

struct MotionThresholds
{
    float goalPos = 1.0f;     // the goal must travel this far ...
    float goalRot = 0.02f;    // ... or turn this much (max matrix element change)
    float stillPos = 1.0e-3f; // and the object travel less than this
    float stillRot = 1.0e-4f;
    int minFrames = 10;       // driving frames needed for a verdict
};

// Everything one channel of one applied segment did, frame by frame.
class ActTrack
{
public:
    std::string label; // "aseg A root R target T 'name'"
    void Add(int frame, const ActSample& sample);
    int Frames() const { return m_frames; }
    int FirstFrame() const { return m_firstFrame; }
    int LastFrame() const { return m_lastFrame; }
    // Travel from the first driving sample, per component.
    float GoalPosTravel() const { return m_goalPosTravel; }
    float GoalRotTravel() const { return m_goalRotTravel; }
    float PosTravel() const { return m_posTravel; }
    float RotTravel() const { return m_rotTravel; }
    int PosFrames() const { return m_posFrames; }
    int RotFrames() const { return m_rotFrames; }
    float TAdvance() const { return m_tMax - m_tMin; }

private:
    int m_frames = 0;
    int m_firstFrame = -1;
    int m_lastFrame = -1;
    float m_tMin = 0.0f;
    float m_tMax = 0.0f;
    int m_posFrames = 0;
    int m_rotFrames = 0;
    std::array<float, 3> m_goalPos0{};
    std::array<float, 3> m_pos0{};
    std::array<float, 9> m_goalRot0{};
    std::array<float, 9> m_rot0{};
    float m_goalPosTravel = 0.0f;
    float m_goalRotTravel = 0.0f;
    float m_posTravel = 0.0f;
    float m_rotTravel = 0.0f;
};

enum class MotionVerdict
{
    Moved,        // the object followed (or the goal never moved)
    Static,       // the goal moved, the driven object did not
    TooShort,     // fewer driving frames than minFrames
    NotDriving,   // another action drives the object
};
MotionVerdict JudgeActTrack(const ActTrack& track, const MotionThresholds& thresholds);
const char* MotionVerdictName(MotionVerdict verdict);

// An applied segment as a whole.
struct SegmentTrack
{
    std::string label;            // "aseg A root R"
    int frames = 0;
    float tMin = 0.0f;
    float tMax = 0.0f;
    int channelsWithCurves = 0;   // channels with a position/rotation curve
    int maxActsegs = 0;           // most channels resolved to objects at once
    void Add(float tLocal, int curveChannels, int actsegs);
};
// The segment advanced for minFrames with curve channels, yet no channel
// ever resolved to an object.
bool SegmentUnresolved(const SegmentTrack& track, const MotionThresholds& thresholds);

// A state machine transition (cur -> next) and the time of its animation.
struct TransitionTrack
{
    std::string label;            // "sm S root R from>to"
    int frames = 0;               // frames spent in the transition
    int stalledFrames = 0;        // longest run with time unchanged while running
    float tLast = 0.0f;
    int run = 0;
    void Add(float tLocal, bool running);
};
// The transition animation stood still while running (speed not zero, not
// paused) for at least `frames` frames.
bool TransitionStalled(const TransitionTrack& track, int frames);

// --- trace --------------------------------------------------------------------

struct TraceEvent
{
    int frame = 0;
    std::string kind; // sm, aseg+, aseg-, lo+, lo-, opt, ficg, state, transition
    std::string key;
};

struct Trace
{
    std::string level;
    int frames = 0;
    std::vector<TraceEvent> events;
};

nlohmann::json ToJson(const Trace& trace);
Trace TraceFromJson(const nlohmann::json& json);
// "kind key" -> how many times it happened.
std::map<std::string, int> TraceKeys(const Trace& trace);

struct TraceDiff
{
    std::vector<std::string> added;   // in the run, not in the baseline
    std::vector<std::string> removed; // in the baseline, not in the run
};
// Compares which keys happened, never when or how often.
TraceDiff DiffTraces(const Trace& run, const Trace& baseline);

// Shell-style match with '*' for any run of characters.
bool GlobMatch(const std::string& pattern, const std::string& text);

struct MustHappen
{
    std::string level;
    std::string kind;
    std::string key;    // may hold '*'
    int byFrame = -1;   // -1: any time in the run
    std::string why;
};
std::vector<MustHappen> MustHappenFromJson(const nlohmann::json& json);
// Why the list would check nothing, or less than it says: no "events" (or
// none), an entry without a level, kind or key, a level that is not one of
// `levels` (the game's 46: a typo). Empty when the list is usable.
std::vector<std::string> MustHappenProblems(const nlohmann::json& json, const std::vector<std::string>& levels);
// Entries whose level is not among the levels that ran (and loaded), so
// nothing evaluated them.
std::vector<std::string> UnevaluatedMustHappen(const std::vector<MustHappen>& list, const std::vector<std::string>& ran);
// The entries for trace.level that the trace lacks, as report lines.
std::vector<std::string> MissingMustHappen(const Trace& trace, const std::vector<MustHappen>& list);

// --- breakables -----------------------------------------------------------------

// grficSweep (FICG): bit 1 stop, 2 bounce, 16 hit. A cane sweep breaks an
// object when it is hit and not bounced.
bool CaneBreaks(int grficSweep);

struct BreakableRecord
{
    int oid = -1;
    int cid = -1;
    std::string name;
    std::string trigger;      // what changed it ("lockg 1843")
    int sweepBefore = -1;
    int sweepAfter = -1;
    bool hitTried = false;
    bool broke = false;
    int swings = 0;
    // Swings that count: from beside the target (not from wherever Sly
    // happened to be), Sly standing, close enough for the cane to reach.
    int conclusiveSwings = 0;
};
// Swings from beside a breakable that must all miss before it is called a
// failure (one swing can miss).
constexpr int kConclusiveSwingsToFail = 2;
// Findings: objects whose response became breakable but that
// kConclusiveSwingsToFail swings from beside them did not break.
std::vector<std::string> JudgeBreakables(const std::vector<BreakableRecord>& records);
// Notes (advisory): became breakable but too few swings could have reached
// it, or it was out of the world so none was tried; stopped being breakable
// (a switch guarding a window is a design change).
std::vector<std::string> BreakableNotes(const std::vector<BreakableRecord>& records);
nlohmann::json ToJson(const BreakableRecord& record);
BreakableRecord BreakableFromJson(const nlohmann::json& json);

// How far the swings covered the objects that became breakable (the unlock
// summary; the fuse check reports its own swings). One is judged when it
// broke, or when kConclusiveSwingsToFail swings from beside it missed, so a
// failure could be called; the others got too few swings that count, or
// none (out of the world).
struct SwingCoverage
{
    int becameBreakable = 0;
    int swungAt = 0;
    int judged = 0;
    int broke = 0;
    int conclusiveSwings = 0;
};
void AddSwingCoverage(SwingCoverage& coverage, const BreakableRecord& record);
nlohmann::json ToJson(const SwingCoverage& coverage);

// --- live vs reload -------------------------------------------------------------

struct ObjSnap
{
    int inWorld = 0;   // instances in the world
    int count = 0;     // instances loaded
    int sweep = -1;    // grficSweep of the first instance in the world
    int broken = -1;   // BRK::fBroken, -1 when not a BRK
    std::string sm;    // "sm:state" of state machines rooted on it
};

struct Snapshot
{
    std::map<std::string, ObjSnap> objects; // "oid:cid 'name'"
    std::map<std::string, std::string> flags;
};
nlohmann::json ToJson(const Snapshot& snapshot);
Snapshot SnapshotFromJson(const nlohmann::json& json);

struct ReloadMismatch
{
    std::string key;
    std::string field;  // presence, response, broken, sm, flag
    std::string fresh;  // the value at a fresh load (before the changes)
    std::string live;
    std::string reload;
    std::string story;  // what the three values say
    int liveIn = 0;     // instances in the world, live and after the reload
    int reloadIn = 0;
    bool liveBreaks = false; // live response breaks on a cane swing
    bool breakable = false;  // a BRK (ObjSnap::broken known in any snapshot)
};
// Every persisted-relevant value where the live session and the reloaded
// level disagree, explained against the fresh load.
std::vector<ReloadMismatch> CompareLiveReload(const Snapshot& fresh, const Snapshot& live, const Snapshot& reload);
nlohmann::json ToJson(const ReloadMismatch& mismatch);

struct ReloadContext
{
    // Lock group key -> its state machine once Sly walked up to it on the
    // reloaded level ("sm:state").
    std::map<std::string, std::string> reapproached;
    // Keys whose differences are reported, never failed (lock pieces of a
    // group that is open in both).
    std::set<std::string> advisoryKeys;
    // Objects the sweep drove by a change the game also saved: their
    // response was set by an unlock that set its world flag (the fuse, which
    // the sweep then broke). Their presence, response and broken flag must
    // survive the reload. Other objects change with level time or with
    // transient switches the game does not save (a guard breaks a crate, a
    // pressure plate, a response that cycles), and the reload is at another
    // point in that time, so their differences are advisory.
    std::set<std::string> drivenKeys;
    // World and level state read back from the save itself (after
    // LoadCurrentSave, before the level is loaded again): a flag the save
    // kept but the level's load set back is the level's doing, not a lost
    // save.
    std::map<std::string, std::string> savedFlags;
};
// Failures: a world flag the save lost (one the level's own load sets back
// is advisory); an object present live but gone after the reload when the
// sweep drove it (live play missed what the load path does) or when it is a
// breakable the live response cannot break (the fuse); an object gone live
// and back after the reload (the save lost it), unless it is a breakable
// the sweep did not drive (broken crates are not saved); a response or
// broken flag that differs on an object the sweep drove; a lock group that
// walking up to it on the reloaded level does not bring back to its live
// state. Everything else is advisory: differences on objects the sweep did
// not drive (they change with level time), a breakable still breakable live
// (a pending break), lock pieces of a group open in both, state machines
// that wait for Sly to come near.
bool ReloadMismatchFails(const ReloadMismatch& mismatch, const ReloadContext& context);

// --- known findings ---------------------------------------------------------------

// A failure already written up for upstream (world_sweep/known_findings.json),
// reported as known instead of failing the run, like the sanitizer
// suppressions. `match` is a piece of the failure reason.
struct KnownFinding
{
    std::string check;   // behaviour, unlock, fuse
    std::string level;
    // A live/reload difference: the whole key, the field and both values
    // (so the direction too). Anything else: the whole failure reason.
    std::string key, field, live, reload;
    std::string reason;
    std::string writeup;
};
// Why a known-findings file cannot be used (no "findings", an entry without
// a check, level, write-up, or without key+field+live+reload or reason).
std::vector<std::string> KnownFindingProblems(const nlohmann::json& json);
std::vector<KnownFinding> KnownFindingsFromJson(const nlohmann::json& json);
// The write-up for this failure reason, or "" when it is not known; the
// matching entry's index goes into `used`.
std::string KnownWriteup(const std::vector<KnownFinding>& known, const std::string& check, const std::string& level,
    const std::string& reason, std::set<size_t>* used);
// Entries of `check` for levels that ran that matched nothing (stale).
std::vector<std::string> StaleKnown(const std::vector<KnownFinding>& known, const std::string& check,
    const std::vector<std::string>& ran, const std::set<size_t>& used);
}
