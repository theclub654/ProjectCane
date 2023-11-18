#pragma once
#include "so.h"

enum SENSM
{
    SENSM_Nil = -1,
    SENSM_SenseOnly = 0,
    SENSM_DamageTarget = 1,
    SENSM_Max = 2
};
enum SENSORS
{
    SENSORS_Nil = -1,
    SENSORS_SenseEnabling = 0,
    SENSORS_SenseEnabled = 1,
    SENSORS_SenseTriggered = 2,
    SENSORS_SenseDisabling = 3,
    SENSORS_DamageEnabling = 4,
    SENSORS_DamageEnabled = 5,
    SENSORS_DamageTriggered = 6,
    SENSORS_DamageDisabling = 7,
    SENSORS_Disabled = 8,
    SENSORS_Max = 9
};
struct LEMIT 
{
    struct EMITTER* pemitter;
    int fScorch;
};
struct LBEAM 
{
    struct SHAPE* pshape;
    float sShape;
    float sShapeLast;
    struct SO* psoHit;
    glm::vec3 posHit;
    int iposHit;
    int clemitDamage;
    struct LEMIT alemitDamage[4];
};
enum PSSAT
{
    PSSAT_Nil = -1,
    PSSAT_Loop = 0,
    PSSAT_PingPong = 1,
    PSSAT_Max = 2
};
enum CSDTS
{
    CSDTS_Nil = -1,
    CSDTS_Focus = 0,
    CSDTS_Zap = 1,
    CSDTS_Unfocus = 2,
    CSDTS_Max = 3
};

class SENSOR : public SO
{
	public:
        struct ALARM* palarm;
        SENSM sensm;
        SENSORS sensors;
        float tSensors;
        SENSORS sensorsInitial;
        int coidTrigger;
        OID aoidTrigger[4];
        int coidNoTrigger;
        OID aoidNoTrigger[4];
        int ccidTrigger;
        CID acidTrigger[4];
        int ccidNoTrigger;
        CID acidNoTrigger[4];
        int fTriggerAll;
        float dtEnabling;
        float dtDisabling;
        int fRemainDisabledIndefinite;
        struct AMB* pamb;
        struct ASEGA* pasegaPause;
        float svtRestore;
};

class LASEN : public SENSOR
{
	public:
        LBEAM albeam[16];
        int cposBeamShapeMax;
        struct ALO* paloRenderSense;
        struct ALO* paloRenderDamage;
        float dtDamageDisabling;
        DLE dleBusyLasen;
        int fBusyLasen;
        float tSenseNext;
        int fJtOnlyTriggerObject;
        float uDrawMax;
        float svuDrawMax;
};

class CAMSEN : public SENSOR
{
	public:
        struct ALO* paloRenderDamage;
        struct ALO* paloRenderZap;
        CSDTS csdts;
        float tCsdts;
        float dtDamageFocus;
        float dtDamageZap;
        float dtDamageUnfocus;
};

class PRSEN : public SENSOR
{
	public:
        int iframeSenseStart;
        int iframeSenseEnd;
        float dtSenseAnim;
        PSSAT pssatSense;
        int iframeDamageStart;
        int iframeDamageEnd;
        float dtDamageAnim;
        PSSAT pssatDamage;
        int iframeDisabledStart;
        int iframeDisabledEnd;
        float dtDisabledAnim;
        PSSAT pssatDisabled;
        int iframeDisablingFlash;
        float svtDisablingFlash;
        float dtRemainEnabled;
        float dtRemainDisabled;
        struct LOOP* ploop;
        float tSensePrev;
        int fTriggered;
};

void InitSensor(SENSOR* psensor);
void DeleteSensor(LO* plo);
void InitLasen(LASEN* plasen);
void DeleteLasen(LO* plo);
void InitCamsen(CAMSEN* pcamsen);
void DeleteCamsen(LO* plo);
void InitPrsen(PRSEN* pprsen);
void DeletePrsen(LO* plo);
void LoadLasenFromBrx(LASEN *plasen, CBinaryInputStream *pbis);
