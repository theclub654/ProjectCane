#pragma once
#include "stepguard.h"
#include <vector>

class RYTHM;
class RYTHMSEQUENCE;
struct RYTHMCOMMAND;

enum RUBYMUSICSTATE
{
    RUBYMUSICSTATE_Idle = 0,
    RUBYMUSICSTATE_WaitingForMusic = 1,
    RUBYMUSICSTATE_RestartRhythm = 2
};

struct RUBYRYTHMEVENT
{
    ASEGA* pasega;
    int cBeats;
};

// ProjectCane-side resolved event data. This is not part of the retail
// RUBYRYTHMEVENT allocation; it is retained for the current loader code.
struct RUBYEVENT
{
    int oid;
    float time;
    LO* plo;
};

class RUBY : public STEPGUARD
{
    public:
    ASEG * apasegAttack[4];
    ASEG* pasegAttackOverride;
    RWM* aprwmAttack[4];

    union
    {
        RYTHMSEQUENCE* prythmSequencePending;
        LO* ploRythmPending;
        void* pvRythmPending;
    };
    union
    {
        RYTHMSEQUENCE* prythmSequenceActive;
        LO* ploRythmActive;
        void* pvRythmActive;
    };
    RYTHM* prythm;
    int iAttackWeapon;
    union
    {
        float tAttackEvent;
        float dtLaunchBias;
    };
    XFM* pxfmGoal;
    int fRythmEffect;
    SM* psmRuby;
    SMA* psmaRuby;
    int unkC58;
    int framesPerBeat;
    union
    {
        int beatsPerMeasure;
        int beatCount;
    };
    union
    {
        float measureDuration;
        float beatDuration;
    };
    std::vector<RUBYRYTHMEVENT> aRythmEvent;
    union
    {
        int iMeasure;
        int iBeat;
    };
    union
    {
        int cQuarterBeat;
        int iTickTotal;
    };
    union
    {
        int iQuarterBeat;
        int iTick;
    };
    union
    {
        float uBeat;
        float beatPhase;
    };
    union
    {
        float tBeatStart;
        float tTickStart;
    };
    union
    {
        float tBeatNext;
        float tTickNext;
    };
    union
    {
        float dtBeat;
        float dtTick;
    };
    union
    {
        float dtBeatTarget;
        float dtTickTarget;
    };
    unsigned char unkC90[4];
    int attackLatch;
    unsigned char unkC98[8];

    // Reconstruction-only caches used by the current function bodies. They
    // follow the mapped retail RUBY data and can be removed when those
    // functions are converted to use the original dynamic event storage.
    RUBYEVENT aevent[9];
    ASEG* apasegRuby[4];
};

RUBY*NewRuby();
void InitRuby(RUBY* pruby);
void LoadRubyFromBrx(RUBY* pruby, CBinaryInputStream* pbis);
void CloneRuby(RUBY* pruby, RUBY* prubyBase);
void PostRubyLoad(RUBY* pruby);
void OnRubyRemove(RUBY* pruby);
void OnRubyAdd(RUBY* pruby);
int  FCanRubyAttack(RUBY* pruby);
int  FDetectRuby(RUBY* pruby);
void DrawRubyHaloFire(RPL* prpl);
void RenderRubyAll(RUBY* pruby, CM* pcm, RO* pro);
void RenderRubySelf(RUBY* pruby, CM* pcm, RO* pro);
void OnRubyExitingSgs(RUBY* pruby, SGS sgsNext);
void DamageRubyBoss(RUBY* pruby);
void OnRubyEnteringSgs(RUBY* pruby, SGS sgsPrev, ASEG* pasegOverride);
void AddRubyRythmMatch(RUBY* pruby, const RYTHMCOMMAND* prythmCommand);
void HandleRubyMessage(RUBY* pruby, MSGID msgid, void* pv);
void SyncRubyRythmEvents(RUBY* pruby);
void UpdateRuby(RUBY* pruby, float dt);
void UpdateRubyBlot(RUBY* pruby);
void UpdateRubyGoal(RUBY* pruby, int fEnter);
void UpdateRubySgs(RUBY* pruby);
void SetRubyPendingRythm(RUBY* pruby, RYTHMSEQUENCE* prythmSequence);
void ClearRubyRythmSequence(RUBY* pruby);
void SetRubyFramesPerBeat(RUBY* pruby, int framesPerBeat);
void SetRubyBeatsPerMeasure(RUBY* pruby, int beatsPerMeasure);
void AddRubyRythmEvent(RUBY* pruby, ASEGA* pasega, int cBeats);
void RemoveRubyRythmEvent(RUBY* pruby, ASEGA* pasega);

// Supporting helpers not present in the retail function-order reference.
int FCheckRubyXpBase(RUBY* pruby, XP* pxp, int ixpd);
int GetRubySize();
void DeleteRuby(RUBY* pruby);
