#pragma once
#include "steprun.h"

enum RCHMK
{
	RCHMK_Nil = -1,
	RCHMK_Reach = 0,
	RCHMK_Swing = 1,
	RCHMK_Max = 2
};
struct RCH
{
	int fEnabled;
	float mpiblu[24];
};
struct TWR
{
	int aipos[4];
};

struct TWD 
{
	struct TWR *ptwrNeg;
	struct TWR *ptwrPos;
};

struct BLRCH : BL
{
	OID  oidAseg;
	MRSG mprchsmrsg[2];
};

class RCHM : public LO
{
	public:
	short oidHost;
	short oidTouch;
	class ALO* paloHost;
	class ALO* paloTouch;
	RCHMK rchmk;
	int cposGrid;
	int grftak;
	int fEnabled;
	float rclNatural;
	float ioNatural;
	float lhubNatural;
	float dtContactNatural;
	glm::vec3 posContactNatural;
	int fCenterSet;
	float rclCenter;
	float ioCenter;
	float lhubCenter;
	glm::vec3 posCenter;
	GEOM geomLocal;
	std::vector <RCH> mpiposrch;
	int ctwr;
	std::vector <TWR> atwr;
	BSPC bspcCat;
	std::vector <TWD> mpibsptwdCat;
	float gRadiusSquared;
	BLRCH ablrch[24];
	struct ASEGBL* pasegbl;
	float dtPause;

};

RCHM*NewRchm();
void InitRchm(RCHM* prchm);
int  GetRchmSize();
void*GetRchmRchmk(RCHM* prchm);
void SetRchmRchmk(RCHM* prchm, RCHMK rchmk);
void*GetRchmOidHost(RCHM* prchm);
void SetRchmOidHost(RCHM* prchm, int oidHost);
void*GetRchmOidTouch(RCHM* prchm);
void SetRchmOidTouch(RCHM* prchm, int oidTouch);
void*GetRchmGrftak(RCHM* prchm);
void SetRchmGrftak(RCHM* prchm, GRFTAK grftak);
void LoadRchmFromBrx(RCHM *prchm, CBinaryInputStream *pbis);
void CloneRchm(RCHM* prchm, RCHM* prchmBase);
void PostRchmLoad(RCHM* prchm);
void ReblendRchm(RCHM* prchm, TWR* ptwr, const glm::vec3* ppos);
void BuildRchmCoefficients(RCHM* prchm, float rcl, float io, float lhub, float* mpiblu);
void ConvertRchmIposToRclIoLhub(RCHM* prchm, int ipos, float* prcl, float* pio, float* plhub);
void SetRchmNaturalCoefficients(RCHM* prchm, float rcl, float io, float lhub);
void SetRchmCenterCoefficients(RCHM* prchm, float rcl, float io, float lhub);
void PredictRchmTargetPos(RCHM* prchm, TARGET* ptarget, float dt, glm::vec3* ppos);
void PredictRchmTargetLocalPos(RCHM* prchm, TARGET* ptarget, float dt, glm::vec3* pposLocal);
TWR* PtwrMapRchmSafe(RCHM* prchm, BSP* pbsp, const glm::vec3* ppos);
void FindRchmClosestPoint(RCHM* prchm, const glm::vec3* ppos, glm::vec3* pposClosest, TWR** pptwr, float* ps);
void TrackJtTarget(JT* pjt, RCHM* prchm, TARGET* ptarget);
void TrackJtPipe(JT* pjt, RCHM* prchm, PIPE* ppipe, float* psPipe);
void DeleteRchm(RCHM *prchm);

extern OID s_mprchsoid[2];
extern int s_mpimrsgcibReach;
extern int s_mpimrsgccmrsgReach;