#pragma once
#include "bis.h"
#include "shd.h"

// Glob is just another word for model.

struct SUBGLOB // NOT DONE
{
	glm::vec3 posCenter;
	float sRadius;
	uint32_t unSelfIllum;
	struct SHD *pshd;
	int cibnd;
	int aibnd[4];
}; // NOT DONE

// Model data
struct GLOB // NOT DONE
{
	// Moodel origin position
	glm::vec3 posCenter;
	float sRadius;
	//RP rp;
	int fThreeWay;
	float sMRD;
	float sCelBorderMRD;
	float gZOrder;
	int fDynamic;
	int grfglob;
	//GLEAM *pgleam
	//RTCK rtck;
	struct SAA* psaa;
	float uFog;
	//FGFN fgfn;
	float rSubglobRadius;
	//WRBG *pwrbg;
	// Number of submodels for model
	int csubglob;
	SUBGLOB *asubglob;
	int csubcel;
	//SUBCEL *asubcel;
	glm::mat4 pdmat;
	//BLOT *pblot;
	enum OID oid;
	char* pchzName;
}; // NOT DONE

struct GLOBI // NOT DONE
{
	int grfzon;
	//SUBGLOBI *asubglobi;
	int cframeStaticLights;
	//TWPS twps;
	float uAlpha;
	float tUnfade;
}; // NOT DONE

struct GLOBSET // NOT DONE
{
	int cbnd;
	//BND *abnd;
	OID *mpibndoid;
	// Number of submodeles for a model
	int cglob;
	struct GLOB *aglob;
	struct GLOBI *aglobi;
	//LTFN ltfn;
	uint32_t grfglobset;
	struct RGBA rgbaCel;
	int cpose;
	float* agPoses;
	float* agPosesOrig;
	//WRBG *pwrbgFirst;
	int cpsaa;
	struct SAA** apsaa;
}; // NOT DONE

void LoadGlobsetFromBrx(GLOBSET *pglobset, CBinaryInputStream *pbis); // NOT FINISHED
