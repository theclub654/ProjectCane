#pragma once
// Forward declarations
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

class CBinaryInputStream;
class PROXY;
class ALO;
class CM;
struct RO;
enum LIGHTK;
struct LM;
extern bool loadEmitMesh;

// Local Obeject
class LO;
enum MSGID;
void* NewLo();
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
void PostLoLoad(LO* plo);
void SetLoParent(LO* plo, ALO* paloParent);
void SubscribeLoObject(LO* plo, LO* ploTarget);
void UnsubscribeLoObject(LO* plo, LO* ploTarget);
void SubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
void UnsubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
int  GetLoSize();
void DeleteLo(LO* plo);

// A Local Object;
void* NewAlo();
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
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void UpdateAlo(ALO* palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* proDup);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
int  GetAloSize();
void DeleteAlo(LO* palo);

// Static Object
class SO;
struct SOP;
void* NewSo();
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
void DeleteSo(LO* plo);

// Merged Statics
class MS;
void* NewMs();
int  GetMsSize();
void DeleteMs(LO* plo);

// Player Object
class PO;
void* NewPo();
void InitPo(PO* ppo);
void OnPoAdd(PO* ppo);
void OnPoRemove(PO* ppo);
void ClonePo(PO* ppo, PO* ppoBase);
int  GetPoSize();
void DeletePo(LO* plo);

class STEP;
void* NewStep();
void InitStep(STEP* pstep);
int  GetStepSize();
void UpdateStepXfWorld(STEP* pstep);
void CloneStep(STEP* pstep, STEP* pstepBase);
void RotateStepToMat(STEP* pstep, glm::mat3& pmat);
void DeleteStep(LO* plo);

// Sly
class JT;
void* NewJt();
void InitJt(JT* pjt);
void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis);
void CloneJt(JT* pjt, JT* pjtBase);
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);
int  GetJtSize();
void DeleteJt(LO* plo);

// Guard
class STEPGUARD;

void* NewStepguard();
void InitStepGuard(STEPGUARD* pstepguard);
void OnStepguardAdd(STEPGUARD* pstepguard);
void OnStepguardRemove(STEPGUARD* pstepguard);
void CloneStepguard(STEPGUARD* pstepguard, STEPGUARD* pstepguardBase);
void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);
int  GetStepguardSize();
void DeleteStepguard(LO* plo);

class SMARTGUARD;
void* NewSmartguard();
void InitSmartGuard(SMARTGUARD* psmartguard);
int  GetSmartguardSize();
void CloneSmartguard(SMARTGUARD* psmartguard, SMARTGUARD* psmartguardBase);
void DeleteSmartGuard(LO* plo);

class GOMER;
void* NewGomer();
void InitGomer(GOMER* pgomer);
int  GetGomerSize();
void CloneGomer(GOMER* pgomer, GOMER* pgomerBase);
void DeleteGomer(LO* plo);

class UBG;
void* NewUbg();
int  GetUbgSize();
void CloneUbg(UBG* pubg, UBG* pubgBase);
void DeleteUbg(LO* plo);

class MBG;
void* NewMbg();
void InitMbg(MBG* pmbg);
int  GetMbgSize();
void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis);
void CloneMbg(MBG* pmbg, MBG* pmbgBase);
void DeleteMbg(LO* plo);

class BHG;
void* NewBhg();
void InitBhg(BHG* pbhg);
int  GetBhgSize();
void CloneBhg(BHG* pbhg, BHG* pbhgBase);
void DeleteBhg(LO* plo);

class MURRAY;
void* NewMurray();
void InitMurray(MURRAY* pmurray);
int  GetMurraySize();
void CloneMurray(MURRAY* pmurray, MURRAY* pmurrayBase);
void DeleteMurray(LO* plo);

class PUFFC;
void* NewPuffc();
int  GetPuffcSize();
void ClonePuffc(PUFFC* ppuffc, PUFFC* ppuffcBase);
void DeletePuffc(LO* plo);

class CRFOD;
void* NewCrfod();
void OnCrfodAdd(CRFOD* pcrfod);
void OnCrfodRemove(CRFOD* pcrfod);
void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase);
int  GetCrfodSize();
void DeleteCrfod(LO* plo);

class CRFODB;
void* NewCrfodb();
void InitCrfodb(CRFODB* pcrfodb);
int  GetCrfodbSize();
void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase);
void DeleteCrfodb(LO* plo);

class CRFODK;
void* NewCrfodk();
int  GetCrfodkSize();
void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase);
void DeleteCrfodk(LO* plo);

class TANK;
void* NewTank();
void InitTank(TANK* ptank);
int  GetTankSize();
void CloneTank(TANK* ptank, TANK* ptankBase);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);
void DeleteTank(LO* plo);

class JP;
void* NewJp();
void InitJp(JP* pjp);
int  GetJpSize();
void CloneJp(JP* pjp, JP* pjpBase);
void RenderJpAll(JP* pjp, CM* pcm, RO* pro);
void DeleteJp(LO* plo);

class HG;
void* NewHg();
void InitHg(HG* phg);
int  GetHgSize();
void CloneHg(HG* phg, HG* phgBase);
void DeleteHg(LO* plo);

class MECHA;
void* NewMecha();
void InitMecha(MECHA* pmecha);
int  GetMechaSize();
void CloneMecha(MECHA* pmecha, MECHA* pmechaBase);
void DeleteMecha(LO* plo);

class ROV;
void* NewRov();
void InitRov(ROV* prov);
int  GetRovSize();
void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis);
void CloneRov(ROV* prov, ROV* provBase);
void DeleteRov(LO* plo);

class TURRET;
void* NewTurret();
int  GetTurretSize();
void CloneTurret(TURRET* pturret, TURRET* pturretBase);
void DeleteTurret(LO* plo);

class VAULT;
void* NewVault();
void InitVault(VAULT* pvault);
int  GetVaultSize();
void CloneVault(VAULT* pvault, VAULT* pvaultBase);
void DeleteVault(LO* plo);

class PUFFER;
void* NewPuffer();
void InitPuffer(PUFFER* ppuffer);
int  GetPufferSize();
void LoadPufferFromBrx(PUFFER* ppuffer, CBinaryInputStream* pbis);
void ClonePuffer(PUFFER* ppuffer, PUFFER* ppufferBase);
void DeletePuffer(LO* plo);

class MGV;
void* NewMgv();
void InitMgv(MGV* pmgv);
int  GetMgvSize();
void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis);
void CloneMgv(MGV* pmgv, MGV* pmgvBase);
void DeleteMgv(LO* plo);

class SUV;
void* NewSuv();
void InitSuv(SUV* psuv);
int  GetSuvSize();
void UpdateSuvXfWorld(SUV* psuv);
void CloneSuv(SUV* psuv, SUV* psuvBase);
void DeleteSuv(LO* plo);

class CYCLE;
void* NewCycle();
void InitCycle(CYCLE* pcycle);
int  GetCycleSize();
void UpdateCycleXfWorld(CYCLE* pcycle);
void CloneCycle(CYCLE* pcycle, CYCLE* pcycleBase);
void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro);
void DeleteCycle(LO* plo);

class LGN;
void* NewLgn();
void InitLgn(LGN* plgn);
int  GetLgnSize();
void CloneLgn(LGN* plgn, LGN* plgnBase);
void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro);
void DeleteLgn(LO* plo);

class JACK;
void* NewJack();
void InitJack(JACK* pjack);
int  GetJackSize();
void CloneJack(JACK* pjack, JACK* pjackBase);
void DeleteJack(LO* plo);

class RIPG;
void* NewRipg();
void InitRipg(RIPG* pripg);
int  GetRipgSize();
void CloneRipg(RIPG* pripg, RIPG* pripgBase);
void DeleteRipg(LO* plo);

class WATER;
void* NewWater();
void InitWater(WATER* pwater);
int  GetWaterSize();
void CloneWater(WATER* pwater, WATER* pwaterBase);
void DeleteWater(LO* plo);

class BRK;
void* NewBrk();
void InitBrk(BRK* pbrk);
int  GetBrkSize();
void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis);
void CloneBrk(BRK* pbrk, BRK* pbrkBase);
void DeleteBrk(LO* plo);

class BREAK;
void* NewBreak();
void InitBreak(BREAK* pbreak);
int  GetBreakSize();
void CloneBreak(BREAK* pbreak, BREAK* pbreakBase);
void DeleteBreak(LO* plo);

class ALBRK;
void* NewAlbrk();
int  GetAlbrkSize();
void CloneAlbrk(ALBRK* palbrk, ALBRK* palbrkBase);
void DeleteAlbrk(LO* plo);

class CAN;
void* NewCan();
void InitCan(CAN* pcan);
int  GetCanSize();
void CloneCan(CAN* pcan, CAN* pcanBase);
void DeleteCan(LO* plo);

class DARTGUN;
void* NewDartgun();
void InitDartgun(DARTGUN* pdartgun);
int  GetDartgunSize();
void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase);
void DeleteDartgun(LO* plo);

class SWP;
void* NewSwp();
void InitSwp(SWP* pswp);
int  GetSwpSize();
void CloneSwp(SWP* pswp, SWP* pswpBase);
void DeleteSwp(LO* plo);

class FRAGILE;
void* NewFragile();
void InitFragile(FRAGILE* pfragile);
int  GetFragileSize();
void CloneFragile(FRAGILE* pfragile, FRAGILE* pfragileBase);
void DeleteFragile(LO* plo);

class ZAPBREAK;
void* NewZapbreak();
int  GetZapbreakSize();
void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase);
void DeleteZapbreak(LO* plo);

class BRKP;
void* NewBrkp();
int  GetBrkpSize();
void CloneBrkp(BRKP* prkp, BRKP* prkpBase);
void DeleteBrkp(LO* plo);

class BUTTON;
void* NewButton();
void InitButton(BUTTON* pbutton);
int  GetButtonSize();
void CloneButton(BUTTON* pbutton, BUTTON* pbuttonBase);
void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis);
void DeleteButton(LO* plo);

class VOLBTN;
void* NewVolbtn();
void InitVolbtn(VOLBTN* pvolbtn);
int  GetVolbtnSize();
void CloneVolbtn(VOLBTN* pvolbtn, VOLBTN* pvolbtnBase);
void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis);
void DeleteVolbtn(LO* plo);

class JLOVOL;
void* NewJlovol();
void InitJlovol(JLOVOL* pjlovol);
int  GetJlovolSize();
void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase);
void DeleteJlovol(LO* plo);

class SQUISH;
void* NewSquish();
int  GetSquishSize();
void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis);
void CloneSquish(SQUISH* psquish, SQUISH* psquishBase);
void DeleteSquish(LO* plo);

class BONE;
void* NewBone();
int  GetBoneSize();
void CloneBone(BONE* pbone, BONE* pboneBase);
void DeleteBone(LO* plo);

class SPRIZE;
void* NewSprize();
void InitSprize(SPRIZE* psprize);
int  GetSprizeSize();
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);
void CloneSprize(SPRIZE* psprize, SPRIZE* psprizeBase);
void DeleteSprize(LO* plo);

class SCPRIZE;
void* NewScprize();
void InitScprize(SCPRIZE* pscprize);
int  GetScprizeSize();
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
void DeleteScprize(LO* plo);

class LIFETKN;
void* NewLifetkn();
int  GetLifetknSize();
void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase);
void DeleteLifetkn(LO* plo);

class CLUE;
void* NewClue();
void InitClue(CLUE* pclue);
int  GetClueSize();
void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis);
void CloneClue(CLUE* pclue, CLUE* pclueBase);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);
void DeleteClue(LO* plo);

class ALARM;
void* NewAlarm();
void InitAlarm(ALARM* palarm);
int  GetAlarmSize();
void CloneAlarm(ALARM* palarm, ALARM* palarmBase);
void DeleteAlarm(LO* plo);

class SENSOR;
void* NewSensor();
void InitSensor(SENSOR* psensor);
int  GetSensorSize();
void CloneSensor(SENSOR* psensor, SENSOR* psensorBase);
void DeleteSensor(LO* plo);

// Laser Sensor
class LASEN;
void* NewLasen();
void InitLasen(LASEN* plasen);
int  GetLasenSize();
void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis);
void CloneLasen(LASEN* plasen, LASEN* plasenBase);
void DeleteLasen(LO* plo);

class CAMSEN;
void* NewCamsen();
void InitCamsen(CAMSEN* pcamsen);
int  GetCamsenSize();
void CloneCamsen(CAMSEN* pcamsen, CAMSEN* pcamsenBase);
void DeleteCamsen(LO* plo);

class PRSEN;
void* NewPrsen();
void InitPrsen(PRSEN* pprsen);
int  GetPrsenSize();
void ClonePrsen(PRSEN* pprsen, PRSEN* pprsenBase);
void DeletePrsen(LO* plo);

class BARRIER;
void* NewBarrier();
void InitBarrier(BARRIER* pbarrier);
int  GetBarrierSize();
void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase);
void DeleteBarrier(LO* plo);

class IKH;
void* NewIkh();
int  GetIkhSize();
void CloneIkh(IKH* pikh, IKH* pikhBase);
void DeleteIkh(LO* plo);

class TZP;
void* NewTzp();
void InitTzp(TZP* ptzp);
int  GetTzpSize();
void CloneTzp(TZP* ptzp, TZP* ptzpBase);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);
void DeleteTzp(LO* plo);

class VOLZP;
void* NewVolzp();
void InitVolzp(VOLZP* pvolzp);
int  GetVolzpSize();
void CloneVolzp(VOLZP* pvolzp, VOLZP* pvolzpBase);
void DeleteVolzp(LO* plo);

// Convo
class CNVO;
void* NewCnvo();
void InitCnvo(CNVO* pcnvo);
int  GetCnvoSize();
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);
void CloneCnvo(CNVO* pcnvo, CNVO* pcnvoBase);
void DeleteCnvo(LO* plo);

class HBSK;
void* NewHbsk();
void InitHbsk(HBSK* phbsk);
void OnHbskAdd(HBSK* phbsk);
void OnHbskRemove(HBSK* phbsk);
int  GetHbskSize();
void CloneHbsk(HBSK* phbsk, HBSK* phbskBase);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);
void DeleteHbsk(LO* plo);

class BOMB;
void* NewBomb();
void InitBomb(BOMB* pbomb);
void OnBombAdd(BOMB* pbomb);
int  GetBombSize();
void CloneBomb(BOMB* pbomb, BOMB* pbombBase);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);
void DeleteBomb(LO* plo);

class MISSILE;
void* NewMissile();
void InitMissile(MISSILE* pmissile);
int  GetMissileSize();
void OnMissileRemove(MISSILE* pmissile);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void CloneMissile(MISSILE* pmissile, MISSILE* pmissileBase);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);
void DeleteMissile(LO* plo);

class ACCMISS;
void* NewAccmiss();
void InitAccmiss(ACCMISS* paccmiss);
int  GetAccmissSize();
void CloneAccmiss(ACCMISS* paccmiss, ACCMISS* paccmissBase);
void DeleteAccmiss(LO* plo);

class TARMISS;
void* NewTarmiss();
void InitTarmiss(TARMISS* ptarmiss);
int  GetTarmissSize();
void CloneTarmiss(TARMISS* ptarmiss, TARMISS* ptarmissBase);
void DeleteTarmiss(LO* plo);

class SPLMISS;
void* NewSplmiss();
int  GetSplmissSize();
void CloneSplmiss(SPLMISS* psplmiss, SPLMISS* psplmissBase);
void DeleteSplmiss(LO* plo);

class GROUNDMISS;
void* NewGroundmiss();
void InitGroundmiss(GROUNDMISS* pgroundmiss);
int  GetGroundmissSize();
void CloneGroundmiss(GROUNDMISS* pgroundmiss, GROUNDMISS* pgroundmissBase);
void DeleteGroundmiss(LO* plo);

class FLY;
void* NewFly();
void InitFly(FLY* pfly);
int  GetFlySize();
void CloneFly(FLY* pfly, FLY* pflyBase);
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);
void DeleteFly(LO* plo);

class RAT;
void* NewRat();
void InitRat(RAT* prat);
int  GetRatSize();
void OnRatAdd(RAT* prat);
void OnRatRemove(RAT* prat);
void CloneRat(RAT* prat, RAT* pratBase);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);
void RenderRatAll(RAT* prat, CM* pcm, RO* pro);
void DeleteRat(LO* plo);

class ROH;
void* NewRoh();
void InitRoh(ROH* proh);
int  GetRohSize();
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);
void CloneRoh(ROH* proh, ROH* prohBase);
void DeleteRoh(LO* plo);

class ROC;
void* NewRoc();
void InitRoc(ROC* proc);
int  GetRocSize();
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);
void CloneRoc(ROC* proc, ROC* procBase);
void DeleteRoc(LO* plo);

class ROST;
void* NewRost();
void InitRost(ROST* prost);
int  GetRostSize();
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);
void CloneRost(ROST* prost, ROST* prostBase);
void DeleteRost(LO* plo);

class ROP;
void* NewRop();
void InitRop(ROP* prop);
int  GetRopSize();
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);
void CloneRop(ROP* prop, ROP* probBase);
void DeleteRop(LO* plo);

class DART;
void* NewDart();
void InitDart(DART* pdart);
int  GetDartSize();
void OnDartAdd(DART* pdart);
void RemoveDart(DART* pdart);
void CloneDart(DART* pdart, DART* pdartBase);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);
void DeleteDart(LO* plo);

class UBV;
void* NewUbv();
int  GetUbvSize();
void CloneUbv(UBV* pubv, UBV* pubvBase);
void DeleteUbv(LO* plo);

class UBP;
void* NewUbp();
void InitUbg(UBG* pubg);
int  GetUbpSize();
void CloneUbp(UBP* pubp, UBP* pubpBase);
void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro);
void DeleteUbp(LO* plo);

class DSP;
void* NewDsp();
int  GetDspSize();
void CloneDsp(DSP* pdsp, DSP* pdspBase);
void DeleteDsp(LO* plo);

class JLO;
void* NewJlo();
void InitJlo(JLO* pjlo);
int  GetJloSize();
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void CloneJlo(JLO* pjlo, JLO* pjloBase);
void DeleteJlo(LO* plo);

class PUFFT;
void* NewPufft();
int  GetPufftSize();
void ClonePufft(PUFFT* ppufft, PUFFT* ppufftBase);
void DeletePufft(LO* plo);

class MRKV;
void* NewMrkv();
void InitMrkv(MRKV* pmrkv);
int  GetMrkvSize();
void CloneMrkv(MRKV* pmrkv, MRKV* pmrkvBase);
void DeleteMrkv(LO* plo);

class LGNB;
void* NewLgnb();
void InitLgnb(LGNB* plgnb);
int  GetLgnbSize();
void CloneLgnb(LGNB* plgnb, LGNB* plgnbBase);
void DeleteLgnb(LO* plo);

class BLIPG;
void* NewBlipg();
void InitBlipg(BLIPG* pblipg);
int  GetBlipgSize();
void CloneBlipg(BLIPG* pblipg, BLIPG* pblipgBase);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);
void DeleteBlipg(LO* plo);

class CAMERA;
void* NewCamera();
void InitCamera(CAMERA* pcamera);
void CloneCamera(CAMERA* pcamera, CAMERA* pcameraBase);
int  GetCameraSize();
void DeleteCamera(LO* plo);

class LBONE;
void* NewLBone();
int  GetLBoneSize();
void CloneLBone(LBONE* lpbone, LBONE* lpboneBase);
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);
void DeleteLBone(LO* plo);

class EMITTER;
struct EMITB;
enum ENSK;
void* NewEmitter();
void InitEmitter(EMITTER* pemitter);
int  GetEmitterSize();
void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);
void DeleteEmitter(LO* plo);

class LIGHT;
void* NewLight();
void InitLight(LIGHT* plight);
int  GetLightSize();
void OnLightAdd(LIGHT* plight);
void OnLightRemove(LIGHT* plight);
void UpdateLightXfWorldHierarchy(LIGHT* plight);
void CloneLight(LIGHT* plight, LIGHT* plightBase);
void SetLightKind(LIGHT* plight, LIGHTK lightk);
void SetLightHighlightColor(LIGHT* plight, glm::vec3& pvecHighlight);
void SetLightMidtoneStrength(LIGHT* plight, float gMidtone);
void SetLightShadowStrength(LIGHT* plight, float gShadow);
void SetLightHighlightAngle(LIGHT* plight, float degHighlight);
void SetLightMidtoneAngle(LIGHT* plight, float degMidtone);
void SetLightShadowAngle(LIGHT* plight, float degShadow);
void SetLightDirection(LIGHT* plight, glm::vec3& pvecDirection);
void SetLightDynamic(LIGHT* plight, int fDynamic);
void SetLightFallOff(LIGHT* plight, LM* plm);
void SetLightConeAngle(LIGHT* plight, float degCone);
void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot);
void SetLightFrustrumUp(LIGHT* plight, glm::vec3& pvecUpLocal);
void DeleteLight(LO* plo);

class LIKH;
void* NewLikh();
int  GetLikhSize();
void CloneLikh(LIKH* plikh, LIKH* plikhBase);
void DeleteLikh(LO* plo);

// Check Point
class CHKPNT;
void* NewChkpnt();
void InitChkpnt(CHKPNT* pchkpnt);
int  GetChkpntSize();
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase);
void DeleteChkpnt(LO* plo);

void* NewProxy();
void InitProxy(PROXY* pproxy);
int  GetProxySize();
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);
void CloneProxy(PROXY* pproxy, PROXY* pproxyBase);
void DeleteProxy(LO* plo);

class SKY;
void* NewSky();
int  GetSkySize();
void CloneSky(SKY* psky, SKY* pskyBase);
void UpdateSky(SKY* psky, float dt);
void DeleteSky(LO* plo);

class DPRIZE;
void* NewDprize();
void InitDprize(DPRIZE* pdprize);
int  GetDprizeSize();
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);
void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase);
void DeleteDprize(LO* plo);

class CHARM;
void* NewCharm();
void InitCharm(CHARM* pcharm);
int  GetCharmSize();
void CloneCharm(CHARM* pcharm, CHARM* pcharmBase);
void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro);
void DeleteCharm(LO* plo);

class COIN;
void* NewCoin();
void InitCoin(COIN* pcoin);
int  GetCoinSize();
void CloneCoin(COIN* pcoin, COIN* pcoinBase);
void DeleteCoin(LO* plo);

class KEY;
void* NewKey();
void InitKey(KEY* pkey);
int  GetKeySize();
void CloneKey(KEY* pkey, KEY* pkeyBase);
void DeleteKey(LO* plo);

class GOLD;
void* NewGold();
void InitGold(GOLD* pgold);
int  GetGoldSize();
void CloneGold(GOLD* pgold, GOLD* pgoldBase);
void DeleteGold(LO* plo);

class LOCK;
void* NewLock();
int  GetLockSize();
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void CloneLock(LOCK* plock, LOCK* plockBase);
void DeleteLock(LO* plo);

class LOCKG;
void* NewLockg();
int  GetLockgSize();
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void CloneLockg(LOCKG* plockg, LOCKG* plockgBase);
void DeleteLockg(LO* plo);

class TAIL;
void* NewTail();
void InitTail(TAIL* ptail);
int  GetTailSize();
void CloneTail(TAIL* ptail, TAIL* ptailBase);
void DeleteTail(LO* plo);

class ROB;
void* NewRob();
void InitRob(ROB* prob);
int  GetRobSize();
void CloneRob(ROB* prob, ROB* probBase);
void DeleteRob(LO* plo);

class FLASH;
void* NewFlash();
void InitFlash(FLASH* pflash);
int  GetFlashSize();
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);
void CloneFlash(FLASH* pflash, FLASH* pflashBase);
void DeleteFlash(LO* plo);

class DYSH;
void* NewDysh();
void InitDysh(DYSH* pdysh);
int  GetDyshSize();
void CloneDysh(DYSH* pdysh, DYSH* pdyshBase);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);
void DeleteDysh(LO* plo);

class SCENTMAP;
void* NewScentmap();
void InitScentmap(SCENTMAP* pscentmap);
int  GetScentmapSize();
void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase);
void DeleteScentmap(LO* plo);

class WAYPOINT;
void* NewWaypoint();
void InitWaypoint(WAYPOINT* pwaypoint);
int  GetWaypointSize();
void CloneWaypoint(WAYPOINT* pwaypoint, WAYPOINT* pwaypointBase);
void DeleteWaypoint(LO* plo);

class TN;
void* NewTn();
void InitTn(TN* ptn);
int  GetTnSize();
void OnTnRemove(TN* ptn);
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void CloneTn(TN* ptn, TN* ptnBase);
void LoadTbspFromBrx(CBinaryInputStream* pbis);
void DeleteTn(LO* plo);

class JLOC;
void* NewJloc();
void InitJloc(JLOC* pjloc);
int  GetJlocSize();
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);
void CloneJloc(JLOC* pjloc, JLOC* pjlocBase);
void DeleteJloc(LO* plo);

class DIALOG;
void* NewDialog();
void InitDialog(DIALOG* pdialog);
int  GetDialogSize();
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void CloneDialog(DIALOG* pdialog, DIALOG* pdialogBase);
void DeleteDialog(LO* plo);

class SPEAKER;
void* NewSpeaker();
void InitSpeaker(SPEAKER* pspeaker);
int  GetSpeakerSize();
void CloneSpeaker(SPEAKER* pspeaker, SPEAKER* pspeakerBase);
void DeleteSpeaker(LO* plo);

class ROPE;
void* NewRope();
void InitRope(ROPE* prope);
int  GetRopeSize();
void CloneRope(ROPE* prope, ROPE* propeBase);
void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro);
void DeleteRope(LO* plo);

class WM;
void* NewWm();
int  GetWmSize();
void CloneWm(WM* pwm, WM* pwmBase);
void RenderWmAll(WM* pwm, CM* pcm, RO* pro);
void DeleteWm(LO* plo);

class PUFFB;
void* NewPuffb();
int  GetPuffbSize();
void ClonePuffb(PUFFB* ppuffb, PUFFB* ppuffbBase);
void DeletePuffb(LO* plo);

class CRBRAIN;
void* NewCrbrain();
void InitCrbrain(CRBRAIN* pcrbrain);
int  GetCrbrainSize();
void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase);
void DeleteCrbrain(LO* plo);

class MGC;
void* NewMgc();
void InitMgc(MGC* pmgc);
int  GetMgcSize();
void CloneMgc(MGC* pmgc, MGC* pmgcBase);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);
void DeleteMgc(LO* plo);

class JACKB;
void* NewJackb();
void InitJackb(JACKB* pjackb);
int  GetJackbSize();
void CloneJackb(JACKB* pjackb, JACKB* pjackbBase);
void DeleteJackb(LO* plo);

class JACKN;
void* NewJackn();
void InitJackn(JACKN* pjackn);
int  GetJacknSize();
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);
void CloneJackn(JACKN* pjackn, JACKN* pjacknBase);
void UpdateJackn(JACKN* pjackn, float dt);
void DeleteJackn(LO* plo);

class JACKF;
void* NewJackf();
void InitJackf(JACKF* pjackf);
int  GetJackfSize();
void CloneJackf(JACKF* pjackf, JACKF* pjackfBase);
void DeleteJackf(LO* plo);

// Static World
class SW;
void* NewSw();
void InitSw(SW* psw);
int  GetSwSize();
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);
void UpdateSw(SW* psw, float dt);
void GetSwParams(SW* psw, SOP** ppsop);
void DeleteSwObj(LO* plo);

// Camera
void* NewCm();
void InitCm(CM* pcm);
int  GetCmSize();
void CloneCm(CM* pcm, CM* pcmBase);
void DeleteCm(LO* plo);

class SHAPE;
void* NewShape();
void InitShape(SHAPE* pshape);
int  GetShapeSize();
void CloneShape(SHAPE* pshape, SHAPE* pshapeBase);
void SetShapeParent(SHAPE* pshape, ALO* paloParent);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);
void DeleteShape(LO* plo);

class HSHAPE;
void* NewHshape();
void InitHshape(HSHAPE* phshape);
int  GetHshapeSize();
void OnHshapeAdd(HSHAPE* phshape);
void OnHshapeRemove(HSHAPE* phshape);
void CloneHshape(HSHAPE* phshape, HSHAPE* phshapeBase);
void DeleteHshape(LO* plo);

class PIPE;
void* NewPipe();
void InitPipe(PIPE* ppipe);
int  GetPipeSize();
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);
void ClonePipe(PIPE* ppipe, PIPE* ppipeBase);
void DeletePipe(LO* plo);

class RAIL;
void* NewRail();
int  GetRailSize();
void OnRailAdd(RAIL* prail);
void OnRailRemove(RAIL* prail);
void CloneRail(RAIL* prail, RAIL* prailBase);
void DeleteRail(LO* plo);

class LANDING;
void* NewLanding();
int  GetLandingSize();
void OnLandingAdd(LANDING* planding);
void OnLandingRemove(LANDING* planding);
void CloneLanding(LANDING* planding, LANDING* plandingBase);
void DeleteLanding(LO* plo);

class XFM;
void* NewXfm();
void InitXfm(XFM* pxfm);
int  GetXfmSize();
void CloneXfm(XFM* pxfm, XFM* pxfmBase);
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);
void SetXfmParent(XFM* pxfm, ALO* paloParent);
void ApplyXfmProxy(XFM* pxfm, PROXY* pproxyApply);
void DeleteXfm(LO* plo);

class WARP;
void* NewWarp();
int  GetWarpSize();
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);
void CloneWarp(WARP* pwarp, WARP* pwarpBase);
void DeleteWarp(LO* plo);

class TARGET;
void* NewTarget();
void InitTarget(TARGET* ptarget);
int  GetTargetSize();
void OnTargetAdd(TARGET* ptarget);
void OnTargetRemove(TARGET* ptarget);
void CloneTarget(TARGET* ptarget, TARGET* ptargetBase);
void DeleteTarget(LO* plo);

class HND;
void* NewHnd();
void InitHnd(HND* phnd);
int  GetHndSize();
void CloneHnd(HND* phnd, HND* phndBase);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
void DeleteHnd(LO* plo);

class EXPL;
void* NewExpl();
int  GetExplSize();
void CloneExpl(EXPL* pexpl, EXPL* pexplBase);
void DeleteExpl(LO* plo);

class EXPLG;
void* NewExplg();
int  GetExplgSize();
void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase);
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void DeleteExplg(LO* plo);

class EXPLO;
void* NewExplo();
int  GetExploSize();
void InitExplo(EXPLO* pexplo);
void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase);
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);
void DeleteExplo(LO* plo);

class EXPLS;
void* NewExpls();
int  GetExplsSize();
void InitExpls(EXPLS* pexpls);
void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase);
void DeleteExpls(LO* plo);

class VOL;
void* NewVol();
int  GetVolSize();
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);
void CloneVol(VOL* pvol, VOL* pvolBase);
void DeleteVol(LO* plo);

class RATHOLE;
void* NewRathole();
int  GetRatholeSize();
void OnRatholeAdd(RATHOLE* prathole);
void OnRatholeRemove(RATHOLE* prathole);
void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase);
void DeleteRathole(LO* plo);

class PUFFV;
void* NewPuffv();
int  GetPuffvSize();
void InitPuffv(PUFFV* ppuffv);
void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase);
void DeletePuffv(LO* plo);

class EXIT;
void* NewExit();
int  GetExitSize();
void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis);
void CloneExit(EXIT* pexit, EXIT* pexitBase);
void DeleteExit(LO* plo);

class PNT;
void* NewPnt();
int  GetPntSize();
void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis);
void ClonePnt(PNT* pnt, PNT* pntBase);
void SetPntParent(PNT* ppnt, ALO* paloParent);
void ApplyPntProxy(PNT* ppnt, PROXY* pproxyApply);
void DeletePnt(LO* plo);

class HPNT;
void* NewHpnt();
void InitHpnt(HPNT* phpnt);
int  GetHpntSize();
void OnHpntAdd(HPNT* phpnt);
void OnHpntRemove(HPNT* phpnt);
void CloneHpnt(HPNT* phpnt, HPNT* phpntBase);
void DeleteHpnt(LO* plo);

class JMT;
void* NewJmt();
int  GetJmtSize();
void OnJmtAdd(JMT* pjmt);
void OnJmtRemove(JMT* pjmt);
void CloneJmt(JMT* pjmt, JMT* pjmtBase);
void DeleteJmt(LO* plo);

class SPIRE;
void* NewSpire();
int  GetSpireSize();
void OnSpireAdd(SPIRE* pspire);
void OnSpireRemove(SPIRE* pspire);
void CloneSpire(SPIRE* pspire, SPIRE* pspireBase);
void DeleteSpire(LO* plo);

class SCAN;
void* NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void DeleteScan(LO* plo);

// Animation Segment
class ASEG;
void* NewAseg();
void InitAseg(ASEG* paseg);
int  GetAsegSize();
void CloneAseg(ASEG* paseg, ASEG* pasegBase);
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void DeleteAseg(LO* plo);

class ASEGBL;
void* NewAsegbl();
void DeleteAsegbl(LO* plo);

class MAP;
void* NewMap();
int  GetMapSize();
void CloneMap(MAP* pmap, MAP* pmapBase);
void DeleteMap(LO* plo);

// Visibility Map
class VISMAP;
void* NewVismap();
void InitVismap(VISMAP* pvismap);
int  GetVismapSize();
void LoadVismapFromBrx(VISMAP* pvismap, CBinaryInputStream* pbis);
void DeleteVismap(LO* plo);

class FRZG;
void* NewFrzg();
int  GetFrzgSize();
void CloneFrzg(FRZG* pfrzg, FRZG* pfrzgBase);
void DeleteFrzg(LO* plo);

class SM;
void* NewSm();
int  GetSmSize();
void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis);
void DeleteSm(LO* plo);

class SGG;
void* NewSgg();
void InitSgg(SGG* psgg);
int  GetSggSize();
void CloneSgg(SGG* psgg, SGG* psggBase);
void DeleteSgg(LO* plo);

// AI Path's
class PATHZONE;
void* NewPathzone();
int  GetPathzoneSize();
void OnPathzoneAdd(PATHZONE* ppathzone);
void OnPathzoneRemove(PATHZONE* ppathzone);
void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);
void DeletePathzone(LO* plo);

// Reach Map
class RCHM;
void* NewRchm();
void InitRchm(RCHM* prchm);
int  GetRchmSize();
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);
void DeleteRchm(LO* plo);

class RWM;
void* NewRwm();
void InitRwm(RWM* prwm);
int  GetRwmSize();
void DeleteRwm(LO* plo);

class WR;
void* NewWr();
void InitWr(WR* pwr);
int  GetWrSize();
void DeleteWr(LO* plo);

class KEYHOLE;
void* NewKeyhole();
void InitKeyhole(KEYHOLE* pkeyhole);
int  GetKeyholeSize();
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);
void CloneKeyhole(KEYHOLE* pkeyhole, KEYHOLE* pkeyholeBase);
void DeleteKeyhole(LO* plo);

class JSG;
void* NewJsg();
void InitJsg(JSG* pjsg);
int  GetJsgSize();
void CloneJsg(JSG* pjsg, JSG* pjsgBase);
void DeleteJsg(LO* plo);

class ASEGA;
int  GetAsegaSize();
void DeleteAsega(LO* plo);

class SMA;
int  GetSmaSize();
void DeleteSma(LO* plo);

struct CRVL;
struct CRVC;
void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);


void LoadAcpcFromBrx(CBinaryInputStream* pbis);
void LoadAcpvFromBrx(CBinaryInputStream* pbis);
void LoadAkvbFromBrx(CBinaryInputStream* pbis);
void LoadAcrcFromBrx(CBinaryInputStream* pbis);
void LoadAcscFromBrx(CBinaryInputStream* pbis);
void LoadAcrbFromBrx(CBinaryInputStream* pbis);
void LoadApacgFromBrx(CBinaryInputStream* pbis);
void LoadAcgbFromBrx(CBinaryInputStream* pbis);
void LoadAcgbwFromBrx(CBinaryInputStream* pbis);
void LoadAcglFromBrx(CBinaryInputStream* pbis);
void LoadAcpbFromBrx(CBinaryInputStream* pbis);