#pragma once
#include "jt.h"

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
enum LASK 
{
    LASK_Nil = -1,
    LASK_Dynamic = 0,
    LASK_Static = 1,
    LASK_Max = 2
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
    class SO* psoHit;
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
    LASK lask;
    int clbeam;
    LBEAM albeam[16];
    int cposBeamShapeMax;
    struct ALO *paloRenderSense;
    struct ALO *paloRenderDamage;
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
    class ALO* paloRenderDamage;
    class ALO* paloRenderZap;
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

SENSOR*NewSensor();
void InitSensor(SENSOR* psensor);
void*GetSensorFTriggerAll(SENSOR* psensor);
void SetSensorFTriggerAll(SENSOR* psensor, int fTriggerAll);
void*GetSensorDtEnabling(SENSOR* psensor);
void SetSensorDtEnabling(SENSOR* psensor, float dtEnabling);
void*GetSensorDtDisabling(SENSOR* psensor);
void SetSensorDtDisabling(SENSOR* psensor, float dtDisabling);
void*GetSensorSensorsInitial(SENSOR* psensor);
void SetSensorSensorsInitial(SENSOR* psensor, SENSORS sensorsInitial);
void*GetSensorPalarm(SENSOR* psensor);
void*GetSensorSensors(SENSOR* psensor);
void*GetSensorSensm(SENSOR* psensor);
void*GetSensorFRemainDisabledIndefinite(SENSOR* psensor);
void SetSensorFRemainDisabledIndefinite(SENSOR* psensor, int fRemainDisabledIndefinite);
int  GetSensorSize();
void CloneSensor(SENSOR* psensor, SENSOR* psensorBase);
void SetSensorAlarm(SENSOR* psensor, ALARM* palarm);
void SetSensorSensors(SENSOR* psensor, SENSORS sensors);
int  FCheckSensorObject(SENSOR* psensor, SO* psoOther);
int  FIgnoreSensorObject(SENSOR* psensor, SO* psoOther);
int  FOnlySensorTriggerObject(SENSOR* psensor, SO* psoOther);
void PauseSensor(SENSOR* psensor);
void UpdateSensor(SENSOR* psensor, float dt);
void AddSensorTriggerObject(SENSOR* psensor, int oid);
void AddSensorNoTriggerObject(SENSOR* psensor, int oid);
void AddSensorTriggerClass(SENSOR* psensor, int cid);
void AddSensorNoTriggerClass(SENSOR* psensor, int cid);
void DeleteSensor(SENSOR* psensor);

LASEN*NewLasen();
void InitSwLasenDl(SW* psw);
void InitLasen(LASEN* plasen);
void*GetLasenLask(LASEN* plasen);
void SetLasenLask(LASEN* plasen, LASK lask);
void*GetLasenDtDamageDisabling(LASEN* plasen);
void SetLasenDtDamageDisabling(LASEN* plasen, float dtDamageDisabling);
int  GetLasenSize();
void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis);
void CloneLasen(LASEN* plasen, LASEN* plasenBase);
void BindLasen(LASEN* plasen);
void PostLasenLoad(LASEN* plasen);
void UpdateBusyLasenSenseTimes();
void UpdateLasen(LASEN* plasen, float dt);
void FreezeLasen(LASEN* plasen, int fFreeze);
void RenderLasenSelf(LASEN* plasen, CM* pcm, RO* pro);
int  FFilterLasen(void* pv, JT* pjt);
void SenseLasen(LASEN* plasen, SENSORS* psensors);
void EnableLasen(LASEN* plasen, SENSM sensm);
void DisableLasen(LASEN* plasen);
void OnLasenAlarmTriggered(LASEN* plasen);
void SetLasenSensors(LASEN* plasen, SENSORS sensors);
float SCalcLasenShapeExtent(LASEN* plasen, LBEAM* plbeam);
void RetractLasen(LASEN* plasen, float dtRetract);
void ExtendLasen(LASEN* plasen, float dtExpand);
void DeleteLasen(LASEN *plasen);

CAMSEN*NewCamsen();
void InitCamsen(CAMSEN* pcamsen);
void*GetCamsenDtDamageFocus(CAMSEN* pcamsen);
void SetCamsenDtDamageFocus(CAMSEN* pcamsen, float dtDamageFocus);
void*GetCamsenDtDamageZap(CAMSEN* pcamsen);
void SetCamsenDtDamageZap(CAMSEN* pcamsen, float dtDamageZap);
void*GetCamsenDtDamageUnfocus(CAMSEN* pcamsen);
void SetCamsenDtDamageUnfocus(CAMSEN* pcamsen, float dtDamageUnfocus);
void*GetCamsenCsdts(CAMSEN* pcamsen);
int  GetCamsenSize();
void CloneCamsen(CAMSEN* pcamsen, CAMSEN* pcamsenBase);
void PostCamsenLoad(CAMSEN* pcamsen);
void UpdateCamsen(CAMSEN* pcamsen, float dt);
void RenderCamsenSelf(CAMSEN* pcamsen, CM* pcm, RO* pro);
int  FIgnoreCamsenIntersection(CAMSEN* pcamsen, SO* psoOther);
int  FFilterCamsen(void* pv, SO* pso);
void SenseCamsen(CAMSEN* pcamsen, SENSORS* psensors);
void EnableCamsen(CAMSEN* pcamsen, SENSM sensm);
void DisableCamsen(CAMSEN* pcamsen);
void OnCamsenAlarmTriggered(CAMSEN* pcamsen);
void SetCamsenSensors(CAMSEN* pcamsen, SENSORS sensors);
void SetCamsenCsdts(CAMSEN* pcamsen, CSDTS csdts);
void DeleteCamsen(CAMSEN *pcamsen);

PRSEN*NewPrsen();
void InitPrsen(PRSEN* pprsen);
void*GetPrsenIframeSenseStart(PRSEN* pprsen);
void SetPrsenIframeSenseStart(PRSEN* pprsen, int iframeSenseStart);
void*GetPrsenIframeSenseEnd(PRSEN* pprsen);
void SetPrsenIframeSenseEnd(PRSEN* pprsen, int iframeSenseEnd);
void*GetPrsenDtSenseAnim(PRSEN* pprsen);
void SetPrsenDtSenseAnim(PRSEN* pprsen, float dtSenseAnim);
void*GetPrsenPssatSense(PRSEN* pprsen);
void SetPrsenPssatSense(PRSEN* pprsen, PSSAT pssatSense);
void*GetPrsenIframeDamageStart(PRSEN* pprsen);
void SetPrsenIframeDamageStart(PRSEN* pprsen, int iframeDamageStart);
void*GetPrsenIframeDamageEnd(PRSEN* pprsen);
void SetPrsenIframeDamageEnd(PRSEN* pprsen, int iframeDamageEnd);
void*GetPrsenDtDamageAnim(PRSEN* pprsen);
void SetPrsenDtDamageAnim(PRSEN* pprsen, float dtDamageAnim);
void*GetPrsenPssatDamage(PRSEN* pprsen);
void SetPrsenPssatDamage(PRSEN* pprsen, PSSAT pssatDamage);
void*GetPrsenIframeDisabledStart(PRSEN* pprsen);
void SetPrsenIframeDisabledStart(PRSEN* pprsen, int iframeDisabledStart);
void*GetPrsenIframeDisabledEnd(PRSEN* pprsen);
void SetPrsenIframeDisabledEnd(PRSEN* pprsen, int iframeDisabledEnd);
void*GetPrsenDtDisabledAnim(PRSEN* pprsen);
void SetPrsenDtDisabledAnim(PRSEN* pprsen, float dtDisabledAnim);
void*GetPrsenPssatDisabled(PRSEN* pprsen);
void SetPrsenPssatDisabled(PRSEN* pprsen, PSSAT pssatDisabled);
void*GetPrsenIframeDisablingFlash(PRSEN* pprsen);
void SetPrsenIframeDisablingFlash(PRSEN* pprsen, int iframeDisablingFlash);
void*GetPrsenSvtDisablingFlash(PRSEN* pprsen);
void SetPrsenSvtDisablingFlash(PRSEN* pprsen, float svtDisablingFlash);
void*GetPrsenDtRemainEnabled(PRSEN* pprsen);
void SetPrsenDtRemainEnabled(PRSEN* pprsen, float dtRemainEnabled);
void*GetPrsenDtRemainDisabled(PRSEN* pprsen);
void SetPrsenDtRemainDisabled(PRSEN* pprsen, float dtRemainDisabled);
int  GetPrsenSize();
void ClonePrsen(PRSEN* pprsen, PRSEN* pprsenBase);
void PostPrsenLoad(PRSEN* pprsen);
void UpdatePrsen(PRSEN* pprsen, float dt);
void SensePrsen(PRSEN* pprsen, SENSORS* psensors);
void EnablePrsen(PRSEN* pprsen, SENSM sensm);
void DisablePrsen(PRSEN* pprsen);
void OnPrsenAlarmTriggered(PRSEN* pprsen);
void SetPrsenSensors(PRSEN* pprsen, SENSORS sensors);
void UpdatePrsenLoopShader(PRSEN* pprsen);
void DeletePrsen(PRSEN* ppprsen);

extern SNIP s_asnipLasen[2];
extern int g_fLasenBusyListChange;
extern SNIP s_asnipCamsen[2];
