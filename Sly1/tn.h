#pragma once
#include "alo.h"

void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);

struct TSURF
{
	float gDot;
	glm::vec3 normal;
};
struct TBSP
{
	struct TSURF* ptsurf;
	struct TBSP* ptbspNeg;
	struct TBSP* ptbspPos;
};
enum TNS
{
	TNS_Nil = -1,
	TNS_Out = 0,
	TNS_In = 1,
	TNS_Max = 2
};

class TN : public ALO
{
	public:
		struct PO *ppo;
		int ctsurf;
		std::vector <TSURF> atsurf;
		int ctbsp;
		std::vector <TBSP> atbsp;
		struct CRV* pcrv;
		char TNFN[128];
		int fCylinder;
		int fSwitchInAir;
		int priCamera;
		int fFakeCylinder;
		float dradSlack;
		TNS tns;
		float tTns;
		float sClosest;
		glm::mat3 matXfm;
		glm::vec3 posXfm;
		float sTotal;
		glm::vec3 posFake;
		float sRadFake;
		int grftnd;
		int fPanOnEntry;
		int fPanOnIdle;
		float dtPanOnEntry;
		float rswPanOnEntry;
		float dtPanOnIdle;
		float rswPanOnIdle;
		int fUseVolume;
		CFK cfk;
		int fCutOnEntry;
		float radFOV;
		int fNoSquish;
};

void* NewTn();
void InitTn(TN* ptn);
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void LoadTbspFromBrx(CBinaryInputStream* pbis);