#pragma once
#include "lo.h"
// GOTTA COME BACK AND PUT THE REST OF RCHM AND UPDATE THE CB
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
};

void InitRchm(RCHM* prchm);
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);