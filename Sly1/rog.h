#pragma once
#include "po.h"

enum ROVS 
{
    ROVS_Nil = -1,
    ROVS_Brake = 0,
    ROVS_Drive = 1,
    ROVS_Descend = 2,
    ROVS_Ascend = 3,
    ROVS_Max = 4
};
enum ROVTS 
{
    ROVTS_Nil = -1,
    ROVTS_Calm = 0,
    ROVTS_Firing = 1,
    ROVTS_Max = 2
};
enum ROHS 
{
    ROHS_Nil = -1,
    ROHS_Inactive = 0,
    ROHS_Spawn = 1,
    ROHS_Wander = 2,
    ROHS_Collect = 3,
    ROHS_Grab = 4,
    ROHS_Return = 5,
    ROHS_Exit = 6,
    ROHS_Happy = 7,
    ROHS_Sad = 8,
    ROHS_Die = 9,
    ROHS_Dead = 10,
    ROHS_Max = 11
};
enum ROCS 
{
    ROCS_Nil = -1,
    ROCS_Rest = 0,
    ROCS_Hit1 = 1,
    ROCS_Hit2 = 2,
    ROCS_Hit3 = 3,
    ROCS_Hit4 = 4,
    ROCS_Hit5 = 5,
    ROCS_Hit6 = 6,
    ROCS_Hit7 = 7,
    ROCS_Hit8 = 8,
    ROCS_Destroyed = 9,
    ROCS_Max = 10
};
enum ROSTS 
{
    ROSTS_Nil = -1,
    ROSTS_Close = 0,
    ROSTS_Open = 1,
    ROSTS_Max = 2
};
enum ROPS 
{
    ROPS_Nil = -1,
    ROPS_StayPut = 0,
    ROPS_Reveal = 1,
    ROPS_Collect = 2,
    ROPS_Collected = 3,
    ROPS_Max = 4
};
enum ROBK 
{
    ROBK_Nil = -1,
    ROBK_Primary = 0,
    ROBK_Secondary = 1,
    ROBK_Tertiary = 2,
    ROBK_Max = 3
};
enum ROBS 
{
    ROBS_Nil = -1,
    ROBS_Descend = 0,
    ROBS_Instruct = 1,
    ROBS_Normal = 2,
    ROBS_Lose = 3,
    ROBS_Win = 4,
    ROBS_Ascend = 5,
    ROBS_Max = 6
};
struct RODD 
{
    float crocActiveMaxEasy;
    float crocActiveMaxHard;
    CLQ clqCrocActive;
    float crohActiveMaxEasy;
    float crohActiveMaxHard;
    CLQ clqCrohActive;
    LM lmDtAddRocEasy;
    LM lmDtAddRocHard;
    CLQ clqDtAddRoc;
    LM lmDtAddRohEasy;
    LM lmDtAddRohHard;
    CLQ clqDtAddRoh;
    float svRohMaxEasy;
    float svRohMaxHard;
    CLQ clqSvRoh;
    float dradRohMaxEasy;
    float dradRohMaxHard;
    CLQ clqDradRoh;
};

class ROV : public PO
{
	public:
        ROVS rovs;
        ROVTS rovts;
        float radJoyLeft;
        float uJoyDeflectLeft;
        float radJoyRight;
        float uJoyDeflectRight;
        float svMax;
        glm::vec3 vTarget;
        struct SM* psmBody;
        struct SMA* psmaBody;
        struct AMB* pambBody;
        struct ALO* paloTurret;
        struct SM* psmTurret;
        struct SMA* psmaTurret;
        struct AMB* pambTurret;
        struct EMITTER* pemitterLeftGun;
        struct EMITTER* pemitterRightGun;
        struct ALO* paloProp;
        int cpemitterProp;
        struct EMITTER* apemitterProp[3];
        struct ROB* prob;
        struct ASEG* pasegAscend;
        struct ASEG* pasegDescend;
        struct ASEGA* pasegaDrive;
        int fRetractDrive;
};

class ROH : public SO
{
	public:
        ROHS rohs;
        float tRohs;
        struct ROB* prob;
        struct ROC* proc;
        struct ROST* prost;
        float svMax;
        float dradMax;
        glm::vec3 posWander;
        float tWanderNext;
        LM lmDtWander;
        struct ASEG* pasegMove;
        struct ASEG* pasegMoveCollect;
        struct ASEG* pasegGrab;
        struct ASEG* pasegDie;
        struct ASEGA* pasega;
        struct SM* psmHappy;
        struct SM* psmSad;
        struct SMA* psma;
        struct EXPL* pexpl;
        DLE dleRob;
};

class ROC : public SO
{
	public:
        ROCS rocs;
        float tRocs;
        struct ROB* prob;
        struct ROH* proh;
        struct SM* psm;
        struct SMA* psma;
        int cHitMax;
        float dtRocsHitDown;
        float dtRocsHitUp;
        struct EXPL* pexpl;
        struct EMITTER* pemitterBubble;
        LM lmSvcBubble;
        CLQ clqBubble;
        DLE dleRob;
};

class ROST : public SO
{
	public:
        ROSTS rosts;
        float tRosts;
        struct ROB* prob;
        struct SM* psm;
        struct SMA* psma;
        struct PNT* ppnt;
        DLE dleRob;
};

class ROP : public SO
{
	public:
        ROPS rops;
        float tRops;
        struct ROB* prob;
        struct KEY* pkey;
        struct ASEG* pasegReveal;
        struct ASEGA* pasega;
        struct EXPL* pexpl;
};

class ROB : public ALO
{
	public:
        OID oidCollectible;
        OID oidHarvester;
        OID oidVehicle;
        OID oidPrize;
        int coidRost;
        OID aoidRost[16];
        glm::vec3 posCenterArena;
        float sRadiusArenaTube;
        float sRadiusArenaRoc;
        float sRadiusArenaWander;
        float rAspectArena;
        float sCollectibleDropHeight;
        struct ROV* prov;
        struct ROP* prop;
        DL dlRocOwned;
        DL dlRocUnowned;
        DL dlRocInactive;
        int crocActive;
        DL dlRohActive;
        DL dlRohInactive;
        int crohActive;
        DL dlRostAvailable;
        DL dlRostUnavailable;
        int crocCreateMax;
        int crocCreated;
        int crocReturned;
        int crocDestroyed;
        CLQ clqTune;
        RODD arodd[2];
        RODD rodd;
        int irodd;
        int crocActiveMax;
        int crohActiveMax;
        LM lmDtAddRoc;
        float tAddRoc;
        LM lmDtAddRoh;
        float tAddRoh;
        float svRohMax;
        float dradRohMax;
        ROBK robk;
        ROBS robs;
        float tRobs;
        struct DIALOG* pdialog;
};

void*NewRov();
void InitRov(ROV* prov);
int  GetRovSize();
void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis);
void CloneRov(ROV* prov, ROV* provBase);
void DeleteRov(LO* plo);

void*NewRoh();
void InitRoh(ROH* proh);
int  GetRohSize();
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);
void CloneRoh(ROH* proh, ROH* prohBase);
void DeleteRoh(LO* plo);

void*NewRoc();
void InitRoc(ROC* proc);
int  GetRocSize();
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);
void CloneRoc(ROC* proc, ROC* procBase);
void DeleteRoc(LO* plo);

void*NewRost();
void InitRost(ROST* prost);
int  GetRostSize();
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);
void CloneRost(ROST* prost, ROST* prostBase);
void DeleteRost(LO* plo);

void*NewRop();
void InitRop(ROP* prop);
int  GetRopSize();
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);
void CloneRop(ROP* prop, ROP* probBase);
void DeleteRop(LO* plo);

void*NewRob();
void InitRob(ROB* prob);
int  GetRobSize();
void CloneRob(ROB* prob, ROB* probBase);
void DeleteRob(LO* plo);