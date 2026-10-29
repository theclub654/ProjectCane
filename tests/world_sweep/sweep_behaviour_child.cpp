// Behaviour child modes of the world sweep: they watch what the game does,
// frame by frame, by reading its state after every RunGameFrame (no game
// code is changed or wrapped here). sweep_behaviour.cpp judges the records.
//
// behaviour (checks 1 and 2): every level in the list plays with no input
// for --frames frames. Each frame the observer reads the applied animation
// segments (SW::dlAsega, dlAsegaRealClock) and their channels' actions
// (ASEGA::dlActseg: the object each channel drives, where its curve wants
// it and where it is), the state machines (SW::dlSma), which objects are in
// the world, every object's custom options (its sidebag), every object's
// cane response (FICG) and the world and level state (WS/LS). It writes the
// structural trace to --traces/<level>.json and a result line with the
// motion verdicts.
//
// unlock (checks 3 and 4): every level in the list is loaded and settles;
// then the in-session changes the sweep can drive through the game's own
// paths are made: each lock group (LOCKG: the hub doors, the fuse breaker)
// gets the world's keys, and once it waits for Sly (state 558) and he
// stands, TriggerLockg runs (what the group's volume calls when he walks
// in) so Sly plays the unlock himself; a group parked until he comes near
// gets him put beside it first, and the goals the unlock sets are the last
// resort ("forced"). Sly stands in each volume button (switches, mission
// triggers), each clue is collected by touch, and each object whose
// cane response became breakable is swung at with the square button (from
// where Sly stands, then from four sides). Sly is moved by teleporting, so
// a fall of more than 400 units puts him back where he last stood (the
// checkpoint reload that would revive him is held back). The persisted-
// relevant state is snapshot at the fresh load and after the changes; then
// the game is saved and loaded (SaveCurrentGameToDisk / LoadCurrentSave),
// the level is loaded again, Sly walks back up to each group that was open
// live, and the state is snapshot a third time.
//
// unlock --scenario fuse: the user's sequence in Tide of Terror. Two keys
// from earlier levels, the third collected in A Cunning Disguise (by touch,
// else through the key's own SetKeyDprizes collection), back to Prowling
// the Grounds through the level's own exit, then the breaker (LOCKG 1843)
// is unlocked from where Sly arrives and he swings at the fuse (BREAK 1487).
//
// What polling cannot see: anything that starts and ends inside a single
// RunGameFrame (a segment applied and retracted in one frame, an object
// added and removed again) and load-time changes made before the first
// frame.

#include "sweep_child.h"
#include "sweep_behaviour.h"

#include "../../Sly1-Release/acp.h"
#include "../../Sly1-Release/acr.h"
#include "../../Sly1-Release/actseg.h"
#include "../../Sly1-Release/aseg.h"
#include "../../Sly1-Release/asega.h"
#include "../../Sly1-Release/break.h"
#include "../../Sly1-Release/button.h"
#include "../../Sly1-Release/coin.h"
#include "../../Sly1-Release/jt.h"
#include "../../Sly1-Release/pzo.h"
#include "../../Sly1-Release/save.h"
#include "../../Sly1-Release/sidebag.h"
#include "../../Sly1-Release/sm.h"
#include "../../Sly1-Release/sw.h"
#include "../../Sly1-Release/transition.h"
#include "../../Sly1-Release/xform.h"

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>

namespace sweep_child
{
namespace
{
namespace fs = std::filesystem;

std::string Name(const LO* plo)
{
    return plo != nullptr && plo->pchzName != nullptr ? plo->pchzName : "";
}

int OidOf(const LO* plo)
{
    return plo != nullptr ? static_cast<int>(plo->oid) : -1;
}

int CidOf(const BASIC* pbasic)
{
    return pbasic != nullptr && pbasic->pvtbasic != nullptr ? static_cast<int>(pbasic->pvtbasic->cid) : -1;
}

std::string ObjectKey(const LO* plo)
{
    const std::string name = Name(plo);
    return std::to_string(OidOf(plo)) + ":" + std::to_string(CidOf(plo)) + (name.empty() ? "" : " '" + name + "'");
}

std::string Hex(unsigned value)
{
    char text[16];
    std::snprintf(text, sizeof(text), "0x%x", value);
    return text;
}

std::string RefText(const CRef& ref)
{
    char text[64];
    switch (ref.m_tagk)
    {
    case TAGK_S32: std::snprintf(text, sizeof(text), "%d", static_cast<int>(ref.m_n)); break;
    case TAGK_F32: std::snprintf(text, sizeof(text), "%g", static_cast<double>(ref.m_g)); break;
    case TAGK_Bool: return ref.m_bool ? "#t" : "#f";
    case TAGK_Symid: std::snprintf(text, sizeof(text), "sym%u", static_cast<unsigned>(ref.m_symid)); break;
    case TAGK_Basic:
        if (ref.m_pbasic != nullptr && ref.m_pbasic->pvtbasic != nullptr && FIsBasicDerivedFrom(ref.m_pbasic, CID_LO))
            std::snprintf(text, sizeof(text), "<%d>", OidOf(reinterpret_cast<const LO*>(ref.m_pbasic)));
        else
            std::snprintf(text, sizeof(text), "<basic>");
        break;
    default: std::snprintf(text, sizeof(text), "<tag%d>", static_cast<int>(ref.m_tagk)); break;
    }
    return text;
}

template <typename T, typename F>
void ForEachDl(const DL& dl, DLE T::*link, F&& fn)
{
    for (T* p = static_cast<T*>(dl.pvFirst); p != nullptr; p = static_cast<T*>((p->*link).pvNext))
        fn(p);
}

std::array<float, 3> Arr(const glm::vec3& v)
{
    return { v.x, v.y, v.z };
}

std::array<float, 9> Arr(const glm::mat3& m)
{
    return { m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2] };
}

// --- the observer ---------------------------------------------------------------

struct Observer
{
    sweep::Trace trace;
    sweep::MotionThresholds thresholds;
    int stallFrames = 120;
    bool recordMotion = true;

    // Results.
    std::vector<nlohmann::json> staticTargets;
    std::vector<nlohmann::json> unresolvedSegments;
    std::vector<nlohmann::json> stalledTransitions;
    int tracksJudged = 0;
    int segmentsSeen = 0;
    std::map<std::string, int> verdicts;

    using AsegaKey = std::tuple<const ASEGA*, const ASEG*, const ALO*>;
    using ActKey = std::tuple<const ACTSEG*, const ASEGA*, const ALO*, int>;
    struct LiveSegment { std::string key; sweep::SegmentTrack track; nlohmann::json channels; };
    struct SmaState { int sm = -1; int root = -1; int cur = -1; int next = -1; };

    bool first = true;
    std::map<AsegaKey, LiveSegment> segments;
    std::map<ActKey, sweep::ActTrack> acts;
    // Channels with a blend curve, not judged (see PollSegments).
    std::set<ActKey> blendChannels;
    std::map<const SMA*, SmaState> smas;
    std::map<const SMA*, sweep::TransitionTrack> transitions;
    std::unordered_map<const ALO*, int> responseValues;
    std::map<std::string, std::string> state;
    bool transitionSeen = false;

    void Event(int frame, const char* kind, const std::string& key)
    {
        trace.events.push_back({ frame, kind, key });
    }

    void FinishAct(const sweep::ActTrack& track)
    {
        const sweep::MotionVerdict verdict = sweep::JudgeActTrack(track, thresholds);
        ++verdicts[sweep::MotionVerdictName(verdict)];
        ++tracksJudged;
        if (verdict == sweep::MotionVerdict::Static)
            staticTargets.push_back({ { "track", track.label }, { "first_frame", track.FirstFrame() },
                { "last_frame", track.LastFrame() }, { "frames", track.Frames() },
                { "goal_pos_travel", track.GoalPosTravel() }, { "goal_rot_travel", track.GoalRotTravel() },
                { "pos_travel", track.PosTravel() }, { "rot_travel", track.RotTravel() },
                { "t_advance", track.TAdvance() } });
    }

    void FinishSegment(const LiveSegment& segment)
    {
        ++segmentsSeen;
        if (sweep::SegmentUnresolved(segment.track, thresholds))
            unresolvedSegments.push_back({ { "segment", segment.track.label }, { "frames", segment.track.frames },
                { "curve_channels", segment.track.channelsWithCurves },
                { "t_advance", segment.track.tMax - segment.track.tMin }, { "channels", segment.channels } });
    }

    void FinishTransition(const sweep::TransitionTrack& track)
    {
        if (sweep::TransitionStalled(track, stallFrames))
            stalledTransitions.push_back({ { "transition", track.label }, { "frames", track.frames },
                { "stalled_frames", track.stalledFrames } });
    }

    void PollSegments(int frame)
    {
        std::set<AsegaKey> seen;
        std::set<ActKey> actsSeen;
        auto visit = [&](ASEGA* pasega)
        {
            const AsegaKey key{ pasega, pasega->paseg, pasega->paloRoot };
            seen.insert(key);
            ASEG* paseg = pasega->paseg;
            const std::string label = std::to_string(OidOf(paseg)) + "@" + std::to_string(OidOf(pasega->paloRoot));
            auto it = segments.find(key);
            if (it == segments.end())
            {
                it = segments.emplace(key, LiveSegment{ label, {} }).first;
                it->second.track.label = "aseg " + std::to_string(OidOf(paseg)) + " root " +
                    std::to_string(OidOf(pasega->paloRoot)) + (Name(pasega->paloRoot).empty() ? "" : " '" + Name(pasega->paloRoot) + "'");
                if (!first)
                    Event(frame, "aseg+", label);
            }
            if (!recordMotion || paseg == nullptr)
                return;
            int curveChannels = 0;
            for (const CHN& chn : paseg->achn)
                curveChannels += (chn.pacp != nullptr || chn.pacr != nullptr) ? 1 : 0;
            if (it->second.track.frames == 0 && curveChannels > 0)
            {
                // What each curve channel names, for a person reading the
                // report: is there such an object at all, and where.
                for (const CHN& chn : paseg->achn)
                {
                    if (chn.pacp == nullptr && chn.pacr == nullptr)
                        continue;
                    int found = 0, inWorldCount = 0, parent = -1, proxyRoot = -1;
                    for (const std::shared_ptr<LO>& sp : allWorldObjs)
                        if (sp != nullptr && sp->oid == chn.oid)
                        {
                            if (found++ == 0)
                            {
                                parent = OidOf(sp->paloParent);
                                if (sp->ppxr != nullptr)
                                    proxyRoot = static_cast<int>(sp->ppxr->oidProxyRoot);
                            }
                            inWorldCount += FIsLoInWorld(sp.get()) != 0 ? 1 : 0;
                        }
                    it->second.channels.push_back({ { "oid", static_cast<int>(chn.oid) }, { "objects", found },
                        { "in_world", inWorldCount }, { "first_parent", parent }, { "proxy_root", proxyRoot },
                        { "is_root", pasega->paloRoot != nullptr && chn.oid == pasega->paloRoot->oid } });
                }
            }
            int actsegs = 0;
            ForEachDl<ACTSEG>(pasega->dlActseg, &ACTSEG::dleAsega, [&](ACTSEG* pactseg)
            {
                ++actsegs;
                ALO* palo = pactseg->palo;
                if (palo == nullptr || pactseg->ichn < 0 || pactseg->ichn >= static_cast<int>(paseg->achn.size()))
                    return;
                const CHN& chn = paseg->achn[pactseg->ichn];
                if (chn.pacp == nullptr && chn.pacr == nullptr)
                    return;
                sweep::ActSample sample;
                sample.tLocal = pasega->tLocal;
                sample.updated = FIsLoInWorld(palo) != 0 && palo->fFrozen == 0;
                sample.pos = Arr(palo->xf.pos);
                sample.rot = Arr(palo->xf.mat);
                // The channel's curves, evaluated without the object (no
                // AdjustAloPosition/Rotation call, so nothing in the game
                // changes): where the segment wants the object now. Blend
                // curves read the object's original pose (posOrig/eulOrig,
                // acpbl.cpp/acrbl.cpp), so without it their goal is wrong:
                // those channels are not judged, only counted.
                const bool blendPos = chn.pacp != nullptr && chn.pacp->acvk == ACVK_Blend;
                const bool blendRot = chn.pacr != nullptr && chn.pacr->acvk == ACVK_Blend;
                const ActKey actKeyBlend{ pactseg, pasega, palo, pactseg->ichn };
                if (blendPos || blendRot)
                    blendChannels.insert(actKeyBlend);
                if (!blendPos && chn.pacp != nullptr && chn.pacp->pvtacp != nullptr && chn.pacp->pvtacp->pfnEvaluateAcp != nullptr)
                {
                    glm::vec3 goal(0.0f);
                    chn.pacp->pvtacp->pfnEvaluateAcp(chn.pacp.get(), nullptr, pasega->tLocal, pasega->svtLocal, 0, &goal, nullptr);
                    sample.goalPos = Arr(goal);
                    sample.hasPosCurve = true;
                    sample.drivesPos = palo->pactPos == static_cast<ACT*>(pactseg);
                }
                if (!blendRot && chn.pacr != nullptr && chn.pacr->pvtacr != nullptr && chn.pacr->pvtacr->pfnEvaluateAcr != nullptr)
                {
                    glm::mat3 goal(1.0f);
                    chn.pacr->pvtacr->pfnEvaluateAcr(chn.pacr.get(), nullptr, pasega->tLocal, pasega->svtLocal, 0, &goal, nullptr);
                    sample.goalRot = Arr(goal);
                    sample.hasRotCurve = true;
                    sample.drivesRot = palo->pactRot == static_cast<ACT*>(pactseg);
                }
                const ActKey actKey{ pactseg, pasega, palo, pactseg->ichn };
                actsSeen.insert(actKey);
                auto track = acts.find(actKey);
                if (track == acts.end())
                {
                    track = acts.emplace(actKey, sweep::ActTrack{}).first;
                    track->second.label = it->second.track.label + " target " + std::to_string(OidOf(palo)) +
                        (Name(palo).empty() ? "" : " '" + Name(palo) + "'") + " chn " + std::to_string(pactseg->ichn);
                }
                track->second.Add(frame, sample);
            });
            it->second.track.Add(pasega->tLocal, curveChannels, actsegs);
        };
        if (g_psw != nullptr)
        {
            ForEachDl<ASEGA>(g_psw->dlAsega, &ASEGA::dleSw, visit);
            ForEachDl<ASEGA>(g_psw->dlAsegaRealClock, &ASEGA::dleSw, visit);
        }
        for (auto it = segments.begin(); it != segments.end();)
        {
            if (seen.count(it->first) != 0)
            {
                ++it;
                continue;
            }
            Event(frame, "aseg-", it->second.key);
            FinishSegment(it->second);
            it = segments.erase(it);
        }
        // An action ends with its segment, or when the segment drops the
        // channel (retracted actions leave dlActseg).
        for (auto it = acts.begin(); it != acts.end();)
        {
            if (actsSeen.count(it->first) != 0)
            {
                ++it;
                continue;
            }
            FinishAct(it->second);
            it = acts.erase(it);
        }
    }

    void PollSmas(int frame)
    {
        std::set<const SMA*> seen;
        if (g_psw != nullptr)
            ForEachDl<SMA>(g_psw->dlSma, &SMA::dleSw, [&](SMA* psma)
            {
                seen.insert(psma);
                const SM* psm = psma->psm;
                auto stateOid = [&](int isms) {
                    return psm != nullptr && isms >= 0 && isms < static_cast<int>(psm->asms.size()) ? static_cast<int>(psm->asms[isms].oid) : -1;
                };
                SmaState now{ OidOf(psm), OidOf(psma->paloRoot), stateOid(psma->ismsCur), stateOid(psma->ismsNext) };
                const std::string id = std::to_string(now.sm) + "@" + std::to_string(now.root);
                auto it = smas.find(psma);
                if (it == smas.end())
                {
                    if (!first)
                        Event(frame, "sma+", id + ":" + std::to_string(now.cur));
                    smas[psma] = now;
                }
                else
                {
                    if (it->second.cur != now.cur)
                        Event(frame, "sm", id + ":" + std::to_string(it->second.cur) + ">" + std::to_string(now.cur));
                    it->second = now;
                }
                // The transition animation of a state machine on its way to
                // the next state.
                if (recordMotion && psma->ismsNext >= 0 && psma->pasegaCur != nullptr && psma->paloRoot != nullptr &&
                    FIsLoInWorld(psma->paloRoot) != 0)
                {
                    sweep::TransitionTrack& track = transitions[psma];
                    if (track.frames == 0)
                        track.label = "sm " + std::to_string(now.sm) + " root " + std::to_string(now.root) + " " +
                            std::to_string(now.cur) + ">" + std::to_string(now.next) + " aseg " +
                            std::to_string(OidOf(psma->pasegaCur->paseg));
                    // UpdateAsega leaves a segment with a frozen target (the
                    // game freezes objects it does not need to update) where
                    // it is; that is not a stall.
                    bool frozen = false;
                    ForEachDl<ACTSEG>(psma->pasegaCur->dlActseg, &ACTSEG::dleAsega,
                        [&](ACTSEG* pactseg) { frozen |= pactseg->palo != nullptr && pactseg->palo->fFrozen != 0; });
                    const bool running = !frozen && psma->pasegaCur->svtLocal != 0.0f && psma->pasegaCur->svtMaster != 0.0f &&
                        psma->pasegaCur->dtPause <= 0.0f;
                    track.Add(psma->pasegaCur->tLocal, running);
                }
                else if (auto t = transitions.find(psma); t != transitions.end())
                {
                    FinishTransition(t->second);
                    transitions.erase(t);
                }
            });
        for (auto it = smas.begin(); it != smas.end();)
        {
            if (seen.count(it->first) != 0)
            {
                ++it;
                continue;
            }
            Event(frame, "sma-", std::to_string(it->second.sm) + "@" + std::to_string(it->second.root) + ":" +
                std::to_string(it->second.cur));
            if (auto t = transitions.find(it->first); t != transitions.end())
            {
                FinishTransition(t->second);
                transitions.erase(t);
            }
            it = smas.erase(it);
        }
    }

    // The value of one custom option, compared without formatting it.
    struct OptValue
    {
        int tag = 0, n = 0, b = 0;
        float g = 0.0f;
        unsigned sym = 0;
        const void* p = nullptr;
        bool operator==(const OptValue& o) const
        {
            return tag == o.tag && n == o.n && b == o.b && g == o.g && sym == o.sym && p == o.p;
        }
    };
    struct PairHash
    {
        size_t operator()(const std::pair<const LO*, int>& k) const
        {
            return std::hash<const void*>()(k.first) ^ (static_cast<size_t>(k.second) * 0x9e3779b97f4a7c15ull);
        }
    };
    std::unordered_map<std::pair<const LO*, int>, OptValue, PairHash> optionValues;
    std::unordered_map<uint64_t, int> worldCounts, worldCountsNow;

    static uint64_t OidCid(const LO* plo)
    {
        return (static_cast<uint64_t>(static_cast<uint32_t>(OidOf(plo))) << 32) | static_cast<uint32_t>(CidOf(plo));
    }

    static std::string OidCidText(uint64_t key)
    {
        return std::to_string(static_cast<int>(key >> 32)) + ":" + std::to_string(static_cast<int>(key & 0xffffffffu));
    }

    void PollObjects(int frame)
    {
        worldCountsNow.clear();
        for (const std::shared_ptr<LO>& sp : allWorldObjs)
        {
            LO* plo = sp.get();
            if (plo == nullptr || plo->pvtbasic == nullptr)
                continue;
            if (FIsLoInWorld(plo) != 0)
                ++worldCountsNow[OidCid(plo)];
            if (const CSidebag* psidebag = plo->psidebag.get())
                for (int i = 0; i < psidebag->m_csbb && i < static_cast<int>(std::size(psidebag->m_asbb)); ++i)
                {
                    const CRef& ref = psidebag->m_asbb[i].ref;
                    OptValue value;
                    value.tag = static_cast<int>(ref.m_tagk);
                    value.n = ref.m_n;
                    value.b = ref.m_bool;
                    value.g = ref.m_g;
                    value.sym = static_cast<unsigned>(ref.m_symid);
                    value.p = ref.m_pbasic;
                    const std::pair<const LO*, int> key{ plo, psidebag->m_asbb[i].n };
                    const auto [it, inserted] = optionValues.try_emplace(key, value);
                    if (!inserted && it->second == value)
                        continue;
                    it->second = value;
                    if (!first)
                        Event(frame, "opt", std::to_string(OidOf(plo)) + ":" + std::to_string(key.second) + "=" + RefText(ref));
                }
            if (FIsBasicDerivedFrom(plo, CID_ALO))
            {
                const ALO* palo = static_cast<const ALO*>(plo);
                const int sweepResponse = palo->ficg.grficSweep;
                const auto [it, inserted] = responseValues.try_emplace(palo, sweepResponse);
                if (!inserted && it->second != sweepResponse)
                {
                    if (!first)
                        Event(frame, "ficg", std::to_string(OidOf(plo)) + ":" + std::to_string(it->second) + ">" +
                            std::to_string(sweepResponse));
                    it->second = sweepResponse;
                }
            }
        }
        if (!first)
        {
            for (const auto& [key, count] : worldCountsNow)
            {
                const auto old = worldCounts.find(key);
                if (count > (old == worldCounts.end() ? 0 : old->second))
                    Event(frame, "lo+", OidCidText(key));
            }
            for (const auto& [key, count] : worldCounts)
            {
                const auto cur = worldCountsNow.find(key);
                if ((cur == worldCountsNow.end() ? 0 : cur->second) < count)
                    Event(frame, "lo-", OidCidText(key));
            }
        }
        worldCounts.swap(worldCountsNow);
    }

    void PollState(int frame)
    {
        std::map<std::string, std::string> now;
        if (g_pwsCur != nullptr)
        {
            now["fws"] = Hex(static_cast<unsigned>(g_pwsCur->fws));
            now["ckey"] = std::to_string(g_pwsCur->ckey);
            now["cvault"] = std::to_string(g_pwsCur->cvault);
        }
        if (g_plsCur != nullptr)
        {
            now["grfls"] = Hex(static_cast<unsigned>(g_plsCur->grfls));
            now["clues"] = std::to_string(g_plsCur->abitClue.m_cbitSet);
            for (const LEVELDATA& data : g_plsCur->alevelData)
                if (data.key != 0)
                    now["ldata " + std::to_string(data.key)] = std::to_string(data.value);
        }
        for (const auto& [key, value] : now)
        {
            const auto old = state.find(key);
            if (!first && (old == state.end() || old->second != value))
                Event(frame, "state", key + "=" + value);
        }
        state.swap(now);
        const bool pending = g_transition.m_fPending != 0;
        if (pending && !transitionSeen)
            Event(frame, "transition", g_transition.m_worldPending);
        transitionSeen = pending;
    }

    void Poll(int frame)
    {
        PollSegments(frame);
        PollSmas(frame);
        PollObjects(frame);
        PollState(frame);
        first = false;
        trace.frames = frame + 1;
    }

    // Judges whatever is still running at the end of the run.
    void Finish()
    {
        for (const auto& [key, track] : acts)
            FinishAct(track);
        acts.clear();
        for (const auto& [key, segment] : segments)
            FinishSegment(segment);
        segments.clear();
        for (const auto& [psma, track] : transitions)
            FinishTransition(track);
        transitions.clear();
    }

    // Starts again for the next level (the trace and results stay).
    void ResetLive()
    {
        first = true;
        segments.clear();
        acts.clear();
        smas.clear();
        transitions.clear();
        worldCounts.clear();
        optionValues.clear();
        responseValues.clear();
        state.clear();
        transitionSeen = false;
    }
};

bool LoadLevel(Stepper& stepper, sweep::LevelResult& result, const std::string& name)
{
    LEVELINFO* plevel = PlevelinfoFromLevelName(name);
    result.level = name;
    stepper.result = &result;
    stepper.plevel = plevel;
    if (plevel == nullptr)
    {
        result.note = "not in g_levelTable";
        return false;
    }
    g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
    result.loadMs = stepper.Step(-1);
    result.loaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
    return result.loaded;
}

std::vector<std::string> ReadList(const fs::path& path)
{
    std::vector<std::string> names;
    std::ifstream list(path);
    for (std::string line; std::getline(list, line);)
        if (!line.empty())
            names.push_back(line);
    return names;
}

void BeginLevel(const std::string& name)
{
    std::fprintf(stderr, "=== LEVEL %s ===\n", name.c_str());
    std::printf("=== LEVEL %s ===\n", name.c_str());
    std::fflush(stderr);
}

nlohmann::json Head(nlohmann::json list, size_t limit)
{
    if (list.size() > limit)
        list.erase(list.begin() + static_cast<long>(limit), list.end());
    return list;
}

// --- snapshots and driving (unlock) ----------------------------------------------

bool Relevant(LO* plo)
{
    const int cid = CidOf(plo);
    return FIsBasicDerivedFrom(plo, CID_BRK) || cid == CID_CLUE || cid == CID_KEY || cid == CID_LOCK ||
        cid == CID_LOCKG || cid == CID_VAULT;
}

// The current world's and level's saved state (WS/LS inside g_pgsCur).
std::map<std::string, std::string> Flags()
{
    std::map<std::string, std::string> flags;
    if (g_pwsCur != nullptr)
    {
        flags["fws"] = Hex(static_cast<unsigned>(g_pwsCur->fws));
        flags["ckey"] = std::to_string(g_pwsCur->ckey);
        flags["cvault"] = std::to_string(g_pwsCur->cvault);
    }
    if (g_plsCur != nullptr)
    {
        flags["grfls"] = Hex(static_cast<unsigned>(g_plsCur->grfls));
        flags["clues"] = std::to_string(g_plsCur->abitClue.m_cbitSet);
        for (const LEVELDATA& data : g_plsCur->alevelData)
            if (data.key != 0)
                flags["ldata " + std::to_string(data.key)] = std::to_string(data.value);
    }
    return flags;
}

sweep::Snapshot Snap(const std::set<std::string>& alsoKeys)
{
    sweep::Snapshot snapshot;
    std::map<const LO*, std::string> machines;
    if (g_psw != nullptr)
        ForEachDl<SMA>(g_psw->dlSma, &SMA::dleSw, [&](SMA* psma)
        {
            const SM* psm = psma->psm;
            if (psm == nullptr || psma->paloRoot == nullptr)
                return;
            const int cur = psma->ismsCur >= 0 && psma->ismsCur < static_cast<int>(psm->asms.size())
                ? static_cast<int>(psm->asms[psma->ismsCur].oid) : -1;
            std::string& text = machines[psma->paloRoot];
            text += (text.empty() ? "" : ",") + std::to_string(OidOf(psm)) + ":" + std::to_string(cur);
        });
    for (const std::shared_ptr<LO>& sp : allWorldObjs)
    {
        LO* plo = sp.get();
        if (plo == nullptr || plo->pvtbasic == nullptr)
            continue;
        const std::string key = ObjectKey(plo);
        if (!Relevant(plo) && alsoKeys.count(key) == 0)
            continue;
        sweep::ObjSnap& o = snapshot.objects[key];
        ++o.count;
        if (FIsLoInWorld(plo) == 0)
            continue;
        if (o.inWorld++ == 0)
        {
            if (FIsBasicDerivedFrom(plo, CID_ALO))
                o.sweep = static_cast<ALO*>(plo)->ficg.grficSweep;
            if (FIsBasicDerivedFrom(plo, CID_BRK))
                o.broken = static_cast<BRK*>(plo)->fBroken;
            const auto m = machines.find(plo);
            if (m != machines.end())
                o.sm = m->second;
        }
    }
    snapshot.flags = Flags();
    return snapshot;
}

template <typename T>
std::vector<T*> InWorldOfClass(CID cid)
{
    std::vector<T*> list;
    for (const std::shared_ptr<LO>& sp : allWorldObjs)
        if (sp != nullptr && sp->pvtbasic != nullptr && CidOf(sp.get()) == cid && FIsLoInWorld(sp.get()) != 0)
            list.push_back(static_cast<T*>(static_cast<ALO*>(sp.get())));
    return list;
}

std::map<ALO*, int> Responses()
{
    std::map<ALO*, int> map;
    for (ALO* palo : allSWAloObjs)
        if (palo != nullptr)
            map[palo] = palo->ficg.grficSweep;
    return map;
}

bool Gone(ALO* palo)
{
    if (FIsLoInWorld(palo) == 0)
        return true;
    return FIsBasicDerivedFrom(palo, CID_BRK) && static_cast<BRK*>(palo)->fBroken != 0;
}

// A check of the motion check itself (--freeze-root): every object that a
// segment rooted on this oid drives is put back where it was first seen
// after each frame, before the observer looks, so the segment plays and its
// targets never move: "applied, target static" by construction.
struct Freezer
{
    int root = -1;
    std::map<ALO*, std::pair<glm::vec3, glm::mat3>> held;

    void Apply()
    {
        if (root < 0 || g_psw == nullptr)
            return;
        ForEachDl<ASEGA>(g_psw->dlAsega, &ASEGA::dleSw, [&](ASEGA* pasega)
        {
            if (OidOf(pasega->paloRoot) != root)
                return;
            ForEachDl<ACTSEG>(pasega->dlActseg, &ACTSEG::dleAsega, [&](ACTSEG* pactseg)
            {
                ALO* palo = pactseg->palo;
                if (palo == nullptr || (palo->pactPos != static_cast<ACT*>(pactseg) && palo->pactRot != static_cast<ACT*>(pactseg)))
                    return;
                const auto it = held.emplace(palo, std::make_pair(palo->xf.pos, palo->xf.mat)).first;
                palo->xf.pos = it->second.first;
                palo->xf.mat = it->second.second;
            });
        });
    }
};

// A swing counts as one at the target only from a side placement (70 units
// off and 40 below its centre) with Sly standing no farther than this from
// it after he lands. He lands 125-155 units away (the fuse, which breaks
// from there), and the in-place swings are hundreds of units off.
constexpr float kSwingReach = 160.0f;

struct Driver
{
    Stepper& stepper;
    Observer* observer = nullptr;
    int frame = 0;
    Freezer* freezer = nullptr;
    int conclusiveSwings = 0; // swings of the last SwingAt that count
    // Unlock mode moves Sly around by teleporting; a spot that gives way
    // under him (a breaker that lowers) drops him into a kill volume, and
    // the checkpoint reload that would revive him is held back. With the
    // guard on, Sly falling 400 units below where he last stood is put back
    // there before he lands.
    bool fallGuard = false;
    bool haveSafe = false;
    glm::vec3 safe{ 0.0f };
    int fallsCaught = 0;

    // Armed by the first teleport, so a level's own opening (Sly jumping
    // out of the van) is left alone.
    bool guardArmed = false;

    void GuardFall()
    {
        if (!fallGuard || !guardArmed || g_pjt == nullptr || g_pjt->jts == JTS_Zap)
            return;
        if (g_pjt->jts == JTS_Stand)
        {
            safe = g_pjt->xf.posWorld;
            haveSafe = true;
        }
        else if (haveSafe && g_pjt->xf.posWorld.z < safe.z - 400.0f)
        {
            TeleportSly(safe);
            ++fallsCaught;
        }
    }

    void Step(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            stepper.Step(frame);
            GuardFall();
            if (freezer != nullptr)
                freezer->Apply();
            if (observer != nullptr)
                observer->Poll(frame);
            ++frame;
        }
    }

    template <typename F>
    bool StepUntil(int limit, F&& done)
    {
        for (int i = 0; i < limit; ++i)
        {
            if (done())
                return true;
            Step(1);
        }
        return done();
    }

    // Puts Sly at `pos` and lets him land. Teleporting can drop him off a
    // ledge into a kill volume (then he is zapped and the level asks for a
    // checkpoint reload, which the sweep holds back, so he stays dead): as
    // soon as he falls more than 250 units below the spot he is put back
    // where he last stood. True when he stands there.
    bool Place(const glm::vec3& pos)
    {
        if (g_pjt == nullptr || g_pjt->jts == JTS_Zap)
            return false;
        const glm::vec3 safe = g_pjt->xf.posWorld;
        guardArmed = true;
        haveSafe = false;
        TeleportSly(pos);
        for (int i = 0; i < 60; ++i)
        {
            Step(1);
            if (g_pjt->jts == JTS_Zap)
                return false;
            if (g_pjt->xf.posWorld.z < pos.z - 250.0f)
            {
                TeleportSly(safe);
                StepUntil(60, [] { return g_pjt->jts == JTS_Stand || g_pjt->jts == JTS_Zap; });
                return false;
            }
            if (g_pjt->jts == JTS_Stand && i >= 5)
                return true;
        }
        return g_pjt->jts == JTS_Stand;
    }

    // Puts Sly beside `target` (70 units off on each side in turn, 40 below
    // its centre) until he stands there: a place the player could stand to
    // reach it. False when no side works.
    bool ApproachNear(ALO* ptarget)
    {
        const float angles[] = { 1.5708f, -1.5708f, 0.0f, 3.14159f };
        for (float angle : angles)
            if (Place(ptarget->xf.posWorld + glm::vec3(std::cos(angle) * 70.0f, std::sin(angle) * 70.0f, -40.0f)))
                return true;
        return false;
    }

    // Walks Sly up to a lock group: on the group (where Sly stands to use
    // it, a little above so he lands), else beside one of its locks (the
    // panel the player faces), else beside the group.
    bool ApproachGroup(LOCKG* plockg)
    {
        if (Place(plockg->xf.posWorld + glm::vec3(0.0f, 0.0f, 50.0f)))
            return true;
        for (int i = 0; i < plockg->cplock; ++i)
            if (plockg->aplock[i] != nullptr && FIsLoInWorld(plockg->aplock[i]) != 0 && ApproachNear(plockg->aplock[i]))
                return true;
        return ApproachNear(plockg);
    }

    // Turns Sly to face the target (rotation only).
    static void Face(ALO* ptarget)
    {
        if (g_pjt == nullptr)
            return;
        const glm::vec3 d = ptarget->xf.posWorld - g_pjt->xf.posWorld;
        const float face = std::atan2(d.y, d.x);
        glm::mat3 m(1.0f);
        m[0] = glm::vec3(std::cos(face), std::sin(face), 0.0f);
        m[1] = glm::vec3(-std::sin(face), std::cos(face), 0.0f);
        m[2] = glm::vec3(0.0f, 0.0f, 1.0f);
        g_pjt->xf.mat = m;
        g_pjt->xf.matWorld = m;
    }

    // One swing of the cane (square) at the target from where Sly stands
    // once he stands still; true when the target broke or left the world.
    bool Swing(ALO* ptarget, nlohmann::json& attempts, const std::string& from, bool fromSide)
    {
        StepUntil(90, [] { return g_pjt != nullptr && g_pjt->jts == JTS_Stand; });
        Face(ptarget);
        const int jts = g_pjt != nullptr ? g_pjt->jts : -1;
        const float reach = g_pjt != nullptr ? glm::length(ptarget->xf.posWorld - g_pjt->xf.posWorld) : -1.0f;
        // Only a swing from beside it, with Sly standing close, says
        // anything about the target (a fallen or zapped Sly, or one far off
        // where he happened to be, swings at nothing).
        const bool counts = fromSide && jts == JTS_Stand && reach >= 0.0f && reach < kSwingReach;
        conclusiveSwings += counts ? 1 : 0;
        InjectButton(BTN_SQUARE, true);
        Step(1);
        InjectButton(BTN_SQUARE, false);
        StepUntil(45, [&] { return Gone(ptarget); });
        const bool broke = Gone(ptarget);
        attempts.push_back({ { "from", from }, { "jts", jts }, { "distance", reach }, { "counts", counts },
            { "broke", broke }, { "player", PlayerPosition() } });
        return broke;
    }

    // Swings at the target until it breaks: first from where Sly is (after
    // an unlock he stands at the lock group, where the player swings), then
    // from four sides, 70 units off and 40 below its centre. Returns the
    // swings made.
    int SwingAt(ALO* ptarget, bool& broke, nlohmann::json& attempts)
    {
        conclusiveSwings = 0;
        broke = Gone(ptarget);
        int swings = 0;
        if (!broke)
        {
            ++swings;
            broke = Swing(ptarget, attempts, "in place", false);
        }
        const float angles[] = { 1.5708f, -1.5708f, 0.0f, 3.14159f };
        for (float angle : angles)
        {
            if (broke)
                break;
            if (!Place(ptarget->xf.posWorld + glm::vec3(std::cos(angle) * 70.0f, std::sin(angle) * 70.0f, -40.0f)))
                continue;
            ++swings;
            broke = Swing(ptarget, attempts, "side " + std::to_string(static_cast<int>(angle * 57.3f)), true);
        }
        return swings;
    }
};

struct UnlockResult
{
    int oid = -1;
    std::string how;      // see Unlock: triggered, approach+trigger, self, forced, failed, already open
    int framesToUnlock = -1;
    int stateBefore = -1;
    int stateAfter = -1;
    int cplock = 0;
    unsigned grfws = 0;
    int jtsAtTrigger = -1;
    int locksInWorldAfter = 0;
    std::string levelChange; // a level change the game asked for meanwhile
    nlohmann::json position;     // the group's
    nlohmann::json playerAfter;  // Sly's, and his state, once it is open
    int jtsAfter = -1;
    bool approached = false;     // Sly found a place to stand beside it
};

int SmaCurOid(const SMA* psma)
{
    if (psma == nullptr || psma->psm == nullptr || psma->ismsCur < 0 ||
        psma->ismsCur >= static_cast<int>(psma->psm->asms.size()))
        return -1;
    return static_cast<int>(psma->psm->asms[psma->ismsCur].oid);
}

// Unlocks one lock group the way the player does. The world gets its keys;
// once the group waits for Sly (state 558) and Sly stands, TriggerLockg is
// called, which is what the group's volume does when he walks in, and Sly
// plays the unlock (jt.cpp: jts 9, then case 320 sets the goals and the
// world flag). A group parked until Sly approaches (state 1374) gets Sly put
// at it first. The last resort sets the goals case 320 sets ("forced").
UnlockResult Unlock(Driver& driver, LOCKG* plockg)
{
    UnlockResult r;
    r.oid = OidOf(plockg);
    r.cplock = plockg->cplock;
    r.grfws = static_cast<unsigned>(plockg->grfws);
    r.stateBefore = SmaCurOid(plockg->psma);
    if (g_pwsCur != nullptr)
        g_pwsCur->ckey = (std::max)(g_pwsCur->ckey, plockg->cplock);
    auto unlocked = [&] { return SmaCurOid(plockg->psma) == 559; };
    auto waiting = [&] { return SmaCurOid(plockg->psma) == 558; };
    auto standing = [] { return g_pjt != nullptr && g_pjt->jts == JTS_Stand; };
    const int start = driver.frame;
    auto attempt = [&](const char* how)
    {
        driver.StepUntil(300, standing);
        r.jtsAtTrigger = g_pjt != nullptr ? g_pjt->jts : -1;
        r.how = how;
        TriggerLockg(plockg);
        return driver.StepUntil(600, unlocked);
    };
    // How it opened: "triggered" (TriggerLockg from where Sly stood),
    // "approach+trigger" (Sly put beside it first), "self" (it opened by
    // itself once Sly came near), "forced" (case 320's goals set by the
    // sweep), "failed" (none of these), "already open".
    if (unlocked())
        r.how = "already open";
    bool done = unlocked() || (waiting() && attempt("triggered"));
    if (!done && !unlocked())
    {
        r.approached = driver.ApproachGroup(plockg);
        driver.StepUntil(600, [&] { return waiting() || unlocked(); });
        if (unlocked())
        {
            r.how = "self";
            done = true;
        }
        else
            done = waiting() && attempt("approach+trigger");
    }
    if (!done && !unlocked())
    {
        r.how = "forced";
        for (int i = 0; i < plockg->cplock; ++i)
            if (plockg->aplock[i] != nullptr)
                SetSmaGoal(plockg->aplock[i]->psma, static_cast<OID>(559));
        SetSmaGoal(plockg->psma, static_cast<OID>(559));
        if (g_pwsCur != nullptr)
            g_pwsCur->fws |= plockg->grfws;
        if (!driver.StepUntil(400, unlocked))
            r.how = "failed";
    }
    r.framesToUnlock = r.how == "failed" ? -1 : driver.frame - start;
    r.position = { plockg->xf.posWorld.x, plockg->xf.posWorld.y, plockg->xf.posWorld.z };
    r.playerAfter = PlayerPosition();
    r.jtsAfter = g_pjt != nullptr ? g_pjt->jts : -1;
    r.stateAfter = SmaCurOid(plockg->psma);
    for (int i = 0; i < plockg->cplock; ++i)
        r.locksInWorldAfter += plockg->aplock[i] != nullptr && FIsLoInWorld(plockg->aplock[i]) != 0 ? 1 : 0;
    if (driver.stepper.result != nullptr && !driver.stepper.result->transitionRequested.empty())
        r.levelChange = driver.stepper.result->transitionRequested;
    return r;
}

nlohmann::json ToJson(const UnlockResult& r)
{
    return { { "lockg", r.oid }, { "how", r.how }, { "frames", r.framesToUnlock }, { "state_before", r.stateBefore },
        { "state_after", r.stateAfter }, { "locks", r.cplock }, { "grfws", Hex(r.grfws) },
        { "jts_at_trigger", r.jtsAtTrigger }, { "locks_in_world_after", r.locksInWorldAfter },
        { "level_change_requested", r.levelChange }, { "position", r.position }, { "player_after", r.playerAfter },
        { "jts_after", r.jtsAfter }, { "approached", r.approached } };
}

// Saves through the game's own path and loads the same level again.
// `loseFlagOf` (a check of the checks, --inject-lose-flag): before saving,
// the world flag of that lock group is cleared, as if the save had lost the
// unlock, so the reload brings back what the unlock removed (the broken
// fuse).
bool SaveAndReload(Driver& driver, nlohmann::json& out, int loseFlagOf)
{
    constexpr int kSlot = 2;
    if (loseFlagOf >= 0 && g_pwsCur != nullptr)
        for (LOCKG* plockg : InWorldOfClass<LOCKG>(CID_LOCKG))
            if (OidOf(plockg) == loseFlagOf)
            {
                std::fprintf(stderr, "[unlock] injected: lock group %d's world flag 0x%x cleared before the save\n",
                    loseFlagOf, static_cast<unsigned>(plockg->grfws));
                g_pwsCur->fws &= ~plockg->grfws;
                out["injected_lost_flag"] = { { "lockg", loseFlagOf }, { "grfws", Hex(static_cast<unsigned>(plockg->grfws)) } };
            }
    g_saveData.pgsCurrentSave = &g_saveData.saveData[kSlot];
    const bool saved = SaveCurrentGameToDisk(&g_saveData);
    const bool loaded = LoadCurrentSave(&g_saveData) != 0;
    out["saved"] = saved;
    out["save_loaded"] = loaded;
    // What the save itself holds, before the level's load can change it.
    out["saved_flags"] = Flags();
    LEVELINFO* plevel = g_transition.m_plevelCurrent;
    if (plevel == nullptr)
        return false;
    g_transition.Set(plevel, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
    driver.stepper.Step(-1);
    const bool reloaded = g_transition.m_plevelCurrent == plevel && g_psw != nullptr;
    out["reloaded"] = reloaded;
    return saved && loaded && reloaded;
}

struct LevelDrive
{
    nlohmann::json unlocks = nlohmann::json::array();
    nlohmann::json buttons = nlohmann::json::array(); // the ones that changed a response
    int buttonsTouched = 0;
    std::vector<sweep::BreakableRecord> breakables;
    int cluesTouched = 0;
    int cluesBefore = -1;
    int cluesAfter = -1;
    std::set<std::string> changedKeys; // responses any trigger changed (snapshot these too)
    // Responses changed by a trigger that also changed the saved state (an
    // unlock that set its world flag): the reload is expected to show them.
    std::set<std::string> savedKeys;
};

// Whether the saved world/level state differs (the key count the sweep
// itself gives a lock group does not count).
bool SavedStateChanged(std::map<std::string, std::string> before, std::map<std::string, std::string> after)
{
    before.erase("ckey");
    after.erase("ckey");
    return before != after;
}

// The objects whose cane response differs from `before` after `trigger`
// ran: each becomes a record, and Sly swings at those that became
// breakable.
void CollectResponseChanges(Driver& driver, const std::map<ALO*, int>& before, const std::string& trigger,
    bool saved, LevelDrive& drive, nlohmann::json& changed, nlohmann::json& swings)
{
    for (const auto& [palo, response] : before)
    {
        // The object list of the level does not change while it runs. A
        // player object's response follows its own state (Murray mid
        // attack), so only level objects count.
        if (palo->ficg.grficSweep == response || FIsBasicDerivedFrom(palo, CID_PO))
            continue;
        sweep::BreakableRecord record;
        record.oid = OidOf(palo);
        record.cid = CidOf(palo);
        record.name = Name(palo);
        record.trigger = trigger;
        record.sweepBefore = response;
        record.sweepAfter = palo->ficg.grficSweep;
        drive.changedKeys.insert(ObjectKey(palo));
        if (saved)
            drive.savedKeys.insert(ObjectKey(palo));
        if (sweep::CaneBreaks(record.sweepAfter) && !sweep::CaneBreaks(record.sweepBefore) && FIsLoInWorld(palo) != 0)
        {
            record.hitTried = true;
            nlohmann::json attempts = nlohmann::json::array();
            record.swings = driver.SwingAt(palo, record.broke, attempts);
            record.conclusiveSwings = driver.conclusiveSwings;
            swings.push_back({ { "oid", record.oid }, { "attempts", attempts } });
        }
        changed.push_back(sweep::ToJson(record));
        drive.breakables.push_back(record);
    }
}

// The in-session changes: unlocks, volume buttons (switches and mission
// triggers) touched, swings at whatever became breakable, clue pickups.
// `injectStuck` puts the named object's response back after its unlock (a
// check of the checks: the live transition is missed).
LevelDrive DriveLevel(Driver& driver, int injectStuck, nlohmann::json& log)
{
    LevelDrive drive;
    for (LOCKG* plockg : InWorldOfClass<LOCKG>(CID_LOCKG))
    {
        const std::map<ALO*, int> before = Responses();
        const std::map<std::string, std::string> flagsBefore = Flags();
        UnlockResult u = Unlock(driver, plockg);
        driver.Step(120);
        if (injectStuck >= 0)
            for (const auto& [palo, response] : before)
                if (OidOf(palo) == injectStuck && palo->ficg.grficSweep != response)
                {
                    std::fprintf(stderr, "[unlock] injected: oid %d response put back from %d to %d\n", injectStuck,
                        palo->ficg.grficSweep, response);
                    palo->ficg.grficSweep = palo->ficg.grficRush = palo->ficg.grficSmash = static_cast<byte>(response);
                    log["injected"] = { { "oid", injectStuck }, { "response", response } };
                }
        nlohmann::json row = ToJson(u);
        nlohmann::json changed = nlohmann::json::array();
        nlohmann::json swings = nlohmann::json::array();
        CollectResponseChanges(driver, before, "lockg " + std::to_string(u.oid) + " (" + u.how + ")",
            SavedStateChanged(flagsBefore, Flags()), drive, changed, swings);
        row["responses_changed"] = changed;
        row["swings"] = swings;
        drive.unlocks.push_back(row);
    }
    // Volume buttons: Sly stands in each (the player walking into a switch
    // or a mission trigger); a level change it asks for is held back.
    for (VOLBTN* pvolbtn : InWorldOfClass<VOLBTN>(CID_VOLBTN))
    {
        if (FIsLoInWorld(pvolbtn) == 0)
            continue;
        const std::map<ALO*, int> before = Responses();
        const sweep::Snapshot snapBefore = Snap({});
        const std::map<std::string, std::string> flagsBefore = Flags();
        driver.guardArmed = true;
        TeleportSly(pvolbtn->xf.posWorld);
        driver.Step(60);
        ++drive.buttonsTouched;
        nlohmann::json changed = nlohmann::json::array();
        nlohmann::json swings = nlohmann::json::array();
        CollectResponseChanges(driver, before, "volbtn " + std::to_string(OidOf(pvolbtn)),
            SavedStateChanged(flagsBefore, Flags()), drive, changed, swings);
        // Which persisted-relevant objects the button put into or took out
        // of the world, so a live/reload difference can be traced to it.
        nlohmann::json moved = nlohmann::json::array();
        const sweep::Snapshot snapAfter = Snap({});
        for (const auto& [key, o] : snapAfter.objects)
        {
            const auto it = snapBefore.objects.find(key);
            const int was = it == snapBefore.objects.end() ? 0 : it->second.inWorld;
            if (was != o.inWorld)
                moved.push_back({ { "key", key }, { "in_world_before", was }, { "in_world_after", o.inWorld } });
        }
        if (!changed.empty() || !moved.empty())
            drive.buttons.push_back({ { "volbtn", OidOf(pvolbtn) }, { "responses_changed", changed }, { "swings", swings },
                { "presence_changed", moved } });
    }
    drive.cluesBefore = g_plsCur != nullptr ? g_plsCur->abitClue.m_cbitSet : -1;
    for (ALO* pclue : InWorldOfClass<ALO>(CID_CLUE))
    {
        if (FIsLoInWorld(pclue) == 0)
            continue;
        driver.guardArmed = true;
        TeleportSly(pclue->xf.posWorld);
        driver.Step(30);
        ++drive.cluesTouched;
    }
    drive.cluesAfter = g_plsCur != nullptr ? g_plsCur->abitClue.m_cbitSet : -1;
    driver.Step(120);
    return drive;
}

// Whether a snapshot's "sm:state,sm:state" text holds a machine in `state`.
bool HasState(const std::string& machines, int state)
{
    const std::string want = std::to_string(state);
    size_t start = 0;
    while (start <= machines.size())
    {
        const size_t end = machines.find(',', start);
        const std::string one = machines.substr(start, end == std::string::npos ? std::string::npos : end - start);
        const size_t colon = one.find(':');
        if (colon != std::string::npos && one.substr(colon + 1) == want)
            return true;
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return false;
}

// After the reload: Sly walks up to each lock group that was open live, so a
// group that waits for him to come near shows whether it opens again (its
// world flag kept the unlock) or waits for the keys again.
sweep::ReloadContext Reapproach(Driver& driver, const sweep::Snapshot& live, nlohmann::json& log)
{
    sweep::ReloadContext context;
    log = nlohmann::json::array();
    for (LOCKG* plockg : InWorldOfClass<LOCKG>(CID_LOCKG))
    {
        const std::string key = ObjectKey(plockg);
        const auto it = live.objects.find(key);
        if (it == live.objects.end() || !HasState(it->second.sm, 559))
            continue;
        const int before = SmaCurOid(plockg->psma);
        if (before != 559)
        {
            driver.ApproachGroup(plockg);
            driver.StepUntil(600, [&] { const int state = SmaCurOid(plockg->psma); return state == 559 || state == 558; });
            driver.Step(30);
        }
        const sweep::Snapshot now = Snap({});
        const auto at = now.objects.find(key);
        const std::string sm = at != now.objects.end() ? at->second.sm : std::string();
        context.reapproached[key] = sm;
        if (sm == it->second.sm)
            for (int i = 0; i < plockg->cplock; ++i)
                if (plockg->aplock[i] != nullptr)
                    context.advisoryKeys.insert(ObjectKey(plockg->aplock[i]));
        log.push_back({ { "lockg", key }, { "live", it->second.sm }, { "after_reload", before }, { "after_approach", sm } });
    }
    return context;
}

void Classify(const std::vector<sweep::ReloadMismatch>& mismatches, const sweep::ReloadContext& context,
    nlohmann::json& failing, nlohmann::json& advisory)
{
    failing = nlohmann::json::array();
    advisory = nlohmann::json::array();
    for (const sweep::ReloadMismatch& m : mismatches)
        (sweep::ReloadMismatchFails(m, context) ? failing : advisory).push_back(sweep::ToJson(m));
}

// --- the user's fuse sequence ---------------------------------------------------

constexpr int kWorldTide = 1;
constexpr OID kBreakerLockg = static_cast<OID>(1843);
constexpr OID kFuse = static_cast<OID>(1487);

ALO* AloFromOid(OID oid)
{
    for (ALO* palo : allSWAloObjs)
        if (palo != nullptr && palo->oid == oid)
            return palo;
    return nullptr;
}

int ScenarioFuse(const Options& options, Observer& observer, nlohmann::json& out, sweep::LevelResult& result)
{
    const int injectStuck = options.Int("--inject-stuck", -1);
    const std::string keyLevel = options.Get("--key-level", "A Cunning Disguise");
    const std::string hub = "Prowling the Grounds";
    Stepper stepper;
    stepper.result = &result;
    Driver driver{ stepper, &observer };
    driver.fallGuard = true;
    nlohmann::json steps = nlohmann::json::array();

    // Two keys from earlier levels in Tide of Terror.
    if (g_pgsCur == nullptr)
    {
        out["error"] = "no game state";
        return 1;
    }
    WS& tide = g_pgsCur->aws[kWorldTide];
    tide.ckey = 2;
    tide.als[2].grfls |= FLS_Visited | FLS_KeyCollected;
    tide.als[3].grfls |= FLS_Visited | FLS_KeyCollected;
    steps.push_back("ckey 2 (High Class Heist, Into the Machine)");

    // The third in the level before.
    BeginLevel(keyLevel + " (fuse: third key)");
    sweep::LevelResult keyResult;
    if (!LoadLevel(stepper, keyResult, keyLevel))
    {
        out["error"] = keyLevel + " did not load";
        return 1;
    }
    stepper.result = &result;
    driver.Step(60);
    const std::vector<KEY*> keys = InWorldOfClass<KEY>(CID_KEY);
    out["keys_in_level"] = static_cast<int>(keys.size());
    nlohmann::json keyStates = nlohmann::json::array();
    for (KEY* pkey : keys)
    {
        keyStates.push_back({ { "oid", OidOf(pkey) }, { "dprizes", static_cast<int>(pkey->dprizes) } });
        if (g_pwsCur->ckey >= 3)
            break;
        TeleportSly(pkey->xf.posWorld);
        driver.StepUntil(180, [] { return g_pwsCur->ckey >= 3; });
    }
    out["key_states"] = keyStates;
    out["key_collected_by_touch"] = g_pwsCur->ckey >= 3;
    if (g_pwsCur->ckey < 3 && !keys.empty())
    {
        // The key's own collection (what touching it does once it can be
        // taken): flags the level, counts the key, shows the counter.
        SetKeyDprizes(keys.front(), DPRIZES_Collect);
        driver.Step(60);
        steps.push_back("key not taken by touch (state " + std::to_string(keyStates.front().value("dprizes", -1)) +
            "); collected through SetKeyDprizes");
    }
    if (g_pwsCur->ckey < 3)
    {
        g_pwsCur->ckey = 3;
        g_plsCur->grfls |= FLS_KeyCollected;
        steps.push_back("no key object; set ckey 3 and the level's key flag");
    }
    else if (out["key_collected_by_touch"].get<bool>())
        steps.push_back("third key collected by touch in " + keyLevel);
    out["ckey_before_hub"] = g_pwsCur->ckey;

    // Back to the hub through the level's own exit, as a transition.
    EXIT* pexit = nullptr;
    for (EXIT* candidate : InWorldOfClass<EXIT>(CID_EXIT))
        if (candidate->plevel != nullptr && candidate->plevel->levelName == hub)
            pexit = candidate;
    stepper.allowTransitions = true;
    if (pexit != nullptr)
    {
        TriggerExit(pexit);
        steps.push_back("exit " + std::to_string(OidOf(pexit)) + " to " + hub);
    }
    else
    {
        TriggerDefaultExit(0, static_cast<WIPEK>(0));
        steps.push_back("default exit");
    }
    LEVELINFO* phub = PlevelinfoFromLevelName(hub);
    BeginLevel(hub + " (fuse: arrival)");
    observer.ResetLive();
    const bool arrived = driver.StepUntil(900, [&] { return g_transition.m_plevelCurrent == phub && g_psw != nullptr; });
    stepper.allowTransitions = false;
    out["arrived"] = arrived;
    out["arrival_frame"] = driver.frame;
    if (!arrived)
    {
        out["error"] = "never arrived in " + hub + " (current " +
            (g_transition.m_plevelCurrent != nullptr ? g_transition.m_plevelCurrent->levelName : std::string("none")) + ")";
        out["steps"] = steps;
        return 1;
    }
    driver.Step(120);
    const sweep::Snapshot fresh = Snap({});
    ALO* pfuse = AloFromOid(kFuse);
    LOCKG* pbreaker = nullptr;
    for (LOCKG* p : InWorldOfClass<LOCKG>(CID_LOCKG))
        if (p->oid == kBreakerLockg)
            pbreaker = p;
    out["fuse_found"] = pfuse != nullptr;
    out["breaker_found"] = pbreaker != nullptr;
    out["fuse_in_world_at_arrival"] = pfuse != nullptr && FIsLoInWorld(pfuse) != 0;
    out["fuse_response_before"] = pfuse != nullptr ? pfuse->ficg.grficSweep : -1;
    out["breaker_state_at_arrival"] = pbreaker != nullptr ? SmaCurOid(pbreaker->psma) : -1;
    if (pfuse == nullptr || pbreaker == nullptr)
    {
        out["steps"] = steps;
        return 1;
    }

    // Arrive at the breaker and unlock it.
    const UnlockResult u = Unlock(driver, pbreaker);
    out["unlock"] = ToJson(u);
    driver.Step(120);
    if (injectStuck == static_cast<int>(kFuse))
    {
        std::fprintf(stderr, "[unlock] injected: fuse response put back to 3 after the unlock\n");
        pfuse->ficg.grficSweep = pfuse->ficg.grficRush = pfuse->ficg.grficSmash = 3;
        out["injected"] = true;
    }
    out["fuse_response_after_unlock"] = pfuse->ficg.grficSweep;
    // The unlock segment on the breaker (1874 in every run so far; its
    // start event is the script's (set-o fuse 197 17)).
    nlohmann::json segments = nlohmann::json::array();
    for (const sweep::TraceEvent& e : observer.trace.events)
        if (e.kind == "aseg+" && sweep::GlobMatch("*@" + std::to_string(static_cast<int>(kBreakerLockg)), e.key))
            segments.push_back(e.key);
    const bool segment = !segments.empty();
    out["breaker_segments_applied"] = segments;

    // The swing.
    sweep::BreakableRecord record;
    record.oid = static_cast<int>(kFuse);
    record.cid = CidOf(pfuse);
    record.name = Name(pfuse);
    record.trigger = "lockg 1843 (" + u.how + ")";
    record.sweepBefore = out["fuse_response_before"].get<int>();
    record.sweepAfter = pfuse->ficg.grficSweep;
    record.hitTried = true;
    nlohmann::json attempts = nlohmann::json::array();
    record.swings = driver.SwingAt(pfuse, record.broke, attempts);
    record.conclusiveSwings = driver.conclusiveSwings;
    out["fuse"] = sweep::ToJson(record);
    out["fuse_swings"] = attempts;
    std::vector<std::string> findings;
    if (u.how == "failed" || u.how == "forced")
        findings.push_back("the breaker did not unlock through the player's path (" + u.how + ")");
    if (!segment)
        findings.push_back("no unlock segment was applied to the breaker (LOCKG 1843)");
    if (!sweep::CaneBreaks(record.sweepAfter))
        findings.push_back("after the unlock the fuse's cane response is " + std::to_string(record.sweepAfter) +
            ", not breakable (expected 17)");
    if (!record.broke)
        findings.push_back("the cane swings (" + std::to_string(record.swings) + ") did not break the fuse");
    driver.Step(120);
    // The fuse's key, kept: pfuse dies with the world on the reload.
    const std::string fuseKey = ObjectKey(pfuse);
    const sweep::Snapshot live = Snap({ fuseKey });

    // Saved and loaded again: the fuse must agree.
    nlohmann::json reload;
    BeginLevel(hub + " (fuse: reload)");
    observer.ResetLive();
    if (!SaveAndReload(driver, reload, options.Int("--inject-lose-flag", -1)))
        findings.push_back("save and reload failed");
    driver.Step(120);
    ALO* pfuseAgain = AloFromOid(kFuse);
    reload["fuse_in_world"] = pfuseAgain != nullptr && FIsLoInWorld(pfuseAgain) != 0;
    if (record.broke && reload["fuse_in_world"].get<bool>())
        findings.push_back("the fuse broke live but is back after the save and reload");
    nlohmann::json approach;
    sweep::ReloadContext context = Reapproach(driver, live, approach);
    context.drivenKeys.insert(fuseKey);
    context.savedFlags = reload.value("saved_flags", std::map<std::string, std::string>{});
    const sweep::Snapshot again = Snap({ fuseKey });
    const std::vector<sweep::ReloadMismatch> mismatches = sweep::CompareLiveReload(fresh, live, again);
    nlohmann::json failing, advisory;
    Classify(mismatches, context, failing, advisory);
    reload["mismatches"] = failing;
    reload["advisory"] = advisory;
    reload["reapproach"] = approach;
    for (const sweep::ReloadMismatch& m : mismatches)
        if (sweep::ReloadMismatchFails(m, context))
            findings.push_back("live vs reload: " + m.key + " " + m.field + ": live " + m.live + ", reload " + m.reload +
                " (" + m.story + ")");
    out["reload"] = reload;
    out["snapshots"] = { { "arrival", sweep::ToJson(fresh) }, { "live", sweep::ToJson(live) },
        { "reload", sweep::ToJson(again) } };
    out["steps"] = steps;
    out["findings"] = findings;
    out["falls_caught"] = driver.fallsCaught;
    return 0;
}
}

// --- mode: behaviour ------------------------------------------------------------

int ModeBehaviour(const Options& options)
{
    BootGame(options);
    const fs::path results = options.Get("--results");
    const fs::path traces = options.Get("--traces");
    const int frames = options.Int("--frames", 900);
    fs::create_directories(traces);
    while (glGetError() != GL_NO_ERROR) {}

    for (const std::string& name : ReadList(options.Get("--list")))
    {
        BeginLevel(name);
        sweep::LevelResult result;
        Stepper stepper;
        Observer observer;
        observer.trace.level = name;
        Freezer freezer;
        freezer.root = options.Int("--freeze-root", -1);
        Driver driver{ stepper, &observer, 0, &freezer };
        if (LoadLevel(stepper, result, name))
        {
            result.objects = static_cast<int>(allSWAloObjs.size());
            driver.Step(frames);
            result.frames = frames;
        }
        observer.Finish();
        const fs::path tracePath = traces / (SafeName(name) + ".json");
        WriteJson(tracePath, sweep::ToJson(observer.trace));
        nlohmann::json line = LevelSummary(result);
        line["trace"] = tracePath.string();
        line["events"] = observer.trace.events.size();
        line["tracks_judged"] = observer.tracksJudged;
        line["segments_seen"] = observer.segmentsSeen;
        line["verdicts"] = observer.verdicts;
        line["blend_channels_not_judged"] = observer.blendChannels.size();
        line["static_targets"] = observer.staticTargets;
        line["unresolved_segments"] = observer.unresolvedSegments;
        line["stalled_transitions"] = observer.stalledTransitions;
        line["frozen_root"] = freezer.root;
        line["frozen_objects"] = static_cast<int>(freezer.held.size());
        line["sidebag_rebuild_list"] = TakeSidebagRebuilds(result.sidebagRebuilds);
        line["sidebag_rebuilds"] = result.sidebagRebuilds;
        std::ofstream(results, std::ios::app) << line.dump() << "\n";
    }
    ShutdownGame();
    return 0;
}

// --- mode: unlock ---------------------------------------------------------------

int ModeUnlock(const Options& options)
{
    BootGame(options);
    const fs::path results = options.Get("--results");
    const int injectStuck = options.Int("--inject-stuck", -1);
    while (glGetError() != GL_NO_ERROR) {}

    if (options.Get("--scenario") == "fuse")
    {
        sweep::LevelResult result;
        result.level = "fuse";
        Observer observer;
        observer.recordMotion = false;
        observer.trace.level = "fuse";
        nlohmann::json out = { { "scenario", "fuse" } };
        ScenarioFuse(options, observer, out, result);
        out["level"] = LevelSummary(result);
        out["sidebag_rebuild_list"] = TakeSidebagRebuilds(result.sidebagRebuilds);
        out["trace_events"] = observer.trace.events.size();
        nlohmann::json fuseEvents = nlohmann::json::array();
        for (const sweep::TraceEvent& e : observer.trace.events)
            if (e.key.find("1487") != std::string::npos || e.key.find("1843") != std::string::npos ||
                e.kind == "state" || e.kind == "transition")
                fuseEvents.push_back({ e.frame, e.kind, e.key });
        out["fuse_events"] = Head(fuseEvents, 200);
        WriteJson(results, out);
        ShutdownGame();
        return 0;
    }

    for (const std::string& name : ReadList(options.Get("--list")))
    {
        BeginLevel(name);
        sweep::LevelResult result;
        Stepper stepper;
        Driver driver{ stepper, nullptr };
        driver.fallGuard = true;
        nlohmann::json line;
        if (LoadLevel(stepper, result, name))
        {
            driver.Step(120);
            const sweep::Snapshot fresh = Snap({});
            nlohmann::json log = nlohmann::json::object();
            LevelDrive drive = DriveLevel(driver, injectStuck, log);
            const sweep::Snapshot live = Snap(drive.changedKeys);
            nlohmann::json reload;
            const bool reloaded = SaveAndReload(driver, reload, options.Int("--inject-lose-flag", -1));
            driver.Step(120);
            // Sly walks back up to the groups that were open live first: a
            // group that reopens when he comes near (its world flag kept the
            // unlock) also redoes what its unlock did, as it would for the
            // player, before the reloaded level is compared.
            nlohmann::json approach;
            sweep::ReloadContext context = Reapproach(driver, live, approach);
            context.drivenKeys = drive.savedKeys;
            context.savedFlags = reload.value("saved_flags", std::map<std::string, std::string>{});
            const sweep::Snapshot again = Snap(drive.changedKeys);
            const std::vector<sweep::ReloadMismatch> mismatches = reloaded
                ? sweep::CompareLiveReload(fresh, live, again) : std::vector<sweep::ReloadMismatch>{};
            nlohmann::json failing, advisory;
            Classify(mismatches, context, failing, advisory);
            nlohmann::json breakables = nlohmann::json::array();
            for (const sweep::BreakableRecord& r : drive.breakables)
                breakables.push_back(sweep::ToJson(r));
            line["unlocks"] = drive.unlocks;
            line["buttons_touched"] = drive.buttonsTouched;
            line["buttons"] = drive.buttons;
            line["breakables"] = breakables;
            line["breakable_findings"] = sweep::JudgeBreakables(drive.breakables);
            line["clues_touched"] = drive.cluesTouched;
            line["clues_before"] = drive.cluesBefore;
            line["clues_after"] = drive.cluesAfter;
            line["reload"] = reload;
            line["reload_mismatches"] = failing;
            line["reload_advisory"] = advisory;
            line["reapproach"] = approach;
            line["breakable_notes"] = sweep::BreakableNotes(drive.breakables);
            line["relevant_objects"] = live.objects.size();
            line["injected"] = log.value("injected", nlohmann::json());
            line["falls_caught"] = driver.fallsCaught;
            line["sly_dead_at_end"] = g_pjt != nullptr && g_pjt->jts == JTS_Zap;
            result.frames = driver.frame;
        }
        nlohmann::json summary = LevelSummary(result);
        for (auto it = summary.begin(); it != summary.end(); ++it)
            line[it.key()] = it.value();
        line["sidebag_rebuild_list"] = TakeSidebagRebuilds(result.sidebagRebuilds);
        line["sidebag_rebuilds"] = result.sidebagRebuilds;
        std::ofstream(results, std::ios::app) << line.dump() << "\n";
    }
    ShutdownGame();
    return 0;
}
}
