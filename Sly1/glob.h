#pragma once
#include "bis.h"
#include "gl.h"
#include "util.h"
// Glob is just another word for model.

extern class CM* g_pcm;

enum TWPS 
{
	TWPS_Shadow = 0,
	TWPS_ShadowMidtone = 1,
	TWPS_ShadowMidtoneSaturate = 2
};
enum TRLK {
	TRLK_Nil = -1,
	TRLK_Download = 0,
	TRLK_DownloadRelight = 1,
	TRLK_Relight = 2,
	TRLK_RelightCache = 3,
	TRLK_Quick = 4,
	TRLK_Max = 5
};
enum RTCK
{
	RTCK_Nil = -1,
	RTCK_None = 0,
	RTCK_All = 1,
	RTCK_WorldZ = 2,
	RTCK_LocalX = 3,
	RTCK_LocalY = 4,
	RTCK_LocalZ = 5,
	RTCK_Max = 6
};
enum WEK
{
	WEK_Nil = -1,
	WEK_XYZ = 0,
	WEK_XY = 1,
	WEK_XZ = 2,
	WEK_YZ = 3,
	WEK_X = 4,
	WEK_Y = 5,
	WEK_Z = 6,
	WEK_Max = 7
};
// Render Priority
enum RP 
{
	RP_Nil = -1,
	RP_DynamicTexture = 0,
	RP_Background = 1,
	RP_BlotContext = 2,
	RP_Opaque = 3,
	RP_Cutout = 4,
	RP_CelBorder = 5,
	RP_ProjVolume = 6,
	RP_OpaqueAfterProjVolume = 7,
	RP_CutoutAfterProjVolume = 8,
	RP_CelBorderAfterProjVolume = 9,
	RP_MurkClear = 10,
	RP_MurkOpaque = 11,
	RP_MurkFill = 12,
	RP_Translucent = 13,
	RP_TranslucentCelBorder = 14,
	RP_Blip = 15,
	RP_Foreground = 16,
	RP_WorldMap = 17,
	RP_Max = 18
};

struct VERTICE
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec4 color;
	glm::vec2 uv;
};

// Vertex Flag
struct VTXFLG
{
	// Vertex Index
	byte ipos;  
	// Normal Index
	byte inormal;
	// UV Index
	byte iuv;
	// Strip Flag
	byte bMisc;
};

struct WEKI
{
	WEK wek;
	float sInner;
	float uInner;
	float sOuter;
	float uOuter;
	glm::mat4 dmat;
};

struct WRBG
{
	struct ALO *palo;
	struct GLOB *pglob;
	OID oid;
	struct WR *pwr;
	int cmat;
	int fDpos;
	int fDuv;
	WEKI weki;
	struct WRBG *pwrbgNextGlobset;
	struct WRBG *pwrbgNextWr;
};

struct GLEAM
{
	glm::vec3 normal;
	CLQC clqc;
};

struct FGFN
{
	float duFogBias;
	float ruFog;
	float sNearFog;
	float duFogPlusClipBias;
};

struct SUBGLOB // NOT DONE
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	glm::vec3 posCenter; // Submodel orgin
	float sRadius;

	std::vector <VERTICE>  vertices;
	std::vector <uint16_t> indices;

	// Object brightness
	float unSelfIllum;
	struct SHD* pshd;
	struct RSGLD *prsgld;
	struct WRBSG *pwrbsg;
	int cibnd;
	int aibnd[4];
	int fThreeWay;

}; // NOT DONE

struct SUBGLOBI 
{
	float tShadowsValid;
	int cpshadow;
	struct SHADOW* apshadow[4];
};

// Model data
struct GLOB // NOT DONE
{
	// Moodel origin position
	glm::vec3 posCenter;
	float sRadius;
	RP rp;
	int fThreeWay;
	float sMRD;
	float sCelBorderMRD;
	float gZOrder;
	int fDynamic;
	GRFGLOB grfglob;
	std::vector <GLEAM> gleam;
	RTCK rtck;
	struct SAA* psaa;
	// Object fog intensity
	float uFog;
	FGFN fgfn;
	float rSubglobRadius;
	std::vector <WRBG> wrbg;
	// Number of submodels for model
	int csubglob;
	std::vector <SUBGLOB> asubglob;
	int csubcel;
	//SUBCEL *asubcel;
	// Instanced object matrix container
	std::vector <glm::mat4> dmat;
	short instanceIndex;
	struct BLOT* pblot;
	OID oid;
	char* pchzName;
}; // NOT DONE

struct GLOBI
{
	int grfzon;
	SUBGLOBI asubglobi;
	int cframeStaticLights;
	TWPS twps;
	float uAlpha;
	float tUnfade;
}; 

struct GLOBSET // NOT DONE
{
	int cbnd;
	struct BND* abnd;
	std::vector <OID> mpibndoid;
	uint64_t cglob;
	std::vector <GLOB> aglob;
	std::vector <GLOBI> aglobi;
	uint32_t grfglobset;
	RGBA rgbaCel;
	int cpose;
	std::vector <float> agPoses;
	std::vector <float> agPosesOrig;
	struct WRBG* pwrbgFirst;
	int cpsaa;
	struct SAA** apsaa;
};

// Loads 3D model data from binary file
void LoadGlobsetFromBrx(GLOBSET *pglobset, ALO *palo, CBinaryInputStream *pbis);
// Converts strips to tri lists and stores 3D sub model in VRAM
void BuildSubGlob(SUBGLOB *psubglob, SHD *pshd, std::vector <glm::vec3> &positions, std::vector <glm::vec3> &normals, std::vector <glm::vec4> &colors, std::vector <glm::vec2> &texcoords, std::vector <VTXFLG> &indexes);
void CloneGlob(GLOBSET *pglobset, GLOB *pglob, GLOBI *pglobi);