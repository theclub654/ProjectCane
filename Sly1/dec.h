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

// Static Object
class SO;
struct SOP;
void InitSo(SO* pso);
void OnSoAdd(SO* pso);
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis);
void UpdateSo(SO* pso, float dt);
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);

// Player Object
class PO;
void InitPo(PO* ppo);
void OnPoAdd(PO* ppo);

class STEP;
void InitStep(STEP* pstep);

// Sly
class JT;
void InitJt(JT* pjt);
void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis);
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);

// Guard
class STEPGUARD;
void InitStepGuard(STEPGUARD *pstepguard);
void OnStepguardAdd(STEPGUARD* pstepguard);
void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream* pbis);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);

class SMARTGUARD;
void InitSmartGuard(SMARTGUARD* psmartguard);

class GOMER;
void InitGomer(GOMER* pgomer);

class UBG;
class UBP;
void InitUbg(UBG* pubg);
void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro);

class MBG;
void InitMbg(MBG* pmbg);
void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream* pbis);

class BHG;
void InitBhg(BHG* pbhg);

class MURRAY;
void InitMurray(MURRAY* pmurray);

class PUFFC;

class CRFOD;
void OnCrfodAdd(CRFOD* pcrfod);

class CRFODB;
void InitCrfodb(CRFODB* pcrfodb);

class CRFODK;

class TANK;
void InitTank(TANK* ptank);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);

class JP;
void InitJp(JP* pjp);
void RenderJpAll(JP* pjp, CM* pcm, RO* pro);

class HG;
void InitHg(HG* phg);

class MECHA;
void InitMecha(MECHA* pmecha);

class ROV;
void InitRov(ROV* prov);
void LoadRovFromBrx(ROV *prov, CBinaryInputStream* pbis);

class VAULT;
void InitVault(VAULT* pvault);

class PUFFER;
void InitPuffer(PUFFER* ppuffer);
void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream* pbis);

class MGV;
void InitMgv(MGV* pmgv);
void LoadMgvFromBrx(MGV *pmgv, CBinaryInputStream* pbis);

class SUV;
void InitSuv(SUV* psuv);

class CYCLE;
void InitCycle(CYCLE* pcycle);
void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro);

class LGN;
void InitLgn(LGN* plgn);
void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro);

class JACK;
void InitJack(JACK* pjack);

class RIPG;
void InitRipg(RIPG* pripg);

class WATER;
void InitWater(WATER* pwater);

class BRK;
void InitBrk(BRK* pbrk);
void LoadBrkFromBrx(BRK *pbrk, CBinaryInputStream* pbis);

class BREAK;
void InitBreak(BREAK* pbreak);

class CAN;
void InitCan(CAN* pcan);

class DARTGUN;
void InitDartgun(DARTGUN* pdartgun);

class SWP;
void InitSwp(SWP* pswp);

class FRAGILE;
void InitFragile(FRAGILE* pfragile);

class BUTTON;
void InitButton(BUTTON* pbutton);
void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis);

class VOLBTN;
void InitVolbtn(VOLBTN* pvolbtn);
void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis);

class JLOVOL;
void InitJlovol(JLOVOL* pjlovol);

class SQUISH;
void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis);

class BONE;
class LBONE;
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);

class SPRIZE;
void InitSprize(SPRIZE* psprize);
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);

class SCPRIZE;
void InitScprize(SCPRIZE* pscprize);

class CLUE;
void InitClue(CLUE* pclue);
void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);

class ALARM;
void InitAlarm(ALARM* palarm);

class SENSOR;
void InitSensor(SENSOR* psensor);

// Laser Sensor
class LASEN;
void InitLasen(LASEN* plasen);
void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis);

class CAMSEN;
void InitCamsen(CAMSEN* pcamsen);

class PRSEN;
void InitPrsen(PRSEN* pprsen);

class BARRIER;
void InitBarrier(BARRIER* pbarrier);

class TZP;
void InitTzp(TZP* ptzp);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);

class VOLZP;
void InitVolzp(VOLZP* pvolzp);

// Convo
class CNVO;
void InitCnvo(CNVO* pcnvo);
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);

class HBSK;
void InitHbsk(HBSK* phbsk);
void OnHbskAdd(HBSK* phbsk);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);

class BOMB;
void InitBomb(BOMB* pbomb);
void OnBombAdd(BOMB* pbomb);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);

class MISSILE;
void InitMissile(MISSILE* pmissile);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);

class ACCMISS;
void InitAccmiss(ACCMISS* paccmiss);

class TARMISS;
void InitTarmiss(TARMISS* ptarmiss);

class GROUNDMISS;
void InitGroundmiss(GROUNDMISS* pgroundmiss);

class FLY;
void InitFly(FLY* pfly);
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);

class RAT;
void InitRat(RAT* prat);
void OnRatAdd(RAT* prat);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);
void RenderRatAll(RAT* prat, CM* pcm, RO* pro);

class ROH;
void InitRoh(ROH* proh);
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);

class ROC;
void InitRoc(ROC* proc);
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);

class ROST;
void InitRost(ROST* prost);
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);

class ROP;
void InitRop(ROP* prop);
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);

class DART;
void InitDart(DART* pdart);
void OnDartAdd(DART* pdart);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);

class JLO;
void InitJlo(JLO* pjlo);
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);

class MRKV;
void InitMrkv(MRKV* pmrkv);

class LGNB;
void InitLgnb(LGNB* plgnb);

class BLIPG;
void InitBlipg(BLIPG* pblipg);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);

class CAMERA;
void InitCamera(CAMERA* pcamera);

class EMITTER;
void InitEmitter(EMITTER* pemitter);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);

class LIGHT;
void InitLight(LIGHT* plight);
void OnLightAdd(LIGHT* plight);

// Check Point
class CHKPNT;
void InitChkpnt(CHKPNT* pchkpnt);
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);

class PROXY;
void InitProxy(PROXY* pproxy);
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);

class DPRIZE;
void InitDprize(DPRIZE* pdprize);
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);

class CHARM;
void InitCharm(CHARM* pcharm);
void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro);

class COIN;
void InitCoin(COIN* pcoin);

class KEY;
void InitKey(KEY* pkey);

class GOLD;
void InitGold(GOLD* pgold);

class LOCK;
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);

class LOCKG;
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);

class TAIL;
void InitTail(TAIL* ptail);

class ROB;
void InitRob(ROB* prob);

class FLASH;
void InitFlash(FLASH* pflash);
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);

class DYSH;
void InitDysh(DYSH* pdysh);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);

class SCENTMAP;
void InitScentmap(SCENTMAP* pscentmap);

class WAYPOINT;
void InitWaypoint(WAYPOINT* pwaypoint);

class TN;
void InitTn(TN* ptn);
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void LoadTbspFromBrx(CBinaryInputStream* pbis);

class JLOC;
void InitJloc(JLOC* pjloc);
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);

class DIALOG;
void InitDialog(DIALOG* pdialog);
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);

class SPEAKER;
void InitSpeaker(SPEAKER* pspeaker);

class ROPE;
void InitRope(ROPE* prope);
void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro);

class WM;
void RenderWmAll(WM* pwm, CM* pcm, RO* pro);

class CRBRAIN;
void InitCrbrain(CRBRAIN* pcrbrain);

class MGC;
void InitMgc(MGC* pmgc);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);

class JACKB;
void InitJackb(JACKB* pjackb);

class JACKN;
void InitJackn(JACKN* pjackn);
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);

class JACKF;
void InitJackf(JACKF* pjackf);

// Static World
class SW;
void InitSw(SW* psw);
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);
void UpdateSw(SW* psw, float dt);
void GetSwParams(SW* psw, SOP** ppsop);

// Camera
void InitCm(CM* pcm);

class SHAPE;
void InitShape(SHAPE* pshape);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);

class HSHAPE;
void InitHshape(HSHAPE* phshape);
void OnHshapeAdd(HSHAPE* phshape);

class PIPE;
void InitPipe(PIPE* ppipe);
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);

class RAIL;
void OnRailAdd(RAIL* prail);

class LANDING;
void OnLandingAdd(LANDING* planding);

class XFM;
void InitXfm(XFM* pxfm);
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);

class WARP;
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);

class TARGET;
void InitTarget(TARGET* ptarget);
void OnTargetAdd(TARGET* ptarget);

class HND;
void InitHnd(HND* phnd);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);

class EXPLG;
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);

class EXPLO;
void InitExplo(EXPLO* pexplo);
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);

class EXPLS;
void InitExpls(EXPLS* pexpls);

class VOL;
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);

class RATHOLE;
void OnRatholeAdd(RATHOLE* prathole);

class PUFFV;
void InitPuffv(PUFFV* ppuffv);

class EXIT;
void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis);

class PNT;
void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis);

class HPNT;
void InitHpnt(HPNT* phpnt);
void OnHpntAdd(HPNT* phpnt);

class JMT;
void OnJmtAdd(JMT* pjmt);

class SPIRE;
void OnSpireAdd(SPIRE* pspire);

// Animation Segment
class ASEG;
void InitAseg(ASEG* paseg);
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);

// Visibility Map
class VISMAP;
void InitVismap(VISMAP* pvismap);
void LoadVismapFromBrx(VISMAP* pvismap, CBinaryInputStream* pbis);

class SM;
void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis);

class SGG;
void InitSgg(SGG* psgg);

// AI Path's
class PATHZONE;
void OnPathzoneAdd(PATHZONE* ppathzone);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);

// Reach Map
class RCHM;
void InitRchm(RCHM* prchm);
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);

class RWM;
void InitRwm(RWM* prwm);

class WR;
void InitWr(WR* pwr);

class KEYHOLE;
void InitKeyhole(KEYHOLE* pkeyhole);
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);

class JSG;
void InitJsg(JSG* pjsg);



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