#pragma once
#include "lo.h"
#include "bsp.h"

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


struct MRSG 
{
	float t;
	float dt;
};

struct BL 
{
	float u;
	struct ASEG *paseg;
	struct CHN  *pchn;
};

struct BLRCH : BL
{
	OID  oidAseg;
	MRSG mprchsmrsg[2];
};

class RCHM : public LO
{
	public:
		OID oidHost;
		OID oidTouch;
		struct ALO* paloHost;
		struct ALO* paloTouch;
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
		std::vector <BLRCH> ablrch;
		//BLRCH ablrch[24];
		struct ASEGBL* pasegbl;
		float dtPause;

};

RCHM*NewRchm();
void InitRchm(RCHM* prchm);
int  GetRchmSize();
void LoadRchmFromBrx(RCHM *prchm, CBinaryInputStream *pbis);
void CloneRchm(RCHM* prchm, RCHM* prchmBase);
void DeleteRchm(RCHM* prchm);