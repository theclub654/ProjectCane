#pragma once
// Forward declarations
class CBinaryInputStream;
class ALO;
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

// Static Object
class SO;
struct SOP;
void InitSo(SO* pso);
void OnSoAdd(SO* pso);
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis);
void UpdateSo(SO* pso, float dt);

// Player Object
class PO;
void InitPo(PO* ppo);

class STEP;
void InitStep(STEP* pstep);

// Sly
class JT;
void InitJt(JT* pjt);
void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis);

// Guard
class STEPGUARD;
void InitStepGuard(STEPGUARD *pstepguard);
void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream* pbis);

class SMARTGUARD;
void InitSmartGuard(SMARTGUARD* psmartguard);

class GOMER;
void InitGomer(GOMER* pgomer);

class UBG;
void InitUbg(UBG* pubg);

class MBG;
void InitMbg(MBG* pmbg);
void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream* pbis);

class BHG;
void InitBhg(BHG* pbhg);

class MURRAY;
void InitMurray(MURRAY* pmurray);

class PUFFC;

class CRFOD;

class CRFODB;
void InitCrfodb(CRFODB* pcrfodb);

class CRFODK;

class TANK;
void InitTank(TANK* ptank);

class JP;
void InitJp(JP* pjp);

class HG;
void InitHg(HG* phg);

class ROV;
void LoadRovFromBrx(ROV *prov, CBinaryInputStream* pbis);

class PUFFER;
void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream* pbis);

class MGV;
void LoadMgvFromBrx(MGV *pmgv, CBinaryInputStream* pbis);

class BRK;
void LoadBrkFromBrx(BRK *pbrk, CBinaryInputStream* pbis);

class BUTTON;
void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis);

class VOLBTN;
void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis);

class SQUISH;
void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis);

class SPRIZE;
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);

class CLUE;
void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis);

// Laser Sensor
class LASEN;
void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis);

// Convo
class CNVO;
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);

class HBSK;
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);

class BOMB;
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);

class MISSILE;
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);

class FLY;
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);

class RAT;
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);

class ROH;
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);

class ROC;
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);

class ROST;
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);

class ROP;
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);

class DART;
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);

class JLO;
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);

class EMITTER;
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);

// Check Point
class CHKPNT;
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);

class PROXY;
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);

class DPRIZE;
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);

class LOCK;
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);

class LOCKG;
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);

class FLASH;
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);

class TN;
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void LoadTbspFromBrx(CBinaryInputStream* pbis);

class JLOC;
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);

class DIALOG;
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);

class MGC;
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);

class JACKN;
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);

// Static World
class SW;
void InitSw(SW* psw);
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);
void UpdateSw(SW* psw, float dt);
void GetSwParams(SW* psw, SOP** ppsop);

// Camera
class CM;
void InitCm(CM* pcm);

class SHAPE;
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);

class XFM;
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);

class WARP;
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);

class HND;
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);

class EXPLG;
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);

class EXPLO;
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);

class VOL;
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);

class EXIT;
void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis);

class PNT;
void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis);

// Animation Segment
class ASEG;
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);

// Visibility Map
class VISMAP;
void InitVismap(VISMAP* pvismap);
void LoadVismapFromBrx(VISMAP* pvismap, CBinaryInputStream* pbis);

class SM;
void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis);

// AI Path's
class PATHZONE;
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);

// Reach Map
class RCHM;
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);

class KEYHOLE;
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);




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