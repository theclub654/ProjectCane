// See sweep_behaviour.h.

#include "sweep_behaviour.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>

namespace sweep
{
namespace
{
float Distance(const std::array<float, 3>& a, const std::array<float, 3>& b)
{
    const float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float MaxElementChange(const std::array<float, 9>& a, const std::array<float, 9>& b)
{
    float change = 0.0f;
    for (size_t i = 0; i < a.size(); ++i)
        change = (std::max)(change, std::fabs(a[i] - b[i]));
    return change;
}

std::string Text(int value)
{
    return std::to_string(value);
}
}

// --- motion -------------------------------------------------------------------

void ActTrack::Add(int frame, const ActSample& sample)
{
    if (m_frames == 0)
    {
        m_firstFrame = frame;
        m_tMin = m_tMax = sample.tLocal;
    }
    ++m_frames;
    m_lastFrame = frame;
    m_tMin = (std::min)(m_tMin, sample.tLocal);
    m_tMax = (std::max)(m_tMax, sample.tLocal);
    // Travel is measured from the first frame on which this segment drives
    // the component, and only over such frames.
    if (sample.updated && sample.hasPosCurve && sample.drivesPos)
    {
        if (m_posFrames++ == 0)
        {
            m_goalPos0 = sample.goalPos;
            m_pos0 = sample.pos;
        }
        m_goalPosTravel = (std::max)(m_goalPosTravel, Distance(sample.goalPos, m_goalPos0));
        m_posTravel = (std::max)(m_posTravel, Distance(sample.pos, m_pos0));
    }
    if (sample.updated && sample.hasRotCurve && sample.drivesRot)
    {
        if (m_rotFrames++ == 0)
        {
            m_goalRot0 = sample.goalRot;
            m_rot0 = sample.rot;
        }
        m_goalRotTravel = (std::max)(m_goalRotTravel, MaxElementChange(sample.goalRot, m_goalRot0));
        m_rotTravel = (std::max)(m_rotTravel, MaxElementChange(sample.rot, m_rot0));
    }
}

MotionVerdict JudgeActTrack(const ActTrack& track, const MotionThresholds& th)
{
    const int driving = (std::max)(track.PosFrames(), track.RotFrames());
    if (driving == 0)
        return track.Frames() >= th.minFrames ? MotionVerdict::NotDriving : MotionVerdict::TooShort;
    if (driving < th.minFrames)
        return MotionVerdict::TooShort;
    const bool posStatic = track.PosFrames() >= th.minFrames && track.GoalPosTravel() > th.goalPos &&
        track.PosTravel() < th.stillPos;
    const bool rotStatic = track.RotFrames() >= th.minFrames && track.GoalRotTravel() > th.goalRot &&
        track.RotTravel() < th.stillRot;
    // Moving on the other component is still movement.
    const bool posMoved = track.PosTravel() >= th.stillPos;
    const bool rotMoved = track.RotTravel() >= th.stillRot;
    if ((posStatic && !rotMoved) || (rotStatic && !posMoved))
        return MotionVerdict::Static;
    return MotionVerdict::Moved;
}

const char* MotionVerdictName(MotionVerdict verdict)
{
    switch (verdict)
    {
    case MotionVerdict::Moved: return "moved";
    case MotionVerdict::Static: return "static";
    case MotionVerdict::TooShort: return "too short";
    case MotionVerdict::NotDriving: return "not driving";
    }
    return "?";
}

void SegmentTrack::Add(float tLocal, int curveChannels, int actsegs)
{
    if (frames++ == 0)
        tMin = tMax = tLocal;
    tMin = (std::min)(tMin, tLocal);
    tMax = (std::max)(tMax, tLocal);
    channelsWithCurves = (std::max)(channelsWithCurves, curveChannels);
    maxActsegs = (std::max)(maxActsegs, actsegs);
}

bool SegmentUnresolved(const SegmentTrack& track, const MotionThresholds& th)
{
    return track.frames >= th.minFrames && track.channelsWithCurves > 0 && track.maxActsegs == 0 &&
        track.tMax > track.tMin;
}

void TransitionTrack::Add(float tLocal, bool running)
{
    if (frames++ != 0 && running && tLocal == tLast)
        ++run;
    else
        run = 0;
    stalledFrames = (std::max)(stalledFrames, run);
    tLast = tLocal;
}

bool TransitionStalled(const TransitionTrack& track, int frames)
{
    return track.stalledFrames >= frames;
}

// --- trace --------------------------------------------------------------------

nlohmann::json ToJson(const Trace& trace)
{
    nlohmann::json events = nlohmann::json::array();
    for (const TraceEvent& event : trace.events)
        events.push_back({ event.frame, event.kind, event.key });
    return { { "level", trace.level }, { "frames", trace.frames }, { "events", events } };
}

Trace TraceFromJson(const nlohmann::json& json)
{
    Trace trace;
    trace.level = json.value("level", std::string());
    trace.frames = json.value("frames", 0);
    for (const nlohmann::json& event : json.value("events", nlohmann::json::array()))
        if (event.is_array() && event.size() == 3)
            trace.events.push_back({ event[0].get<int>(), event[1].get<std::string>(), event[2].get<std::string>() });
    return trace;
}

std::map<std::string, int> TraceKeys(const Trace& trace)
{
    std::map<std::string, int> keys;
    for (const TraceEvent& event : trace.events)
        ++keys[event.kind + " " + event.key];
    return keys;
}

TraceDiff DiffTraces(const Trace& run, const Trace& baseline)
{
    const std::map<std::string, int> a = TraceKeys(run);
    const std::map<std::string, int> b = TraceKeys(baseline);
    TraceDiff diff;
    for (const auto& [key, count] : a)
        if (b.count(key) == 0)
            diff.added.push_back(key);
    for (const auto& [key, count] : b)
        if (a.count(key) == 0)
            diff.removed.push_back(key);
    return diff;
}

bool GlobMatch(const std::string& pattern, const std::string& text)
{
    // Iterative match with one backtrack point per '*'.
    size_t p = 0, t = 0, star = std::string::npos, mark = 0;
    while (t < text.size())
    {
        if (p < pattern.size() && pattern[p] == '*')
        {
            star = p++;
            mark = t;
        }
        else if (p < pattern.size() && pattern[p] == text[t])
        {
            ++p;
            ++t;
        }
        else if (star != std::string::npos)
        {
            p = star + 1;
            t = ++mark;
        }
        else
            return false;
    }
    while (p < pattern.size() && pattern[p] == '*')
        ++p;
    return p == pattern.size();
}

std::vector<MustHappen> MustHappenFromJson(const nlohmann::json& json)
{
    std::vector<MustHappen> list;
    const nlohmann::json& entries = json.is_object() ? json.value("events", nlohmann::json::array()) : json;
    for (const nlohmann::json& entry : entries)
    {
        MustHappen item;
        item.level = entry.value("level", std::string());
        item.kind = entry.value("kind", std::string());
        item.key = entry.value("key", std::string());
        item.byFrame = entry.value("by_frame", -1);
        item.why = entry.value("why", std::string());
        list.push_back(item);
    }
    return list;
}

std::vector<std::string> MissingMustHappen(const Trace& trace, const std::vector<MustHappen>& list)
{
    std::vector<std::string> missing;
    for (const MustHappen& item : list)
    {
        if (item.level != trace.level)
            continue;
        bool found = false;
        for (const TraceEvent& event : trace.events)
            if (event.kind == item.kind && (item.byFrame < 0 || event.frame <= item.byFrame) &&
                GlobMatch(item.key, event.key))
            {
                found = true;
                break;
            }
        if (!found)
            missing.push_back(item.kind + " " + item.key +
                (item.byFrame >= 0 ? " by frame " + Text(item.byFrame) : std::string()) + " never happened (" +
                item.why + ")");
    }
    return missing;
}

// --- breakables -----------------------------------------------------------------

bool CaneBreaks(int grficSweep)
{
    return grficSweep >= 0 && (grficSweep & 16) != 0 && (grficSweep & 2) == 0;
}

std::vector<std::string> JudgeBreakables(const std::vector<BreakableRecord>& records)
{
    std::vector<std::string> findings;
    for (const BreakableRecord& r : records)
    {
        const std::string who = "oid " + Text(r.oid) + " cid " + Text(r.cid) + (r.name.empty() ? "" : " '" + r.name + "'") +
            " (" + r.trigger + ", response " + Text(r.sweepBefore) + " -> " + Text(r.sweepAfter) + ")";
        // Two swings from beside it are needed: a single one can miss.
        if (CaneBreaks(r.sweepAfter) && !CaneBreaks(r.sweepBefore) && r.hitTried && !r.broke &&
            r.conclusiveSwings >= kConclusiveSwingsToFail)
            findings.push_back(who + ": became breakable but " + Text(r.conclusiveSwings) + " cane swings from beside it (" +
                Text(r.swings) + " in all) did not break it");
    }
    return findings;
}

std::vector<std::string> BreakableNotes(const std::vector<BreakableRecord>& records)
{
    std::vector<std::string> list;
    for (const BreakableRecord& r : records)
    {
        const std::string who = "oid " + Text(r.oid) + " cid " + Text(r.cid) + " (" + r.trigger + ", response " +
            Text(r.sweepBefore) + " -> " + Text(r.sweepAfter) + ")";
        const bool became = CaneBreaks(r.sweepAfter) && !CaneBreaks(r.sweepBefore);
        if (became && !r.hitTried)
            list.push_back(who + ": became breakable while out of the world; not swung at");
        else if (became && !r.broke && r.conclusiveSwings < kConclusiveSwingsToFail)
            list.push_back(who + ": became breakable; " + Text(r.swings) + " swings, " + Text(r.conclusiveSwings) +
                " of them from beside it with Sly standing (" + Text(kConclusiveSwingsToFail) + " needed to call it)");
        else if (CaneBreaks(r.sweepBefore) && !CaneBreaks(r.sweepAfter))
            list.push_back(who + ": stopped being breakable");
    }
    return list;
}

nlohmann::json ToJson(const BreakableRecord& r)
{
    return { { "oid", r.oid }, { "cid", r.cid }, { "name", r.name }, { "trigger", r.trigger },
        { "sweep_before", r.sweepBefore }, { "sweep_after", r.sweepAfter }, { "hit_tried", r.hitTried },
        { "broke", r.broke }, { "swings", r.swings }, { "conclusive_swings", r.conclusiveSwings } };
}

BreakableRecord BreakableFromJson(const nlohmann::json& j)
{
    BreakableRecord r;
    r.oid = j.value("oid", -1);
    r.cid = j.value("cid", -1);
    r.name = j.value("name", std::string());
    r.trigger = j.value("trigger", std::string());
    r.sweepBefore = j.value("sweep_before", -1);
    r.sweepAfter = j.value("sweep_after", -1);
    r.hitTried = j.value("hit_tried", false);
    r.broke = j.value("broke", false);
    r.swings = j.value("swings", 0);
    r.conclusiveSwings = j.value("conclusive_swings", 0);
    return r;
}

void AddSwingCoverage(SwingCoverage& coverage, const BreakableRecord& r)
{
    if (!CaneBreaks(r.sweepAfter) || CaneBreaks(r.sweepBefore))
        return;
    ++coverage.becameBreakable;
    coverage.swungAt += r.hitTried ? 1 : 0;
    coverage.judged += r.hitTried && (r.broke || r.conclusiveSwings >= kConclusiveSwingsToFail) ? 1 : 0;
    coverage.broke += r.broke ? 1 : 0;
    coverage.conclusiveSwings += r.conclusiveSwings;
}

nlohmann::json ToJson(const SwingCoverage& c)
{
    return { { "became_breakable", c.becameBreakable }, { "swung_at", c.swungAt }, { "judged", c.judged },
        { "broke", c.broke }, { "conclusive_swings", c.conclusiveSwings } };
}

// --- live vs reload -------------------------------------------------------------

nlohmann::json ToJson(const Snapshot& snapshot)
{
    nlohmann::json objects = nlohmann::json::object();
    for (const auto& [key, o] : snapshot.objects)
        objects[key] = { { "in_world", o.inWorld }, { "count", o.count }, { "sweep", o.sweep }, { "broken", o.broken },
            { "sm", o.sm } };
    return { { "objects", objects }, { "flags", snapshot.flags } };
}

Snapshot SnapshotFromJson(const nlohmann::json& json)
{
    Snapshot snapshot;
    // items() needs the json kept alive for the whole loop.
    const nlohmann::json objects = json.value("objects", nlohmann::json::object());
    const nlohmann::json flags = json.value("flags", nlohmann::json::object());
    for (const auto& [key, o] : objects.items())
        snapshot.objects[key] = { o.value("in_world", 0), o.value("count", 0), o.value("sweep", -1), o.value("broken", -1),
            o.value("sm", std::string()) };
    for (const auto& [key, value] : flags.items())
        snapshot.flags[key] = value.get<std::string>();
    return snapshot;
}

namespace
{
std::string Presence(const ObjSnap* o)
{
    if (o == nullptr || o->count == 0)
        return "not loaded";
    if (o->inWorld == 0)
        return "not in world";
    std::string text = o->inWorld > 1 ? Text(o->inWorld) + " in world" : "in world";
    if (o->sweep >= 0)
        text += ", response " + Text(o->sweep) + (CaneBreaks(o->sweep) ? " (breaks)" : (o->sweep & 2) ? " (bounce)" : "");
    return text;
}

const ObjSnap* Find(const Snapshot& s, const std::string& key)
{
    const auto it = s.objects.find(key);
    return it == s.objects.end() ? nullptr : &it->second;
}

int InWorld(const ObjSnap* o)
{
    return o != nullptr ? o->inWorld : 0;
}
}

std::vector<ReloadMismatch> CompareLiveReload(const Snapshot& fresh, const Snapshot& live, const Snapshot& reload)
{
    std::vector<ReloadMismatch> mismatches;
    std::set<std::string> keys;
    for (const Snapshot* s : { &live, &reload })
        for (const auto& [key, o] : s->objects)
            keys.insert(key);
    for (const std::string& key : keys)
    {
        const ObjSnap* f = Find(fresh, key);
        const ObjSnap* l = Find(live, key);
        const ObjSnap* r = Find(reload, key);
        const int inF = InWorld(f), inL = InWorld(l), inR = InWorld(r);
        if (inL != inR)
        {
            ReloadMismatch m{ key, "presence", Presence(f), Presence(l), Presence(r), "", inL, inR,
                l != nullptr && CaneBreaks(l->sweep) };
            for (const ObjSnap* o : { f, l, r })
                m.breakable = m.breakable || (o != nullptr && o->broken >= 0);
            if (inR < inL)
                m.story = inF == inL
                    ? "live play never removed it, the reload removes it"
                    : "the reload removes more of it than live play did";
            else if (m.breakable)
                m.story = "broken live, it comes back on reload";
            else
                m.story = inF == inR
                    ? "live play removed it, it comes back on reload"
                    : "the reload has more of it than live play";
            if (inR < inL && m.breakable && l != nullptr && l->sweep >= 0 && !CaneBreaks(l->sweep))
                m.story += " (live response cannot break it)";
            mismatches.push_back(m);
            continue;
        }
        if (inL == 0)
            continue; // gone in both: nothing else about it matters
        const bool liveBreaks = CaneBreaks(l->sweep);
        if (l->sweep != r->sweep)
            mismatches.push_back({ key, "response", f != nullptr ? Text(f->sweep) : "-", Text(l->sweep), Text(r->sweep),
                liveBreaks != CaneBreaks(r->sweep) ? "breakable in one, not in the other" : "response differs", inL, inR,
                liveBreaks });
        if (l->broken != r->broken)
            mismatches.push_back({ key, "broken", f != nullptr ? Text(f->broken) : "-", Text(l->broken), Text(r->broken),
                "broken flag differs", inL, inR, liveBreaks });
        if (l->sm != r->sm)
            mismatches.push_back({ key, "sm", f != nullptr ? f->sm : "-", l->sm, r->sm,
                f != nullptr && f->sm == r->sm ? "the reload is back at the fresh state" : "state differs", inL, inR,
                liveBreaks });
    }
    std::set<std::string> flags;
    for (const Snapshot* s : { &live, &reload })
        for (const auto& [key, value] : s->flags)
            flags.insert(key);
    for (const std::string& key : flags)
    {
        auto value = [&](const Snapshot& s) { const auto it = s.flags.find(key); return it == s.flags.end() ? std::string("-") : it->second; };
        if (value(live) != value(reload))
            mismatches.push_back({ key, "flag", value(fresh), value(live), value(reload),
                value(fresh) == value(reload) ? "the save lost a live change" : "flag differs", 0, 0, false });
    }
    return mismatches;
}

nlohmann::json ToJson(const ReloadMismatch& m)
{
    return { { "key", m.key }, { "field", m.field }, { "fresh", m.fresh }, { "live", m.live }, { "reload", m.reload },
        { "story", m.story } };
}

bool ReloadMismatchFails(const ReloadMismatch& m, const ReloadContext& context)
{
    if (context.advisoryKeys.count(m.key) != 0)
        return false;
    if (m.field == "flag")
    {
        // Kept by the save, set back by the level's own load.
        const auto it = context.savedFlags.find(m.key);
        return it == context.savedFlags.end() || it->second != m.live;
    }
    if (m.field == "sm")
    {
        const auto it = context.reapproached.find(m.key);
        return it != context.reapproached.end() && it->second != m.live;
    }
    const bool driven = context.drivenKeys.count(m.key) != 0;
    if (m.field == "presence" && m.reloadIn < m.liveIn)
    {
        // Present live, gone after the reload. A breakable still breakable
        // live is a pending break (the swing did not reach it). Otherwise it
        // fails for an object the sweep drove (live play missed what the
        // load path does) and for a breakable the live response cannot break
        // (the user's fuse, whose response the unlock never changed);
        // anything else is advisory: the reload shows the end state of what
        // live play started (an opened lock group).
        if (m.breakable && m.liveBreaks)
            return false;
        return driven || m.breakable;
    }
    if (m.field == "presence" && m.reloadIn > m.liveIn)
    {
        // Gone live, back after the reload: the save lost it. Advisory only
        // for a breakable the sweep did not drive (broken crates are not
        // saved).
        return driven || !m.breakable;
    }
    if ((m.field == "response" || m.field == "broken") && !driven)
        return false;
    return true;
}

// --- known findings ---------------------------------------------------------------

std::vector<KnownFinding> KnownFindingsFromJson(const nlohmann::json& json)
{
    std::vector<KnownFinding> list;
    const nlohmann::json entries = json.is_object() ? json.value("findings", nlohmann::json::array()) : json;
    for (const nlohmann::json& entry : entries)
        list.push_back({ entry.value("check", std::string()), entry.value("level", std::string()),
            entry.value("key", std::string()), entry.value("field", std::string()), entry.value("live", std::string()),
            entry.value("reload", std::string()), entry.value("reason", std::string()), entry.value("writeup", std::string()) });
    return list;
}

std::vector<std::string> KnownFindingProblems(const nlohmann::json& json)
{
    std::vector<std::string> problems;
    if (!json.is_object() || !json.contains("findings") || !json["findings"].is_array())
        return { "no \"findings\" list" };
    size_t i = 0;
    for (const nlohmann::json& entry : json["findings"])
    {
        const std::string where = "finding " + std::to_string(i++);
        if (!entry.is_object())
        {
            problems.push_back(where + ": not an object");
            continue;
        }
        for (const char* field : { "check", "level", "writeup" })
            if (entry.value(field, std::string()).empty())
                problems.push_back(where + ": no \"" + field + "\"");
        const bool reload = !entry.value("key", std::string()).empty() && !entry.value("field", std::string()).empty() &&
            entry.contains("live") && entry.contains("reload");
        if (!reload && entry.value("reason", std::string()).empty())
            problems.push_back(where + ": needs key, field, live and reload, or reason");
    }
    return problems;
}

namespace
{
// The failure reason a known live/reload entry stands for, up to its story.
std::string KnownReason(const KnownFinding& k)
{
    return k.reason.empty()
        ? "live vs reload: " + k.key + " " + k.field + ": live " + k.live + ", reload " + k.reload
        : k.reason;
}
}

std::string KnownWriteup(const std::vector<KnownFinding>& known, const std::string& check, const std::string& level,
    const std::string& reason, std::set<size_t>* used)
{
    for (size_t i = 0; i < known.size(); ++i)
    {
        const KnownFinding& k = known[i];
        if (k.check != check || k.level != level)
            continue;
        const std::string want = KnownReason(k);
        // Anchored: the whole reason, or the whole live/reload part followed
        // by the story in parentheses.
        const bool match = reason == want || (k.reason.empty() && reason.rfind(want + " (", 0) == 0);
        if (!match)
            continue;
        if (used != nullptr)
            used->insert(i);
        return k.writeup;
    }
    return {};
}

std::vector<std::string> StaleKnown(const std::vector<KnownFinding>& known, const std::string& check,
    const std::vector<std::string>& ran, const std::set<size_t>& used)
{
    std::vector<std::string> stale;
    for (size_t i = 0; i < known.size(); ++i)
        if (known[i].check == check && used.count(i) == 0 &&
            std::find(ran.begin(), ran.end(), known[i].level) != ran.end())
            stale.push_back(known[i].level + ": " + KnownReason(known[i]) + " (" + known[i].writeup + ") matched nothing");
    return stale;
}

std::vector<std::string> MustHappenProblems(const nlohmann::json& json, const std::vector<std::string>& levels)
{
    const nlohmann::json* entries = &json;
    if (json.is_object())
    {
        if (!json.contains("events"))
            return { "no \"events\" list" };
        entries = &json["events"];
    }
    if (!entries->is_array() || entries->empty())
        return { "the \"events\" list is empty" };
    std::vector<std::string> problems;
    size_t i = 0;
    for (const nlohmann::json& entry : *entries)
    {
        const std::string where = "event " + std::to_string(i++);
        if (!entry.is_object())
        {
            problems.push_back(where + ": not an object");
            continue;
        }
        const std::string level = entry.value("level", std::string());
        if (level.empty() || entry.value("kind", std::string()).empty() || entry.value("key", std::string()).empty())
            problems.push_back(where + ": needs level, kind and key");
        else if (std::find(levels.begin(), levels.end(), level) == levels.end())
            problems.push_back(where + ": '" + level + "' is not one of the game's levels");
    }
    return problems;
}

std::vector<std::string> UnevaluatedMustHappen(const std::vector<MustHappen>& list, const std::vector<std::string>& ran)
{
    std::vector<std::string> left;
    for (const MustHappen& item : list)
        if (std::find(ran.begin(), ran.end(), item.level) == ran.end())
            left.push_back(item.level + ": " + item.kind + " " + item.key + " was not evaluated (the level did not run or load)");
    return left;
}
}
