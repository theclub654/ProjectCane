#pragma once
#include <iostream>
#include <vector>
#include "math.h"

// Forward declarations

typedef int GRFFSO;
typedef int GRFCID;
typedef int GRFGLOB;
typedef int GRFIK;
typedef unsigned int SYMID;
typedef uint64_t GRFZON;
typedef uint32_t GRFALOX;
class CBinaryInputStream;
class PROXY;
class ALO;
class SO;
class MS;
class CM;
struct RO;
struct RPL;
enum LIGHTK;
struct LM;
enum CID;
enum OID;
struct RGBA;
struct FGFN;
struct SOP;
extern bool loadEmitMesh;
extern std::vector <RPL> renderBuffer;
extern int numRo;
extern int g_dynamicTextureCount;
extern int g_backGroundCount;
extern int g_blotContextCount;
extern int g_opaqueCount;
extern int g_cutOutCount;
extern int g_celBorderCount;
extern int g_projVolumeCount;
extern int g_opaqueAfterProjVolumeCount;
extern int g_cutoutAfterProjVolumeCount;
extern int g_celBorderAfterProjVolumeCount;
extern int g_murkClearCount;
extern int g_murkOpaqueCount;
extern int g_murkFillCount;
extern int g_translucentCount;
extern int g_translucentCelBorderCount;
extern int g_blipCount;
extern int g_foreGroundCount;
extern int g_worldMapCount;
extern int g_maxCount;
void AllocateLightBlkList();
void DeallocateLightBlkList();
struct BASIC;
struct CLQ;
struct SMPA;
enum ACK;
enum THROBK;
enum SFXID;
struct ASEGD;
void AllocateRpl();
void SubmitRpl(RPL* prpl);

// Local Obeject
class LO;
enum MSGID;
LO* NewLo();
void InitLo(LO* plo);
void SetLoDefaults(LO* plo);
void AddLo(LO* plo);
void OnLoAdd(LO* plo);
void OnLoRemove(LO* plo);
void RemoveLo(LO* plo);
void AddLoHierarchy(LO* plo);
void RemoveLoHierarchy(LO* plo);
void CloneLoHierarchy(LO* plo, LO* ploBase);
void CloneLo(LO* plo, LO* ploBase);
void LoadLoFromBrx(LO* plo, CBinaryInputStream* pbis);
void SendLoMessage(LO* plo, MSGID msgid, void* pv);
void GetLoInWorld(LO* plo, int* pfInWorld);
OID  GetLoOid(LO* plo);
OID  OidProxyLo(LO* plo);
void GetLoOidProxy(LO* plo, OID* poid);
void PostLoLoad(LO* plo);
void SetLoParent(LO* plo, ALO* paloParent);
void SubscribeLoObject(LO* plo, LO* ploTarget);
void UnsubscribeLoObject(LO* plo, LO* ploTarget);
void SubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
void UnsubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
int  GetLoSize();
void DeleteLo(LO* plo);

// A Local Object;
ALO* NewAlo();
void InitAlo(ALO* palo);
void AddAloHierarchy(ALO* palo);
void RemoveAloHierarchy(ALO* palo);
void OnAloAdd(ALO* palo);
void OnAloRemove(ALO* palo);
void CloneAloHierarchy(ALO* palo, ALO* paloBase);
void CloneAlo(ALO* palo, ALO* paloBase);
void UpdateAloXfWorld(ALO* palo);
void UpdateAloXfWorldHierarchy(ALO* palo);
void TranslateAloToPos(ALO* palo, glm::vec3& ppos);
void RotateAloToMat(ALO* palo, glm::mat3& pmat);
void SetAloParent(ALO* palo, ALO* paloParent);
void ApplyAloProxy(ALO* palo, PROXY* pproxyApply);
void BindAlo(ALO* palo);
void SetAloInitialVelocity(ALO* palo, glm::vec3* pv);
void SetAloInitialAngularVelocity(ALO* palo, const glm::vec3* pw);
ASEGD* PasegdEnsureAlo(ALO* palo);
void SetAloAsegdOid(ALO* palo, short oid);
void SetAloAsegdtLocal(ALO* palo, float tLocal);
void SetAloAsegdSvtLocal(ALO* palo, float svtLocal);
void SetAloAsegdiak(ALO* palo, int iak);
void SetAloFrozen(ALO* palo, bool fFrozen);
void SetAloEuler(ALO* palo, glm::vec3* peul);
void SetAloVelocityLocal(ALO* palo, glm::vec3* pvec);
void SetAloFastShadowRadius(ALO* palo, float sRadius);
void SetAloFastShadowDepth(ALO* palo, float sDepth);
void SetAloCastShadow(ALO* palo, int fCastShadow);
void SetAloNoFreeze(ALO* palo, int fNoFreeze);
void SetAloRestorePosition(ALO* palo, int fRestore);
void SetAloRestorePositionAck(ALO* palo, ACK ack);
void SetAloPositionSpring(ALO* palo, float r);
void SetAloPositionSpringDetail(ALO* palo, CLQ* pclq);
void SetAloPositionDamping(ALO* palo, float r);
void SetAloPositionDampingDetail(ALO* palo, CLQ* pclq);
void SetAloRestoreRotation(ALO* palo, int fRestore);
void SetAloRestoreRotationAck(ALO* palo, ACK ack);
void SetAloRotationSpring(ALO* palo, float r);
void SetAloRotationSpringDetail(ALO* palo, CLQ* pclq);
void SetAloRotationDamping(ALO* palo, float r);
void SetAloRotationDampingDetail(ALO* palo, CLQ* pclq);
void SetAloPositionSmooth(ALO* palo, float r);
void SetAloPositionSmoothDetail(ALO* palo, SMPA* psmpa);
void SetAloRotationSmooth(ALO* palo, float r);
void SetAloRotationSmoothDetail(ALO* palo, SMPA* psmpa);
void SetAloPositionSmoothMaxAccel(ALO* palo, float r);
void SetAloRotationSmoothMaxAccel(ALO* palo, float r);
void SetAloDefaultAckPos(ALO* palo, ACK ack);
void SetAloDefaultAckRot(ALO* palo, ACK ack);
void SetAloLookAt(ALO* palo, ACK ack);
void SetAloLookAtIgnore(ALO* palo, float sIgnore);
void SetAloLookAtPanFunction(ALO* palo, CLQ* pclq);
void SetAloLookAtPanLimits(ALO* palo, LM* plm);
void SetAloLookAtTiltFunction(ALO* palo, CLQ* pclq);
void SetAloLookAtTiltLimits(ALO* palo, LM* plm);
void SetAloLookAtEnabledPriority(ALO* palo, int nPriority);
void SetAloLookAtDisabledPriority(ALO* palo, int nPriority);
void SetAloTargetAttacks(ALO* palo, int grftak);
void SetAloTargetRadius(ALO* palo, float sRadiusTarget);
void SetAloThrobKind(ALO* palo, THROBK throbk);
void SetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor);
void SetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor);
void SetAloThrobDtInOut(ALO* palo, float dtInOut);
void SetAloSfxid(ALO* palo, SFXID sfxid);
void SetAloSStart(ALO* palo, float sStart);
void SetAloSFull(ALO* palo, float sFull);
void SetAloUVolumeSpl(ALO* palo, float uVol);
void SetAloUVolume(ALO* palo, float uVol);
void SetAloUPitchSpl(ALO* palo, float uPitch);
void SetAloUPitch(ALO* palo, float uPitch);
void SetAloSndRepeat(ALO* palo, LM* plm);
void SetAloUDoppler(ALO* palo, float uDoppler);
void SetAloInteractCane(ALO* palo, int grfic);
void SetAloInteractCaneSweep(ALO* palo, int grfic);
void SetAloInteractCaneRush(ALO* palo, int grfic);
void SetAloInteractCaneSmash(ALO* palo, int grfic);
void SetAloInteractBomb(ALO* palo, int grfic);
void SetAloInteractShock(ALO* palo, int grfic);
void SetAloPoseCombo(ALO* palo, OID oidCombo);
void SetAloForceCameraFade(ALO* palo, int fFade);
void* GetAloFrozen(ALO* palo);
void* GetAloXfPos(ALO* palo);
void* GetAloXfPosOrig(ALO* palo);
void* GetAloXfPosWorld(ALO* palo);
void* GetAloXfMat(ALO* palo);
void* GetAloMatOrig(ALO* palo);
void* GetAloXfMatWorld(ALO* palo);
void* GetAloEuler(ALO* palo);
void GetAloVelocityLocal(ALO* palo, glm::vec3* pvec);
void* GetAloXfw(ALO* palo);
void* GetAloXfdv(ALO* palo);
void* GetAloXfdw(ALO* palo);
void* GetAloRoot(ALO* palo);
void GetAloFastShadowRadius(ALO* palo, float* psRadius);
void GetAloFastShadowDepth(ALO* palo, float* psDepth);
void GetAloCastShadow(ALO* palo, int* pfCastShadow);
void GetAloLookAtIgnore(ALO* palo, float* psIgnore);
void GetAloLookAtPanFunction(ALO* palo, CLQ* pclq);
void GetAloLookAtPanLimits(ALO* palo, LM* plm);
void GetAloLookAtTiltFunction(ALO* palo, CLQ* pclq);
void GetAloLookAtTiltLimits(ALO* palo, LM* plm);
void GetAloLookAtEnabledPriority(ALO* palo, int* pnPriority);
void GetAloLookAtDisabledPriority(ALO* palo, int* pnPriority);
int  FGetAloChildrenList(ALO* palo, void* pvstate);
void GetAloThrobKind(ALO* palo, THROBK* pthrobk);
void GetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor);
void GetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor);
void GetAloThrobDtInOut(ALO* palo, float* pdtInOut);
void GetAloSfxid(ALO* palo, SFXID* psfxid);
void GetAloSStart(ALO* palo, float* psStart);
void GetAloSFull(ALO* palo, float* psFull);
void GetAloUVolume(ALO* palo, float* puVol);
void GetAloUPitch(ALO* palo, float* puPitch);
void GetAloSndRepeat(ALO* palo, LM* plmRepeat);
void GetAloUDoppler(ALO* palo, float* puDoppler);
void GetAloInteractCane(ALO* palo, int* pgrfic);
void GetAloInteractCaneSweep(ALO* palo, int* pgrfic);
void GetAloInteractCaneRush(ALO* palo, int* pgrfic);
void GetAloInteractCaneSmash(ALO* palo, int* pgrfic);
void GetAloInteractBomb(ALO* palo, int* pgrfic);
void GetAloInteractShock(ALO* palo, int* pgrfic);
void* GetAlofRealClock(ALO* palo);
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void PostAloLoad(ALO* palo);
void UpdateAlo(ALO* palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* pro);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
int  GetAloSize();
void DeleteAlo(ALO* palo);

// Static Object
SO*  NewSo();
void InitSo(SO* pso);
void OnSoAdd(SO* pso);
void OnSoRemove(SO* pso);
void CloneSo(SO* pso, SO* psoBase);
void SetSoParent(SO* pso, ALO* paloParent);
void ApplySoProxy(SO* pso, PROXY* pproxyApply);
void UpdateSoXfWorldHierarchy(SO* pso);
void UpdateSoXfWorld(SO* pso);
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis);
void TranslateSoToPos(SO* pso, glm::vec3& ppos);
void RotateSoToMat(SO* pso, glm::mat3& pmat);
void UpdateSo(SO* pso, float dt);
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);
int  GetSoSize();
void DeleteSo(SO *pso);
void DeallocateSoVector();

// Merged Statics
MS*  NewMs();
int  GetMsSize();
void RenderMsGlobset(MS* pms, CM* pcm, RO* pro);
void DeleteMs(MS* pms);

// Player Object
class PO;
PO*  NewPo();
void InitPo(PO* ppo);
void OnPoAdd(PO* ppo);
void OnPoRemove(PO* ppo);
void ClonePo(PO* ppo, PO* ppoBase);
int  GetPoSize();
void DeletePo(PO* ppo);

class STEP;
STEP*NewStep();
void InitStep(STEP* pstep);
int  GetStepSize();
void UpdateStepXfWorld(STEP* pstep);
void RenderStepSelf(STEP* pstep, CM* pcm, RO* pro);
void CloneStep(STEP* pstep, STEP* pstepBase);
void RotateStepToMat(STEP* pstep, glm::mat3& pmat);
void DeleteStep(STEP *pstep);

// Sly
class JT;
JT*  NewJt();
void InitJt(JT* pjt);
void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis);
void CloneJt(JT* pjt, JT* pjtBase);
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);
int  GetJtSize();
void DeleteJt(JT* pjt);

// Guard
class STEPGUARD;

STEPGUARD*NewStepguard();
void InitStepGuard(STEPGUARD* pstepguard);
void OnStepguardAdd(STEPGUARD* pstepguard);
void OnStepguardRemove(STEPGUARD* pstepguard);
void CloneStepguard(STEPGUARD* pstepguard, STEPGUARD* pstepguardBase);
void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis);
void BindStepguard(STEPGUARD* pstepguard);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);
int  GetStepguardSize();
void DeleteStepguard(STEPGUARD* pstepguard);

class SMARTGUARD;
SMARTGUARD*NewSmartguard();
void InitSmartGuard(SMARTGUARD* psmartguard);
int  GetSmartguardSize();
void CloneSmartguard(SMARTGUARD* psmartguard, SMARTGUARD* psmartguardBase);
void DeleteSmartGuard(SMARTGUARD* psmartguard);

class GOMER;
GOMER*NewGomer();
void InitGomer(GOMER* pgomer);
int  GetGomerSize();
void CloneGomer(GOMER* pgomer, GOMER* pgomerBase);
void RenderGomerSelf(GOMER* pgomer, CM* pcm, RO* pro);
void DeleteGomer(GOMER* pgomer);

class UBG;
UBG* NewUbg();
int  GetUbgSize();
void CloneUbg(UBG* pubg, UBG* pubgBase);
void DeleteUbg(UBG* pubg);

class MBG;
MBG* NewMbg();
void InitMbg(MBG* pmbg);
int  GetMbgSize();
void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis);
void CloneMbg(MBG* pmbg, MBG* pmbgBase);
void DeleteMbg(MBG* pmbg);

class BHG;
BHG* NewBhg();
void InitBhg(BHG* pbhg);
int  GetBhgSize();
void CloneBhg(BHG* pbhg, BHG* pbhgBase);
void DeleteBhg(BHG* phg);

class MURRAY;
MURRAY*NewMurray();
void InitMurray(MURRAY* pmurray);
int  GetMurraySize();
void CloneMurray(MURRAY* pmurray, MURRAY* pmurrayBase);
void DeleteMurray(MURRAY *pmurray);

class PUFFC;
PUFFC*NewPuffc();
int  GetPuffcSize();
void ClonePuffc(PUFFC* ppuffc, PUFFC* ppuffcBase);
void DeletePuffc(PUFFC *ppuffc);

class CRFOD;
CRFOD*NewCrfod();
void OnCrfodAdd(CRFOD* pcrfod);
void OnCrfodRemove(CRFOD* pcrfod);
void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase);
int  GetCrfodSize();
void DeleteCrfod(CRFOD *pcrfod);

class CRFODB;
CRFODB*NewCrfodb();
void InitCrfodb(CRFODB* pcrfodb);
int  GetCrfodbSize();
void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase);
void DeleteCrfodb(CRFODB *pcrfodb);

class CRFODK;
CRFODK*NewCrfodk();
int  GetCrfodkSize();
void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase);
void DeleteCrfodk(CRFODK * pcrfodk);

class UNK16;
void* NewUnk16();
void InitUnk16(UNK16* punk16);
void CloneUnk16(UNK16* punk16, UNK16* punk16Base);
void DeleteUnk16(UNK16* punk16);

class TANK;
TANK*NewTank();
void InitTank(TANK* ptank);
int  GetTankSize();
void CloneTank(TANK* ptank, TANK* ptankBase);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);
void DeleteTank(TANK *ptank);

class JP;
JP*  NewJp();
void InitJp(JP* pjp);
int  GetJpSize();
void CloneJp(JP* pjp, JP* pjpBase);
void RenderJpAll(JP* pjp, CM* pcm, RO* pro);
void RenderJpSelf(JP* pjp, CM* pcm, RO* pro);
void DeleteJp(JP *pjp);

class HG;
HG*  NewHg();
void InitHg(HG* phg);
int  GetHgSize();
void CloneHg(HG* phg, HG* phgBase);
void RenderHgSelf(HG* phg, CM* pcm, RO* pro);
void DeleteHg(HG *phg);

class MECHA;
MECHA*NewMecha();
void InitMecha(MECHA* pmecha);
int  GetMechaSize();
void CloneMecha(MECHA* pmecha, MECHA* pmechaBase);
void RenderMechaSelf(MECHA* pmecha, CM* pcm, RO* pro);
void DeleteMecha(MECHA* pmecha);

class ROV;
ROV* NewRov();
void InitRov(ROV* prov);
int  GetRovSize();
void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis);
void CloneRov(ROV* prov, ROV* provBase);
void DeleteRov(ROV *prov);

class TURRET;
TURRET*NewTurret();
int  GetTurretSize();
void CloneTurret(TURRET* pturret, TURRET* pturretBase);
void DeleteTurret(TURRET *pturret);

class VAULT;
VAULT*NewVault();
void InitVault(VAULT* pvault);
int  GetVaultSize();
void CloneVault(VAULT* pvault, VAULT* pvaultBase);
void DeleteVault(VAULT *pvault);

class PUFFER;
PUFFER*NewPuffer();
void InitPuffer(PUFFER* ppuffer);
void LoadPufferFromBrx(PUFFER* ppuffer, CBinaryInputStream* pbis);
void ClonePuffer(PUFFER* ppuffer, PUFFER* ppufferBase);
int  GetPufferSize();
void DeletePuffer(PUFFER *ppuffer);

class MGV;
MGV* NewMgv();
void InitMgv(MGV* pmgv);
int  GetMgvSize();
void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis);
void CloneMgv(MGV* pmgv, MGV* pmgvBase);
void DeleteMgv(MGV *pmgv);

class SUV;
SUV* NewSuv();
void InitSuv(SUV* psuv);
int  GetSuvSize();
void UpdateSuvXfWorld(SUV* psuv);
void CloneSuv(SUV* psuv, SUV* psuvBase);
void RenderSuvSelf(SUV* psuv, CM* pcm, RO* pro);
void DeleteSuv(SUV *psuv);

class CYCLE;
CYCLE*NewCycle();
void InitCycle(CYCLE* pcycle);
int  GetCycleSize();
void UpdateCycleXfWorld(CYCLE* pcycle);
void CloneCycle(CYCLE* pcycle, CYCLE* pcycleBase);
void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro);
void DeleteCycle(CYCLE *pcycle);

class LGN;
LGN* NewLgn();
void InitLgn(LGN* plgn);
int  GetLgnSize();
void CloneLgn(LGN* plgn, LGN* plgnBase);
void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro);
void DeleteLgn(LGN *plgn);

class UNK0;
void*NewUnk0();
void InitUnk0(UNK0* punk0);
void CloneUnk0(UNK0* punk0, UNK0* punk0Base);
void DeleteUnk0(UNK0* punk0);

class UNK1;
void*NewUnk1();
void LoadUnk1FromBrx(UNK1* punk1, CBinaryInputStream* pbis);
void InitUnk1(UNK1* punk1);
void CloneUnk1(UNK1* punk1, UNK1* punk1Base);
void DeleteUnk1(UNK1* punk1);

class UNK2;
void*NewUnk2();
void InitUnk2(UNK2* punk2);
void CloneUnk2(UNK2* punk2, UNK2* punk2Base);
void DeleteUnk2(UNK2* punk2);

class UNK3;
void*NewUnk3();
void CloneUnk3(UNK3* punk3, UNK3* punk3Base);
void DeleteUnk3(UNK3* punk3);

class JACK;
JACK*NewJack();
void InitJack(JACK* pjack);
int  GetJackSize();
void CloneJack(JACK* pjack, JACK* pjackBase);
void DeleteJack(JACK *pjack);

class RIPG;
RIPG*NewRipg();
void InitRipg(RIPG* pripg);
int  GetRipgSize();
void CloneRipg(RIPG* pripg, RIPG* pripgBase);
void RenderRipgSelf(RIPG* pripg, CM* pcm, RO* pro);
void DeleteRipg(RIPG *pripg);

class WATER;
WATER*NewWater();
void InitWater(WATER* pwater);
int  GetWaterSize();
void CloneWater(WATER* pwater, WATER* pwaterBase);
void DeleteWater(WATER *pwater);

class BRK;
BRK* NewBrk();
void InitBrk(BRK* pbrk);
int  GetBrkSize();
void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis);
void CloneBrk(BRK* pbrk, BRK* pbrkBase);
void DeleteBrk(BRK *pbrk);

class BREAK;
BREAK* NewBreak();
void InitBreak(BREAK* pbreak);
int  GetBreakSize();
void CloneBreak(BREAK* pbreak, BREAK* pbreakBase);
void DeleteBreak(BREAK* pbreak);

class ALBRK;
ALBRK*NewAlbrk();
int  GetAlbrkSize();
void CloneAlbrk(ALBRK* palbrk, ALBRK* palbrkBase);
void DeleteAlbrk(ALBRK* palbrk);

class CAN;
CAN* NewCan();
void InitCan(CAN* pcan);
int  GetCanSize();
void CloneCan(CAN* pcan, CAN* pcanBase);
void DeleteCan(CAN *pcan);

class DARTGUN;
DARTGUN*NewDartgun();
void InitDartgun(DARTGUN* pdartgun);
int  GetDartgunSize();
void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase);
void BindDartgun(DARTGUN* pdartgun);
void DeleteDartgun(DARTGUN *pdartgun);

class SWP;
SWP* NewSwp();
void InitSwp(SWP* pswp);
int  GetSwpSize();
void CloneSwp(SWP* pswp, SWP* pswpBase);
void DeleteSwp(SWP *pswp);

class FRAGILE;
FRAGILE* NewFragile();
void InitFragile(FRAGILE* pfragile);
int  GetFragileSize();
void CloneFragile(FRAGILE* pfragile, FRAGILE* pfragileBase);
void DeleteFragile(FRAGILE* pfragile);

class ZAPBREAK;
ZAPBREAK*NewZapbreak();
int  GetZapbreakSize();
void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase);
void DeleteZapbreak(ZAPBREAK* pzapbreak);

class BRKP;
BRKP*NewBrkp();
int  GetBrkpSize();
void CloneBrkp(BRKP* prkp, BRKP* prkpBase);
void DeleteBrkp(BRKP* pbrkp);

class BUTTON;
BUTTON*NewButton();
void InitButton(BUTTON* pbutton);
int  GetButtonSize();
void CloneButton(BUTTON* pbutton, BUTTON* pbuttonBase);
void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis);
void DeleteButton(BUTTON *pbutton);

class VOLBTN;
VOLBTN*NewVolbtn();
void InitVolbtn(VOLBTN* pvolbtn);
int  GetVolbtnSize();
void CloneVolbtn(VOLBTN* pvolbtn, VOLBTN* pvolbtnBase);
void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis);
void DeleteVolbtn(VOLBTN *pvolbtn);

class JLOVOL;
JLOVOL*NewJlovol();
void InitJlovol(JLOVOL* pjlovol);
int  GetJlovolSize();
void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase);
void BindJlovol(JLOVOL* pjlovol);
void DeleteJlovol(JLOVOL *pjlovol);

class SQUISH;
SQUISH*NewSquish();
int  GetSquishSize();
void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis);
void CloneSquish(SQUISH* psquish, SQUISH* psquishBase);
void RenderSquishSelf(SQUISH* psquish, CM* pcm, RO* pro);
void DeleteSquish(SQUISH *psquish);

class BONE;
BONE* NewBone();
int  GetBoneSize();
void CloneBone(BONE* pbone, BONE* pboneBase);
void DeleteBone(BONE* pbone);

class UNK17;
void*NewUnk17();
void CloneUnk17(UNK17* punk17, UNK17* punk17Base);
void DeleteUnk17(UNK17* punk17);

class SPRIZE;
SPRIZE*NewSprize();
void InitSprize(SPRIZE* psprize);
int  GetSprizeSize();
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);
void CloneSprize(SPRIZE* psprize, SPRIZE* psprizeBase);
void BindSprize(SPRIZE* psprize);
void DeleteSprize(SPRIZE* psprize);

class SCPRIZE;
SCPRIZE*NewScprize();
void InitScprize(SCPRIZE* pscprize);
int  GetScprizeSize();
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
void DeleteScprize(SCPRIZE *pscprize);

class LIFETKN;
LIFETKN*NewLifetkn();
int  GetLifetknSize();
void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase);
void DeleteLifetkn(LIFETKN *plifetkn);

class CLUE;
CLUE*NewClue();
void InitClue(CLUE* pclue);
int  GetClueSize();
void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis);
void CloneClue(CLUE* pclue, CLUE* pclueBase);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);
void DeleteClue(CLUE *pclue);

class ALARM;
ALARM* NewAlarm();
void InitAlarm(ALARM* palarm);
int  GetAlarmSize();
void CloneAlarm(ALARM* palarm, ALARM* palarmBase);
void DeleteAlarm(ALARM* palarm);

class SENSOR;
SENSOR*NewSensor();
void InitSensor(SENSOR *psensor);
int  GetSensorSize();
void CloneSensor(SENSOR *psensor, SENSOR *psensorBase);
void DeleteSensor(SENSOR *psensor);

// Laser Sensor
class LASEN;
LASEN*NewLasen();
void InitLasen(LASEN* plasen);
int  GetLasenSize();
void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis);
void CloneLasen(LASEN* plasen, LASEN* plasenBase);
void BindLasen(LASEN* plasen);
void RenderLasenSelf(LASEN* plasen, CM* pcm, RO* pro);
void DeleteLasen(LASEN *plasen);

class CAMSEN;
CAMSEN*NewCamsen();
void InitCamsen(CAMSEN* pcamsen);
int  GetCamsenSize();
void CloneCamsen(CAMSEN* pcamsen, CAMSEN* pcamsenBase);
void RenderCamsenSelf(CAMSEN* pcamsen, CM* pcm, RO* pro);
void DeleteCamsen(CAMSEN *pcamsen);

class PRSEN;
PRSEN*NewPrsen();
void InitPrsen(PRSEN* pprsen);
int  GetPrsenSize();
void ClonePrsen(PRSEN* pprsen, PRSEN* pprsenBase);
void DeletePrsen(PRSEN *ppprsen);

class BARRIER;
BARRIER*NewBarrier();
void InitBarrier(BARRIER* pbarrier);
int  GetBarrierSize();
void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase);
void DeleteBarrier(BARRIER* pbarrier);

class IKH;
IKH* NewIkh();
int  GetIkhSize();
void CloneIkh(IKH* pikh, IKH* pikhBase);
void RenderIkhSelf(IKH* pikh, CM* pcm, RO* pro);
void DeleteIkh(IKH *pikh);

class TZP;
TZP* NewTzp();
void InitTzp(TZP* ptzp);
int  GetTzpSize();
void CloneTzp(TZP* ptzp, TZP* ptzpBase);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);
void DeleteTzp(TZP *ptzp);

class VOLZP;
VOLZP*NewVolzp();
void InitVolzp(VOLZP* pvolzp);
int  GetVolzpSize();
void CloneVolzp(VOLZP* pvolzp, VOLZP* pvolzpBase);
void DeleteVolzp(VOLZP *pvolzp);

// Convo
class CNVO;
CNVO*NewCnvo();
void InitCnvo(CNVO* pcnvo);
int  GetCnvoSize();
void LoadCnvoFromBrx(CNVO *pcnvo, CBinaryInputStream *pbis);
void CloneCnvo(CNVO* pcnvo, CNVO* pcnvoBase);
void DeleteCnvo(CNVO *pcnvo);

class HBSK;
HBSK*NewHbsk();
void InitHbsk(HBSK* phbsk);
void OnHbskAdd(HBSK* phbsk);
void OnHbskRemove(HBSK* phbsk);
int  GetHbskSize();
void CloneHbsk(HBSK* phbsk, HBSK* phbskBase);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);
void DeleteHbsk(HBSK *phbsk);

class BOMB;
BOMB*NewBomb();
void InitBomb(BOMB* pbomb);
void OnBombAdd(BOMB* pbomb);
int  GetBombSize();
void CloneBomb(BOMB* pbomb, BOMB* pbombBase);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);
void DeleteBomb(BOMB *pbomb);

class MISSILE;
MISSILE*NewMissile();
void InitMissile(MISSILE* pmissile);
int  GetMissileSize();
void OnMissileRemove(MISSILE* pmissile);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void CloneMissile(MISSILE* pmissile, MISSILE* pmissileBase);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);
void DeleteMissile(MISSILE *pmissile);

class ACCMISS;
ACCMISS*NewAccmiss();
void InitAccmiss(ACCMISS* paccmiss);
int  GetAccmissSize();
void CloneAccmiss(ACCMISS* paccmiss, ACCMISS* paccmissBase);
void DeleteAccmiss(ACCMISS * paccmiss);

class TARMISS;
TARMISS*NewTarmiss();
void InitTarmiss(TARMISS* ptarmiss);
int  GetTarmissSize();
void CloneTarmiss(TARMISS* ptarmiss, TARMISS* ptarmissBase);
void DeleteTarmiss(TARMISS *ptarmiss);

class SPLMISS;
SPLMISS*NewSplmiss();
int  GetSplmissSize();
void CloneSplmiss(SPLMISS* psplmiss, SPLMISS* psplmissBase);
void DeleteSplmiss(SPLMISS * psplmiss);

class GROUNDMISS;
GROUNDMISS*NewGroundmiss();
void InitGroundmiss(GROUNDMISS* pgroundmiss);
int  GetGroundmissSize();
void CloneGroundmiss(GROUNDMISS *pgroundmiss, GROUNDMISS *pgroundmissBase);
void DeleteGroundmiss(GROUNDMISS *pgroundmiss);

class FLY;
FLY* NewFly();
void InitFly(FLY* pfly);
int  GetFlySize();
void CloneFly(FLY* pfly, FLY* pflyBase);
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);
void RenderFlySelf(FLY* pfly, CM* pcm, RO* pro);
void DeleteFly(FLY *pfly);

class RAT;
RAT* NewRat();
void InitRat(RAT* prat);
int  GetRatSize();
void OnRatAdd(RAT* prat);
void OnRatRemove(RAT* prat);
void CloneRat(RAT* prat, RAT* pratBase);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);
void RenderRatAll(RAT* prat, CM* pcm, RO* pro);
void DeleteRat(RAT *prat);

class ROH;
ROH* NewRoh();
void InitRoh(ROH* proh);
int  GetRohSize();
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);
void CloneRoh(ROH* proh, ROH* prohBase);
void DeleteRoh(ROH *proh);

class ROC;
ROC* NewRoc();
void InitRoc(ROC* proc);
int  GetRocSize();
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);
void CloneRoc(ROC* proc, ROC* procBase);
void DeleteRoc(ROC *proc);

class ROST;
ROST*NewRost();
void InitRost(ROST* prost);
int  GetRostSize();
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);
void CloneRost(ROST* prost, ROST* prostBase);
void DeleteRost(ROST *prost);

class ROP;
ROP* NewRop();
void InitRop(ROP* prop);
int  GetRopSize();
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);
void CloneRop(ROP* prop, ROP* probBase);
void DeleteRop(ROP *prop);

class DART;
DART*NewDart();
void InitDart(DART* pdart);
int  GetDartSize();
void OnDartAdd(DART* pdart);
void RemoveDart(DART* pdart);
void CloneDart(DART* pdart, DART* pdartBase);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);
void DeleteDart(DART *pdart);

class UBV;
UBV* NewUbv();
int  GetUbvSize();
void CloneUbv(UBV* pubv, UBV* pubvBase);
void DeleteUbv(UBV *pubv);

class UBP;
UBP* NewUbp();
void InitUbg(UBG* pubg);
int  GetUbpSize();
void CloneUbp(UBP* pubp, UBP* pubpBase);
void RenderUbpAll(UBP *pubp, CM *pcm, RO *pro);
void DeleteUbp(UBP *pubp);

class DSP;
DSP* NewDsp();
int  GetDspSize();
void CloneDsp(DSP* pdsp, DSP* pdspBase);
void DeleteDsp(DSP *pdsp);

class JLO;
JLO* NewJlo();
void InitJlo(JLO* pjlo);
int  GetJloSize();
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void CloneJlo(JLO* pjlo, JLO* pjloBase);
void DeleteJlo(JLO *pjlo);

class PUFFT;
PUFFT*NewPufft();
int  GetPufftSize();
void ClonePufft(PUFFT* ppufft, PUFFT* ppufftBase);
void DeletePufft(PUFFT *ppufft);

class MRKV;
MRKV*NewMrkv();
void InitMrkv(MRKV* pmrkv);
int  GetMrkvSize();
void CloneMrkv(MRKV* pmrkv, MRKV* pmrkvBase);
void DeleteMrkv(MRKV *pmrkv);

class UNK4;
void*NewUnk4();
void InitUnk4(UNK4* punk4);
void CloneUnk4(UNK4* punk4, UNK4* punk4Base);
void DeleteUnk4(UNK4* punk4);

class UNK5;
void* NewUnk5();
void InitUnk5(UNK5* punk5);
void CloneUnk5(UNK5* punk5, UNK5* punk5Base);
void DeleteUnk5(UNK5* punk5);

class UNK6;
void* NewUnk6();
void CloneUnk6(UNK6* punk6, UNK6* punk6Base);
void DeleteUnk6(UNK6* punk6);

class LGNB;
LGNB*NewLgnb();
void InitLgnb(LGNB* plgnb);
int  GetLgnbSize();
void CloneLgnb(LGNB* plgnb, LGNB* plgnbBase);
void DeleteLgnb(LGNB* plgnb);

class BLIPG;
BLIPG*NewBlipg();
void InitBlipg(BLIPG* pblipg);
int  GetBlipgSize();
void CloneBlipg(BLIPG* pblipg, BLIPG* pblipgBase);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);
void DeleteBlipg(BLIPG* pblipg);

class CAMERA;
CAMERA*NewCamera();
void InitCamera(CAMERA *pcamera);
void CloneCamera(CAMERA *pcamera, CAMERA *pcameraBase);
int  GetCameraSize();
void DeleteCamera(CAMERA *pcamera);

class LBONE;
LBONE*NewLBone();
int  GetLBoneSize();
void CloneLBone(LBONE* plbone, LBONE* plboneBase);
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);
void DeleteLBone(LBONE *plbone);

class EMITTER;
struct EMITB;
enum ENSK;
EMITTER*NewEmitter();
void InitEmitter(EMITTER* pemitter);
int  GetEmitterSize();
void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);
void BindEmitter(EMITTER* pemitter);
void DeleteEmitter(EMITTER *pemitter);

class LIGHT;
LIGHT*NewLight();
void InitLight(LIGHT* plight);
int  GetLightSize();
void OnLightAdd(LIGHT* plight);
void OnLightRemove(LIGHT* plight);
void UpdateLightXfWorldHierarchy(LIGHT* plight);
void CloneLight(LIGHT* plight, LIGHT* plightBase);
void FitRecipFunction(float x0, float y0, float x1, float y1, float* pdu, float* pru);
void*GetLightKind(LIGHT* plight);
void SetLightKind(LIGHT* plight, LIGHTK lightk);
void*GetLightHighlightColor(LIGHT* plight);
void SetLightHighlightColor(LIGHT* plight, glm::vec3& pvecHighlight);
void*GetLightMidtoneStrength(LIGHT* plight);
void SetLightMidtoneStrength(LIGHT* plight, float gMidtone);
void*GetLightShadowStrength(LIGHT* plight);
void SetLightShadowStrength(LIGHT* plight, float gShadow);
void*GetLightHighlightAngle(LIGHT* plight);
void SetLightHighlightAngle(LIGHT* plight, float degHighlight);
void*GetLightMidtoneAngle(LIGHT* plight);
void SetLightMidtoneAngle(LIGHT* plight, float degMidtone);
void*GetLightShadowAngle(LIGHT* plight);
void SetLightShadowAngle(LIGHT* plight, float degShadow);
void*GetLightDirection(LIGHT* plight);
void SetLightDirection(LIGHT* plight, glm::vec3& pvecDirection);
void*GetLightDynamic(LIGHT* plight);
void SetLightDynamic(LIGHT* plight, int fDynamic);
void*GetLightFallOff(LIGHT* plight);
void SetLightFallOff(LIGHT* plight, LM* plm);
void*GetLightConeAngle(LIGHT* plight);
void SetLightConeAngle(LIGHT* plight, float degCone);
void*GetLightHotSpotAngle(LIGHT* plight);
void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot);
void*GetLightFrustrumUp(LIGHT* plight);
void SetLightFrustrumUp(LIGHT* plight, glm::vec3& pvecUpLocal);
void DeleteLight(LIGHT *plight);
void DeallocateLightVector();

class LIKH;
LIKH*NewLikh();
int  GetLikhSize();
void CloneLikh(LIKH* plikh, LIKH* plikhBase);
void RenderLikhSelf(LIKH* plikh, CM* pcm, RO* pro);
void DeleteLikh(LIKH *plikh);

// Check Point
class CHKPNT;
CHKPNT*NewChkpnt();
void InitChkpnt(CHKPNT* pchkpnt);
int  GetChkpntSize();
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase);
void BindChkpnt(CHKPNT* pchkpnt);
void DeleteChkpnt(CHKPNT* pchkpnt);

PROXY*NewProxy();
void InitProxy(PROXY* pproxy);
int  GetProxySize();
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);
void CloneProxy(PROXY* pproxy, PROXY* pproxyBase);
void PostProxyLoad(PROXY* pproxy);
void DeleteProxy(PROXY *pproxy);

class SKY;
SKY* NewSky();
int  GetSkySize();
void CloneSky(SKY* psky, SKY* pskyBase);
void UpdateSky(SKY* psky, float dt);
void DeleteSky(SKY* psky);

class DPRIZE;
DPRIZE*NewDprize();
void InitDprize(DPRIZE* pdprize);
int  GetDprizeSize();
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);
void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase);
void DeleteDprize(DPRIZE *pdprize);

class CHARM;
CHARM*NewCharm();
void InitCharm(CHARM* pcharm);
int  GetCharmSize();
void CloneCharm(CHARM* pcharm, CHARM* pcharmBase);
void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro);
void DeleteCharm(CHARM* pcharm);

class COIN;
COIN*NewCoin();
void InitCoin(COIN* pcoin);
int  GetCoinSize();
void CloneCoin(COIN* pcoin, COIN* pcoinBase);
void DeleteCoin(COIN *pcoin);

LIFETKN* NewLifetkn();
int  GetLifetknSize();
void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase);
void DeleteLifetkn(LIFETKN* plifetkn);

class KEY;
KEY* NewKey();
void InitKey(KEY* pkey);
int  GetKeySize();
void CloneKey(KEY* pkey, KEY* pkeyBase);
void DeleteKey(KEY* pkey);

class GOLD;
GOLD*NewGold();
void InitGold(GOLD* pgold);
int  GetGoldSize();
void CloneGold(GOLD* pgold, GOLD* pgoldBase);
void DeleteGold(GOLD* pgold);


class UNK7;
void*NewUnk7();
void InitUnk7(UNK7* punk7);
void CloneUnk7(UNK7* punk7, UNK7* punk7Base);
void DeleteUnk7(UNK7* punk7);


class LOCK;
LOCK*NewLock();
int  GetLockSize();
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void CloneLock(LOCK* plock, LOCK* plockBase);
void DeleteLock(LOCK* plock);

class LOCKG;
LOCKG*NewLockg();
int  GetLockgSize();
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void CloneLockg(LOCKG* plockg, LOCKG* plockgBase);
void DeleteLockg(LOCKG* plockg);

class TAIL;
TAIL*NewTail();
void InitTail(TAIL* ptail);
int  GetTailSize();
void CloneTail(TAIL* ptail, TAIL* ptailBase);
void DeleteTail(TAIL* ptail);

class ROB;
ROB* NewRob();
void InitRob(ROB* prob);
int  GetRobSize();
void CloneRob(ROB* prob, ROB* probBase);
void BindRob(ROB* prob);
void DeleteRob(ROB *prob);

class FLASH;
FLASH*NewFlash();
void InitFlash(FLASH* pflash);
int  GetFlashSize();
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);
void CloneFlash(FLASH* pflash, FLASH* pflashBase);
void RenderFlashSelf(FLASH* pflash, CM* pcm, RO* pro);
void DeleteFlash(FLASH *pflash);

class DYSH;
DYSH*NewDysh();
void InitDysh(DYSH* pdysh);
int  GetDyshSize();
void CloneDysh(DYSH* pdysh, DYSH* pdyshBase);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);
void DeleteDysh(DYSH* pdysh);

class SCENTMAP;
SCENTMAP*NewScentmap();
void InitScentmap(SCENTMAP* pscentmap);
int  GetScentmapSize();
void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase);
void DeleteScentmap(SCENTMAP* pscentmap);

class WAYPOINT;
WAYPOINT*NewWaypoint();
void InitWaypoint(WAYPOINT* pwaypoint);
int  GetWaypointSize();
void CloneWaypoint(WAYPOINT* pwaypoint, WAYPOINT* pwaypointBase);
void DeleteWaypoint(WAYPOINT* pwaypoint);

class TN;
TN*  NewTn();
void InitTn(TN* ptn);
int  GetTnSize();
void OnTnRemove(TN* ptn);
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void CloneTn(TN* ptn, TN* ptnBase);
void LoadTbspFromBrx(CBinaryInputStream* pbis);
void RenderTnSelf(TN* ptn, CM* pcm, RO* pro);
void DeleteTn(TN* ptn);

class JLOC;
JLOC*NewJloc();
void InitJloc(JLOC* pjloc);
int  GetJlocSize();
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);
void CloneJloc(JLOC* pjloc, JLOC* pjlocBase);
void DeleteJloc(JLOC* pjloc);

class DIALOG;
DIALOG*NewDialog();
void InitDialog(DIALOG* pdialog);
int  GetDialogSize();
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void CloneDialog(DIALOG* pdialog, DIALOG* pdialogBase);
void DeleteDialog(DIALOG* pdialog);

class SPEAKER;
SPEAKER*NewSpeaker();
void InitSpeaker(SPEAKER* pspeaker);
int  GetSpeakerSize();
void CloneSpeaker(SPEAKER* pspeaker, SPEAKER* pspeakerBase);
void DeleteSpeaker(SPEAKER* pspeaker);

class ROPE;
ROPE*NewRope();
void InitRope(ROPE* prope);
int  GetRopeSize();
void CloneRope(ROPE* prope, ROPE* propeBase);
void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro);
void DeleteRope(ROPE* prope);

class WM;
WM*  NewWm();
int  GetWmSize();
void CloneWm(WM* pwm, WM* pwmBase);
void BindWm(WM* pwm);
void RenderWmAll(WM* pwm, CM* pcm, RO* pro);
void DeleteWm(WM* pwm);

class UNK8;
void*NewUnk8();
void InitUnk8(UNK8* punk8);
void CloneUnk8(UNK8* punk8, UNK8* punk8Base);
void DeleteUnk8(UNK8* punk8);

class UNK9;
void* NewUnk9();
void CloneUnk9(UNK9* punk9, UNK9* punk9Base);
void DeleteUnk9(UNK9* punk9);

class UNK10;
void*NewUnk10();
void CloneUnk10(UNK10* punk10, UNK10* punk10Base);
void DeleteUnk10(UNK10* punk10);

class UNK11;
void*NewUnk11();
void InitUnk11(UNK11* punk11);
void CloneUnk11(UNK11* punk11, UNK11* punk11Base);
void DeleteUnk11(UNK11* punk11);

class PUFFB;
PUFFB*NewPuffb();
int  GetPuffbSize();
void ClonePuffb(PUFFB* ppuffb, PUFFB* ppuffbBase);
void DeletePuffb(PUFFB* ppuffb);

class CRBRAIN;
CRBRAIN*NewCrbrain();
void InitCrbrain(CRBRAIN* pcrbrain);
int  GetCrbrainSize();
void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase);
void DeleteCrbrain(CRBRAIN* pcrbrain);

class MGC;
MGC* NewMgc();
void InitMgc(MGC* pmgc);
int  GetMgcSize();
void CloneMgc(MGC* pmgc, MGC* pmgcBase);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);
void DeleteMgc(MGC* pmgc);

class JACKB;
JACKB*NewJackb();
void InitJackb(JACKB* pjackb);
int  GetJackbSize();
void CloneJackb(JACKB* pjackb, JACKB* pjackbBase);
void DeleteJackb(JACKB* pjackb);

class JACKN;
JACKN*NewJackn();
void InitJackn(JACKN* pjackn);
int  GetJacknSize();
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);
void CloneJackn(JACKN* pjackn, JACKN* pjacknBase);
void UpdateJackn(JACKN* pjackn, float dt);
void RenderJacknSelf(JACKN* pjackn, CM* pcm, RO* pro);
void DeleteJackn(JACKN* pjackn);

class JACKF;
JACKF*NewJackf();
void InitJackf(JACKF* pjackf);
int  GetJackfSize();
void CloneJackf(JACKF* pjackf, JACKF* pjackfBase);
void RenderJackfSelf(JACKF* pjackf, CM* pcm, RO* pro);
void DeleteJackf(JACKF* pjackf);

// Static World
class SW;
SW*  NewSw();
void InitSw(SW* psw);
int  GetSwSize();
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);
void GetSwParams(SW* psw, SOP** ppsop);
void*GetSwIllum(SW *psw);
void SetSwIllum(SW* psw, float uMidtone);
void*GetSwIllumShadow(SW* psw);
void SetSwIllumShadow(SW* psw, float uShadow);
void*GetSwDarken(SW* psw);
void SetSwDarken(SW* psw, float rDarken);
void*GetSwDarkenSmooth(SW* psw);
void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth);
LO*  PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext);
int  CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo);
void UpdateSw(SW* psw, float dt);
void DeleteSw(SW* psw);

// Camera
CM*  NewCm();
void InitCm(CM* pcm);
int  GetCmSize();
void CloneCm(CM* pcm, CM* pcmBase);
void RecalcCm(CM* pcm);
void BuildCmFgfn(CM* pcm, float uFog, FGFN* pfgfn);
void SetSwCameraFov(SW* psw, float radFOV);
void SetSwCameraNearClip(SW* psw, float sNearClip);
void SetSwCameraFarClip(SW* psw, float sFarClip);
void SetSwCameraNearFog(SW* psw, float sNearFog);
void SetSwCameraFarFog(SW* psw, float sFarFog);
void SetSwCameraUFogMax(SW* psw, float uFogMax);
void SetSwCameraRgbaFog(SW* psw, RGBA prgbaFog);
void SetSwCameraMrdRatio(SW* psw, float rMRD);
void*GetCmPos(CM* pcm);
void SetCmPos(CM* pcm, glm::vec3 *ppos);
void*GetCmMat(CM* pcm);
void SetCmMat(CM* pcm, glm::mat3 *pmat);
void*GetCmFov(CM* pcm);
void SetCmFov(CM* pcm, float radFOV);
void*GetCmNearClip(CM* pcm);
void SetCmNearClip(CM* pcm, float sNearClip);
void*GetCmFarClip(CM* pcm);
void SetCmFarClip(CM* pcm, float sFarClip);
void*GetCmNearFog(CM* pcm);
void SetCmNearFog(CM* pcm, float sNearFog);
void*GetCmFarFog(CM* pcm);
void SetCmFarFog(CM* pcm, float sFarFog);
void*GetCmUFogMax(CM* pcm);
void SetCmUFogMax(CM* pcm, float uFogMax);
void*GetCmRgbaFog(CM* pcm);
void SetCmRgbaFog(CM* pcm, RGBA prgbaFog);
void*GetCmMrdRatio(CM* pcm);
void SetCmMrdRatio(CM* pcm, float rMRD);
void DeleteCm(CM* pcm);

// Global pointer to SW camera object
extern inline CM* g_pcm = nullptr;

class SHAPE;
SHAPE*NewShape();
void InitShape(SHAPE* pshape);
int  GetShapeSize();
void CloneShape(SHAPE* pshape, SHAPE* pshapeBase);
void SetShapeParent(SHAPE* pshape, ALO* paloParent);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);
void DeleteShape(SHAPE* pshape);

class HSHAPE;
HSHAPE*NewHshape();
void InitHshape(HSHAPE* phshape);
int  GetHshapeSize();
void OnHshapeAdd(HSHAPE* phshape);
void OnHshapeRemove(HSHAPE* phshape);
void CloneHshape(HSHAPE* phshape, HSHAPE* phshapeBase);
void BindHshape(HSHAPE* phshape);
void DeleteHshape(HSHAPE* phshape);

class PIPE;
PIPE*NewPipe();
void InitPipe(PIPE* ppipe);
int  GetPipeSize();
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);
void ClonePipe(PIPE* ppipe, PIPE* ppipeBase);
void DeletePipe(PIPE* ppipe);

class RAIL;
RAIL*NewRail();
int  GetRailSize();
void OnRailAdd(RAIL* prail);
void OnRailRemove(RAIL* prail);
void CloneRail(RAIL* prail, RAIL* prailBase);
void DeleteRail(RAIL* prail);

class LANDING;
LANDING*NewLanding();
int  GetLandingSize();
void OnLandingAdd(LANDING* planding);
void OnLandingRemove(LANDING* planding);
void CloneLanding(LANDING* planding, LANDING* plandingBase);
void DeleteLanding(LANDING* planding);

class XFM;
XFM* NewXfm();
void InitXfm(XFM* pxfm);
int  GetXfmSize();
void CloneXfm(XFM* pxfm, XFM* pxfmBase);
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);
void SetXfmParent(XFM* pxfm, ALO* paloParent);
void ApplyXfmProxy(XFM* pxfm, PROXY* pproxyApply);
void DeleteXfm(XFM* pxfm);

class WARP;
WARP*NewWarp();
int  GetWarpSize();
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);
void CloneWarp(WARP* pwarp, WARP* pwarpBase);
void DeleteWarp(WARP* pwarp);

class TARGET;
TARGET*NewTarget();
void InitTarget(TARGET* ptarget);
int  GetTargetSize();
void OnTargetAdd(TARGET* ptarget);
void OnTargetRemove(TARGET* ptarget);
void CloneTarget(TARGET* ptarget, TARGET* ptargetBase);
void DeleteTarget(TARGET* ptarget);

class HND;
HND* NewHnd();
void InitHnd(HND* phnd);
int  GetHndSize();
void CloneHnd(HND* phnd, HND* phndBase);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
void DeleteHnd(HND* phnd);

class EXPL;
EXPL*NewExpl();
int  GetExplSize();
void CloneExpl(EXPL* pexpl, EXPL* pexplBase);
void DeleteExpl(EXPL* pexpl);

class EXPLG;
EXPLG*NewExplg();
int  GetExplgSize();
void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase);
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void BindExplg(EXPLG* pexplg);
void DeleteExplg(EXPLG* pexplg);

class EXPLO;
EXPLO*NewExplo();
int  GetExploSize();
void InitExplo(EXPLO* pexplo);
void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase);
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);
void BindExplo(EXPLO* pexplo);
void DeleteExplo(EXPLO* pexplo);

class EXPLS;
EXPLS*NewExpls();
int  GetExplsSize();
void InitExpls(EXPLS* pexpls);
void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase);
void BindExpls(EXPLS* pexpls);
void DeleteExpls(EXPLS* pexpls);

class VOL;
VOL* NewVol();
int  GetVolSize();
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);
void CloneVol(VOL* pvol, VOL* pvolBase);
void DeleteVol(VOL* pvol);

class RATHOLE;
RATHOLE*NewRathole();
int  GetRatholeSize();
void OnRatholeAdd(RATHOLE* prathole);
void OnRatholeRemove(RATHOLE* prathole);
void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase);
void DeleteRathole(RATHOLE* prathole);

class PUFFV;
PUFFV*NewPuffv();
int  GetPuffvSize();
void InitPuffv(PUFFV* ppuffv);
void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase);
void DeletePuffv(PUFFV* ppuffv);

class EXIT;
EXIT*NewExit();
int  GetExitSize();
void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis);
void CloneExit(EXIT* pexit, EXIT* pexitBase);
void DeleteExit(EXIT* pexit);

class PNT;
PNT* NewPnt();
int  GetPntSize();
void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis);
void ClonePnt(PNT* pnt, PNT* pntBase);
void SetPntParent(PNT* ppnt, ALO* paloParent);
void ApplyPntProxy(PNT* ppnt, PROXY* pproxyApply);
void DeletePnt(PNT* ppnt);

class HPNT;
HPNT*NewHpnt();
void InitHpnt(HPNT* phpnt);
int  GetHpntSize();
void OnHpntAdd(HPNT* phpnt);
void OnHpntRemove(HPNT* phpnt);
void CloneHpnt(HPNT* phpnt, HPNT* phpntBase);
void BindHpnt(HPNT* phpnt);
void DeleteHpnt(HPNT* phpnt);

class JMT;
JMT* NewJmt();
int  GetJmtSize();
void OnJmtAdd(JMT* pjmt);
void OnJmtRemove(JMT* pjmt);
void CloneJmt(JMT* pjmt, JMT* pjmtBase);
void DeleteJmt(JMT* pjmt);

class SPIRE;
SPIRE*NewSpire();
int  GetSpireSize();
void OnSpireAdd(SPIRE* pspire);
void OnSpireRemove(SPIRE* pspire);
void CloneSpire(SPIRE* pspire, SPIRE* pspireBase);
void DeleteSpire(SPIRE* pspire);

class SCAN;
SCAN*NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void LoadScanFromBrx(SCAN* pscan, CBinaryInputStream* pbis);
void DeleteScan(SCAN* pscan);

// Animation Segment
class ASEG;
ASEG*NewAseg();
void InitAseg(ASEG* paseg);
int  GetAsegSize();
void CloneAseg(ASEG* paseg, ASEG* pasegBase);
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void DeleteAseg(ASEG* paseg);

class ASEGBL;
void*NewAsegbl();
void DeleteAsegbl(LO* plo);

class MAP;
MAP* NewMap();
void CloneMap(MAP* pmap, MAP* pmapBase);
int  GetMapSize();
void DeleteMap(MAP* pmap);

// Visibility Map
class VISMAP;
VISMAP*NewVismap();
void InitVismap(VISMAP* pvismap);
int  GetVismapSize();
void LoadVismapFromBrx(VISMAP *pvismap, CBinaryInputStream *pbis);
void ClipVismapPointNoHop(VISMAP *pvismap, glm::vec3 *ppos, GRFZON *pgrfzon);
void DeleteVismap(VISMAP* pvismap);

class FRZG;
FRZG*NewFrzg();
int  GetFrzgSize();
void CloneFrzg(FRZG* pfrzg, FRZG* pfrzgBase);
void DeleteFrzg(FRZG* pfrzg);

class SM;
SM*  NewSm();
int  GetSmSize();
void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis);
void DeleteSm(SM* psm);

class SGG;
SGG* NewSgg();
void InitSgg(SGG* psgg);
int  GetSggSize();
void CloneSgg(SGG* psgg, SGG* psggBase);
void BindSgg(SGG* psgg);
void DeleteSgg(SGG* psgg);

// AI Path's
class PATHZONE;
PATHZONE*NewPathzone();
int  GetPathzoneSize();
void OnPathzoneAdd(PATHZONE* ppathzone);
void OnPathzoneRemove(PATHZONE* ppathzone);
void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);
void DeletePathzone(PATHZONE* ppathzone);

// Reach Map
class RCHM;
RCHM*NewRchm();
void InitRchm(RCHM* prchm);
int  GetRchmSize();
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);
void DeleteRchm(RCHM* prchm);

class RWM;
RWM* NewRwm();
void InitRwm(RWM* prwm);
int  GetRwmSize();
void DeleteRwm(RWM* prwm);

class WR;
WR*  NewWr();
void InitWr(WR* pwr);
int  GetWrSize();
void DeleteWr(WR* pwr);

class KEYHOLE;
KEYHOLE*NewKeyhole();
void InitKeyhole(KEYHOLE* pkeyhole);
int  GetKeyholeSize();
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);
void CloneKeyhole(KEYHOLE* pkeyhole, KEYHOLE* pkeyholeBase);
void DeleteKeyhole(KEYHOLE* pkeyhole);

class JSG;
JSG* NewJsg();
void InitJsg(JSG* pjsg);
int  GetJsgSize();
void CloneJsg(JSG* pjsg, JSG* pjsgBase);
void DeleteJsg(JSG* pjsg);

class UNK13;
void*NewUnk13();
void InitUnk13(UNK13* punk13);
void CloneUnk13(UNK13* punk13, UNK13* punk13Base);
void DeleteUnk13(UNK13* punk13);

class UNK14;
void*NewUnk14();
void CloneUnk14(UNK14* punk14, UNK14* punk14Base);
void DeleteUnk14(UNK14* punk14);

class ASEGA;
int  GetAsegaSize();
void DeleteAsega(LO* plo);

class SMA;
int  GetSmaSize();
void DeleteSma(LO* plo);