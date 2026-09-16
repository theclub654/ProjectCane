#pragma once
#include "so.h"

struct DECOYDAMAGE
{
	DECOY* pdecoy;
	JT* pjt;
};

class DECOY : public SO
{
	public:
	int cHitsRemaining;
	XP* pxp;
	class EXPL* pexplDestroy;
};

DECOY*NewDecoy();
void*GetDecoyCHitsRemaining(DECOY* pdecoy);
void SetDecoyCHitsRemaining(DECOY* pdecoy, int cHitsRemaining);
void*GetDecoyPexplDestroy(DECOY* pdecoy);
void SetDecoyPexplDestroy(DECOY* pdecoy, EXPL* pexplDestroy);
void CloneDecoy(DECOY* pdecoy, DECOY* pdecoyBase);
void PostDecoyLoad(DECOY* pdecoy);
void DamageJtDecoy(DECOY* pdecoy, JT* pjt, int fForceDestroy);
void UpdateDecoy(DECOY* pdecoy, float dt);
void PresetDecoyAccel(DECOY* pdecoy, float dt);
void AdjustDecoyXps(DECOY* pdecoy);
bool FAbsorbDecoyWkr(DECOY* pdecoy, WKR* pwkr);
void UpdateDecoyXfWorldHierarchy(DECOY* pdecoy);
void HandleDecoyMessage(DECOY* pdecoy, int msgid, void* pv);
void DeleteDecoy(DECOY* pdecoy);

static SNIP snipDecoyPostLoad =
{
	0x22, (OID)0x152, offsetof(DECOY, pexplDestroy)
};