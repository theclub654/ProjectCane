#pragma once
// Forward declarations
class CBinaryInputStream;
class ALO;
class CM;
struct RO;
extern bool loadEmitMesh;

// Local Obeject
class LO;
enum MSGID;
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

// A Local Object;
void InitAlo(ALO* palo);
void AddAloHierarchy(ALO* palo);
void RemoveAloHierarchy(ALO* palo);
void OnAloAdd(ALO* palo);
void OnAloRemove(ALO* palo);
void CloneAloHierarchy(ALO* palo);
void CloneAlo(ALO* palo);
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void UpdateAlo(ALO* palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* proDup);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void DeleteAlo(LO* palo);

// Static Object
class SO;
struct SOP;
void InitSo(SO* pso);
void OnSoAdd(SO* pso);
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis);
void UpdateSo(SO* pso, float dt);
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);
void DeleteSo(LO* plo);

// Player Object
class PO;
void InitPo(PO* ppo);
void OnPoAdd(PO* ppo);
void DeletePo(LO* plo);

class STEP;
void InitStep(STEP* pstep);
void DeleteStep(LO* plo);

// Sly
class JT;
void InitJt(JT* pjt);
void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis);
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);
void DeleteJt(LO* plo);

// Guard
class STEPGUARD;
void InitStepGuard(STEPGUARD *pstepguard);
void OnStepguardAdd(STEPGUARD* pstepguard);
void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream* pbis);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);
void DeleteStepguard(LO* plo);

class SMARTGUARD;
void InitSmartGuard(SMARTGUARD* psmartguard);
void DeleteSmartGuard(LO* plo);

class GOMER;
void InitGomer(GOMER* pgomer);
void DeleteGomer(LO* plo);

class UBG;
void DeleteUbg(LO* plo);

class MBG;
void InitMbg(MBG* pmbg);
void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream* pbis);
void DeleteMbg(LO* plo);

class BHG;
void InitBhg(BHG* pbhg);
void DeleteBhg(LO* plo);

class MURRAY;
void InitMurray(MURRAY* pmurray);
void DeleteMurray(LO* plo);

class PUFFC;
void DeletePuffc(LO* plo);

class CRFOD;
void OnCrfodAdd(CRFOD* pcrfod);
void DeleteCrfod(LO* plo);

class CRFODB;
void InitCrfodb(CRFODB* pcrfodb);
void DeleteCrfodb(LO* plo);

class CRFODK;
void DeleteCrfodk(LO* plo);

class TANK;
void InitTank(TANK* ptank);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);
void DeleteTank(LO* plo);

class JP;
void InitJp(JP* pjp);
void RenderJpAll(JP* pjp, CM* pcm, RO* pro);
void DeleteJp(LO* plo);

class HG;
void InitHg(HG* phg);
void DeleteHg(LO* plo);

class MECHA;
void InitMecha(MECHA* pmecha);
void DeleteMecha(LO* plo);

class ROV;
void InitRov(ROV* prov);
void LoadRovFromBrx(ROV *prov, CBinaryInputStream* pbis);
void DeleteRov(LO* plo);

class TURRET;
void DeleteTurret(LO* plo);

class VAULT;
void InitVault(VAULT* pvault);
void DeleteVault(LO* plo);

class PUFFER;
void InitPuffer(PUFFER* ppuffer);
void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream* pbis);
void DeletePuffer(LO* plo);

class MGV;
void InitMgv(MGV* pmgv);
void LoadMgvFromBrx(MGV *pmgv, CBinaryInputStream* pbis);
void DeleteMGV(LO* plo);

class SUV;
void InitSuv(SUV* psuv);
void DeleteSUV(LO* plo);

class CYCLE;
void InitCycle(CYCLE* pcycle);
void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro);
void DeleteCycle(LO* plo);

class LGN;
void InitLgn(LGN* plgn);
void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro);
void DeleteLgn(LO* plo);

class JACK;
void InitJack(JACK* pjack);
void DeleteJack(LO* plo);

class RIPG;
void InitRipg(RIPG* pripg);
void DeleteRipg(LO* plo);

class WATER;
void InitWater(WATER* pwater);
void DeleteWater(LO* plo);

class BRK;
void InitBrk(BRK* pbrk);
void LoadBrkFromBrx(BRK *pbrk, CBinaryInputStream* pbis);
void DeleteBrk(LO* plo);

class BREAK;
void InitBreak(BREAK* pbreak);
void DeleteBreak(LO* plo);

class ALBRK;
void DeleteAlbrk(LO* plo);

class CAN;
void InitCan(CAN* pcan);
void DeleteCan(LO* plo);

class DARTGUN;
void InitDartgun(DARTGUN* pdartgun);
void DeleteDartGun(LO* plo);

class SWP;
void InitSwp(SWP* pswp);
void DeleteSwp(LO* plo);

class FRAGILE;
void InitFragile(FRAGILE* pfragile);
void DeleteFragile(LO* plo);

class ZAPBREAK;
void DeleteZapBreak(LO* plo);

class BRKP;
void DeleteBrkp(LO* plo);

class BUTTON;
void InitButton(BUTTON* pbutton);
void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis);
void DeleteButton(LO* plo);

class VOLBTN;
void InitVolbtn(VOLBTN* pvolbtn);
void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis);
void DeleteVolbtn(LO* plo);

class JLOVOL;
void InitJlovol(JLOVOL* pjlovol);
void DeleteJlovol(LO* plo);

class SQUISH;
void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis);
void DeleteSquish(LO* plo);

class BONE;
void DeleteBone(LO* plo);

class LBONE;
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);
void DeleteLBone(LO* plo);

class SPRIZE;
void InitSprize(SPRIZE* psprize);
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);
void DeleteSprize(LO* plo);

class SCPRIZE;
void InitScprize(SCPRIZE* pscprize);
void DeleteScprize(LO* plo);

class LIFETKN;
void DeleteLifetkn(LO* plo);

class CLUE;
void InitClue(CLUE* pclue);
void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);
void DeleteClue(LO* plo);

class ALARM;
void InitAlarm(ALARM* palarm);
void DeleteAlarm(LO* plo);

class SENSOR;
void InitSensor(SENSOR* psensor);
void DeleteSensor(LO* plo);

// Laser Sensor
class LASEN;
void InitLasen(LASEN* plasen);
void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis);
void DeleteLasen(LO* plo);

class CAMSEN;
void InitCamsen(CAMSEN* pcamsen);
void DeleteCamsen(LO* plo);

class PRSEN;
void InitPrsen(PRSEN* pprsen);
void DeletePrsen(LO* plo);

class BARRIER;
void InitBarrier(BARRIER* pbarrier);
void DeleteBarrier(LO* plo);

class IKH;
void DeleteIkh(LO* plo);

class TZP;
void InitTzp(TZP* ptzp);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);
void DeleteTzp(LO* plo);

class VOLZP;
void InitVolzp(VOLZP* pvolzp);
void DeleteVolzp(LO* plo);

// Convo
class CNVO;
void InitCnvo(CNVO* pcnvo);
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);
void DeleteCnvo(LO* plo);

class HBSK;
void InitHbsk(HBSK* phbsk);
void OnHbskAdd(HBSK* phbsk);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);
void DeleteHbsk(LO* plo);

class BOMB;
void InitBomb(BOMB* pbomb);
void OnBombAdd(BOMB* pbomb);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);
void DeleteBomb(LO* plo);

class MISSILE;
void InitMissile(MISSILE* pmissile);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);
void DeleteMissile(LO* plo);

class ACCMISS;
void InitAccmiss(ACCMISS* paccmiss);
void DeleteAccmiss(LO* plo);

class TARMISS;
void InitTarmiss(TARMISS* ptarmiss);
void DeleteTarmiss(LO* plo);

class SPLMISS;
void DeleteSplmiss(LO* plo);

class GROUNDMISS;
void InitGroundmiss(GROUNDMISS* pgroundmiss);
void DeleteGroundmiss(LO* plo);

class FLY;
void InitFly(FLY* pfly);
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);
void DeleteFly(LO* plo);

class RAT;
void InitRat(RAT* prat);
void OnRatAdd(RAT* prat);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);
void RenderRatAll(RAT* prat, CM* pcm, RO* pro);
void DeleteRat(LO* plo);

class ROH;
void InitRoh(ROH* proh);
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);
void DeleteRoh(LO* plo);

class ROC;
void InitRoc(ROC* proc);
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);
void DeleteRoc(LO* plo);

class ROST;
void InitRost(ROST* prost);
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);
void DeleteRost(LO* plo);

class ROP;
void InitRop(ROP* prop);
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);
void DeleteRop(LO* plo);

class DART;
void InitDart(DART* pdart);
void OnDartAdd(DART* pdart);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);
void DeleteDart(LO* plo);

class UBV;
void DeleteUbv(LO* plo);

class UBP;
void InitUbg(UBG* pubg);
void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro);
void DeleteUbp(LO* plo);

class DSP;
void DeleteDsp(LO* plo);

class JLO;
void InitJlo(JLO* pjlo);
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void DeleteJlo(LO* plo);

class PUFFT;
void DeletePufft(LO* plo);

class MRKV;
void InitMrkv(MRKV* pmrkv);
void DeleteMrkv(LO* plo);

class LGNB;
void InitLgnb(LGNB* plgnb);
void DeleteLgnb(LO* plo);

class BLIPG;
void InitBlipg(BLIPG* pblipg);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);
void DeleteBlipg(LO* plo);

class CAMERA;
void InitCamera(CAMERA* pcamera);
void DeleteCamera(LO* plo);

class EMITTER;
void InitEmitter(EMITTER* pemitter);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);
void DeleteEmitter(LO* plo);

class LIGHT;
void InitLight(LIGHT* plight);
void OnLightAdd(LIGHT* plight);
void DeleteLight(LO* plo);

class LIKH;
void DeleteLikh(LO* plo);

// Check Point
class CHKPNT;
void InitChkpnt(CHKPNT* pchkpnt);
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void DeleteChkpnt(LO* plo);

class PROXY;
void InitProxy(PROXY* pproxy);
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);
void DeleteProxy(LO* plo);

class SKY;
void DeleteSky(LO* plo);

class DPRIZE;
void InitDprize(DPRIZE* pdprize);
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);
void DeleteDprize(LO* plo);

class CHARM;
void InitCharm(CHARM* pcharm);
void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro);
void DeleteCharm(LO* plo);

class COIN;
void InitCoin(COIN* pcoin);
void DeleteCoin(LO* plo);

class KEY;
void InitKey(KEY* pkey);
void DeleteKey(LO* plo);

class GOLD;
void InitGold(GOLD* pgold);
void DeleteGold(LO* plo);

class LOCK;
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void DeleteLock(LO* plo);

class LOCKG;
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void DeleteLockg(LO* plo);

class TAIL;
void InitTail(TAIL* ptail);
void DeleteTail(LO* plo);

class ROB;
void InitRob(ROB* prob);
void DeleteRob(LO* plo);

class FLASH;
void InitFlash(FLASH* pflash);
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);
void DeleteFlash(LO* plo);

class DYSH;
void InitDysh(DYSH* pdysh);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);
void DeleteDysh(LO* plo);

class SCENTMAP;
void InitScentmap(SCENTMAP* pscentmap);
void DeleteScentmap(LO* plo);

class WAYPOINT;
void InitWaypoint(WAYPOINT* pwaypoint);
void DeleteWaypoint(LO* plo);

class TN;
void InitTn(TN* ptn);
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void LoadTbspFromBrx(CBinaryInputStream* pbis);
void DeleteTn(LO* plo);

class JLOC;
void InitJloc(JLOC* pjloc);
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);
void DeleteJloc(LO* plo);

class DIALOG;
void InitDialog(DIALOG* pdialog);
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void DeleteDialog(LO* plo);

class SPEAKER;
void InitSpeaker(SPEAKER* pspeaker);
void DeleteSpeaker(LO* plo);

class ROPE;
void InitRope(ROPE* prope);
void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro);
void DeleteRope(LO* plo);

class WM;
void RenderWmAll(WM* pwm, CM* pcm, RO* pro);
void DeleteWm(LO* plo);

class PUFFB;
void DeletePuffb(LO* plo);

class CRBRAIN;
void InitCrbrain(CRBRAIN* pcrbrain);
void DeleteCrbrain(LO* plo);

class MGC;
void InitMgc(MGC* pmgc);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);
void DeleteMgc(LO* plo);

class JACKB;
void InitJackb(JACKB* pjackb);
void DeleteJackb(LO* plo);

class JACKN;
void InitJackn(JACKN* pjackn);
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);
void DeleteJackn(LO* plo);

class JACKF;
void InitJackf(JACKF* pjackf);
void DeleteJackf(LO* plo);

// Static World
class SW;
void InitSw(SW* psw);
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);
void UpdateSw(SW* psw, float dt);
void GetSwParams(SW* psw, SOP** ppsop);

// Camera
void InitCm(CM* pcm);
void DeleteCm(LO* plo);

class SHAPE;
void InitShape(SHAPE* pshape);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);
void DeleteShape(LO* plo);

class HSHAPE;
void InitHshape(HSHAPE* phshape);
void OnHshapeAdd(HSHAPE* phshape);
void DeleteHshape(LO* plo);

class PIPE;
void InitPipe(PIPE* ppipe);
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);
void DeletePipe(LO* plo);

class RAIL;
void OnRailAdd(RAIL* prail);
void DeleteRail(LO* plo);

class LANDING;
void OnLandingAdd(LANDING* planding);
void DeleteLanding(LO* plo);

class XFM;
void InitXfm(XFM* pxfm);
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);
void DeleteXfm(LO* plo);

class WARP;
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);
void DeleteWarp(LO* plo);

class TARGET;
void InitTarget(TARGET* ptarget);
void OnTargetAdd(TARGET* ptarget);
void DeleteTarget(LO* plo);

class HND;
void InitHnd(HND* phnd);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
void DeleteHnd(LO* plo);

class EXPL;
void DeleteExpl(LO* plo);

class EXPLG;
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void DeleteExplg(LO* plo);

class EXPLO;
void InitExplo(EXPLO* pexplo);
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);
void DeleteExplo(LO* plo);

class EXPLS;
void InitExpls(EXPLS* pexpls);
void DeleteExpls(LO* plo);

class VOL;
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);
void DeleteVol(LO* plo);

class RATHOLE;
void OnRatholeAdd(RATHOLE* prathole);
void DeleteRathole(LO* plo);

class PUFFV;
void InitPuffv(PUFFV* ppuffv);
void DeletePuffv(LO* plo);

class EXIT;
void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis);
void DeleteExit(LO* plo);

class PNT;
void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis);
void DeletePnt(LO* plo);

class HPNT;
void InitHpnt(HPNT* phpnt);
void OnHpntAdd(HPNT* phpnt);
void DeleteHpnt(LO* plo);

class JMT;
void OnJmtAdd(JMT* pjmt);
void DeleteJmt(LO* plo);

class SPIRE;
void OnSpireAdd(SPIRE* pspire);
void DeleteSpire(LO* plo);

class SCAN;
void DeleteScan(LO* plo);

// Animation Segment
class ASEG;
void InitAseg(ASEG* paseg);
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void DeleteAseg(LO* plo);

class ASEGBL;
void DeleteAsegbl(LO* plo);

class MAP;
void DeleteMap(LO* plo);

// Visibility Map
class VISMAP;
void InitVismap(VISMAP* pvismap);
void LoadVismapFromBrx(VISMAP* pvismap, CBinaryInputStream* pbis);
void DeleteVismap(LO* plo);

class FRZG;
void DeleteFrzg(LO* plo);

class SM;
void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis);
void DeleteSm(LO* plo);

class SGG;
void InitSgg(SGG* psgg);
void DeleteSgg(LO* plo);

// AI Path's
class PATHZONE;
void OnPathzoneAdd(PATHZONE* ppathzone);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);
void DeletePathzone(LO* plo);

// Reach Map
class RCHM;
void InitRchm(RCHM* prchm);
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);
void DeleteRchm(LO* plo);

class RWM;
void InitRwm(RWM* prwm);
void DeleteRwm(LO* plo);

class WR;
void InitWr(WR* pwr);
void DeleteWr(LO* plo);

class KEYHOLE;
void InitKeyhole(KEYHOLE* pkeyhole);
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);
void DeleteKeyhole(LO* plo);

class JSG;
void InitJsg(JSG* pjsg);
void DeleteJsg(LO* plo);

class ASEGA;
void DeleteAsega(LO* plo);

class SMA;
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