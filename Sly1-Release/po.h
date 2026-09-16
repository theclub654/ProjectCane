#pragma once
#include "so.h"
#include "emitter.h"

enum JTHS 
{
	JTHS_Nil = -1,
	JTHS_Normal = 0,
	JTHS_Damaged = 1,
	JTHS_Dead = 2,
	JTHS_Max = 3
};

enum PCK
{
	PCK_Nil = -1,
	PCK_Key = 0,
	PCK_Gold = 1,
	PCK_Boost = 2,
	PCK_Max = 3
};

struct PZI
{
	int fCollectEnabled;
	class ALO* paloBag;
	class ALO* mpccharmpaloCharm[4];
	float sAutoCollect;
	class ALO* paloCollect;
};

struct DIAPI
{
	int fCallable;
	int fPlayable;
};

class PO : public SO
{
	public:
	int fPlayable;
	int fLockBase;
	glm::vec3 posBasePrev;
	float tWakeRipple;
	struct AMB* pambWake;
	PZI pzi;
	class ALO* paloUseCharm;
	class ALO* paloUnk;
	class ASEG* pasegCelebrateKey;
	ALO* paloCharmEffect;
	ASEG* pasegCollectKey;
	class ALO* paloCollectTarget[4];
	struct MATCH* pmatch[9];
	int cmatchCollect;
	PCK pckCollect;
	glm::vec3 posCollectPrev;
};

PO*  NewPo();
void InitPo(PO *ppo);
void*GetPofPlayable(PO* ppo);
void OnPoRemove(PO* ppo);
void ClonePo(PO* ppo, PO* ppoBase);
void HandlePoMessage(PO* ppo, MSGID msgid, void* pv);
void OnPoActive(PO* ppo, int fActive, PO* ppoOther);
void GetPoCpdefi(PO* ppo, float dt, CPDEFI* pcpdefi);
int  FIsPoSoundBase(PO* ppo);
void MakePoActive(PO* ppo);
bool FInvulnerablePo(PO* ppo);
PO*  PpoCur();
PO*  PpoStart();
int  IppoFindPo(PO* ppo);
void AddPoToList(PO* ppo);
void RemovePoFromList(PO* ppo);
void OnPoAdd(PO* ppo);
void OnPoRemove(PO* ppo);
void SwitchToIppo(int ippo);
void SetPoPlayable(PO* ppo, int fPlayable);
void SwitchToPo(PO* ppo);
PZI* PpziCur();
ALO* PaloFindPoBag(PO* ppo);
void GetPoDiapi(PO* ppo, DIALOG* pdialog, DIAPI* pdiapi);
JTHS JthsCurrentPo(PO* ppo);
void CollectPoPrize(PO* ppo, PCK pck, ALO* paloOther);
void SetPoPck(PO* ppo, PCK pck);
int  FTakePoDamage(PO* ppo, ZPR* pzpr);
void GetJthsCurrentPo(PO* ppo, JTHS* pjths);
void PlayPoDialog(PO* ppo, DIALOG* pdialog);
void HandleDialogButtons(JOY* pjoy);
void UpdatePoCharmVisibility(PO* ppo);
void UpdatePo(PO* ppo, float dt);
void UsePoCharm(PO* ppo);
void UpdatePoPrizeCollection(PO* ppo);
int  GetPoSize();
void DeletePo(PO* ppo);

extern int g_ippoCur;
extern int g_cppo;
extern PO *g_appo[16];
extern PZI s_pziDefault;
extern SMP s_smpZ;
extern CLQ s_clqDtDamageToUSpring;
extern CLQ s_clqUZapToUCel;
extern CLQ s_clqUFlashToUCel;
extern EMITRIP g_emitripCharm;
extern EMITV g_emitvCharm;
