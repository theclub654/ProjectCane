#pragma once
#include "po.h"
#include <vector>

enum RYTHMCOMMANDKIND
{
    RYTHMCOMMAND_AddMatch = 0,
    RYTHMCOMMAND_SpliceEvent = 1,
    RYTHMCOMMAND_EnableInput = 2,
    RYTHMCOMMAND_DisableInput = 3,
    RYTHMCOMMAND_EnableEffect = 4,
    RYTHMCOMMAND_DisableEffect = 5
};

struct RYTHMCOMMAND
{
    RYTHMCOMMANDKIND cmd;
    int iMeasure;
    float tBeat;
    int iWeapon;
};

struct RYTHMMATCH 
{ 
    int button; 
    float time; 
};

class RYTHM : public PO
{
    public:
    SM* psm;
    SMA* psma;
    XFM* pxfmButton[4];
    JSG* pjsgFinish;
    int iMatch;
    int cMatch;
    RYTHMMATCH aMatch[8];
    float tInput;
    int buttonInput;
    XFM* pxfmStart;
    int fInputEnabled;
    float dtInputWindow;
    int fInvulnerable;
    class RUBY* pruby;
};

class RYTHMSEQUENCE : public LO
{
    public:
    std::vector <RYTHMCOMMAND> aCommand;
    int iCommand;          
    class RUBY* pruby;     
    int iMeasureBase;      
};

RYTHM* NewRythm();
void InitRythm(RYTHM* prythm);
void LoadRythmFromBrx(RYTHM* prythm, CBinaryInputStream* pbis);
void CloneRythm(RYTHM* prythm, RYTHM* prythmBase);
void PostRythmCallback(RYTHM* prythm);
void PostRythmLoad(RYTHM* prythm);
void OnRythmActive(RYTHM* prythm, int fActive, PO* ppoOther);
void ApplyRythmThrow(RYTHM* prythm, JT* pjt);
void UpdateRythm(RYTHM* prythm, JOY* pjoy, float dt);
bool FIsRythmInvulnerable(RYTHM* prythm);
int FTakeRythmDamage(RYTHM* prythm, ZPR* pzpr);
void AddRythmMatch(RYTHM* prythm, const RYTHMMATCH* pmatch);
void PredictRythmPosition(RYTHM* prythm, float dt, glm::vec3* ppos, glm::vec3* pv);
void PredictRythmRotation(RYTHM* prythm, float dt, glm::mat3* pmat, glm::vec3* pw);
int GetRythmSize();
void DeleteRythm(RYTHM* prythm);

RYTHMSEQUENCE* NewRythmSequence();
void InitRythmSequence(RYTHMSEQUENCE* prythmSequence);
void CloneRythmSequence(RYTHMSEQUENCE* prythmSequence, RYTHMSEQUENCE* prythmSequenceBase);
void PostRythmSequenceLoad(RYTHMSEQUENCE* prythmSequence);
void ActivateRythmSequence(RYTHMSEQUENCE* prythmSequence);
void DeactivateRythmSequence(RYTHMSEQUENCE* prythmSequence);
void UpdateRythmCommands(RYTHMSEQUENCE* prythmSequence);
void DispatchRythmCommand(RYTHMSEQUENCE* prythmSequence, const RYTHMCOMMAND* prythmCommand);
RYTHMCOMMAND* AddRythmSequenceCommand(RYTHMSEQUENCE* prythmSequence, RYTHMCOMMANDKIND commandKind, int iMeasure, float tBeat);
void AddRythmSequenceMatch(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat, int iWeapon);
void AddRythmSequenceSpliceEvent(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat, int iWeapon);
void AddRythmSequenceEnableInput(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat);
void AddRythmSequenceDisableInput(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat);
void AddRythmSequenceEnableEffect(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat);
void AddRythmSequenceDisableEffect(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat);
int GetRythmSequenceSize();
void DeleteRythmSequence(RYTHMSEQUENCE* prythmSequence);
