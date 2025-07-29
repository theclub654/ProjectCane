#pragma once
#include "bis.h"
#include "gl.h"
#include "util.h"
// Glob is just another word for model.

extern class CM* g_pcm;
extern std::vector <SHD> g_ashd;

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
	glm::uvec4 boneIndices;
	glm::vec4 boneWeights;
};

struct INDICE
{
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;
};

struct RO
{
	GLuint* VAO;
	GLuint* VBO;
	GLuint* EBO;
	int cvtx;

	GLuint* celVAO;
	GLuint* celVBO;
	GLuint* celEBO;
	int celcvtx;

	GRFGLOB grfglob;

	SHD* pshd;
	float unSelfIllum;

	glm::mat4 model;
	float uFog;
	float uAlpha;
	int fDynamic;
	int fCelBorder;
};

// Render Priority List
struct RPL
{
	void (*PFNDRAW)(RPL*);
	RP rp;
	RO ro;
	float z;
	glm::vec3 posCenter;
};

// Vertex Flag
struct VTXFLG
{
	// Vertex Index
	uint32_t ipos;  
	// Normal Index
	uint32_t inormal;
	// UV Index
	uint32_t iuv;
	// Strip Flag
	uint32_t bMisc;
};

struct SUBPOSEF
{
	std::vector <uint16_t> aiposf;
	std::vector <uint16_t> ainormalf;
};

struct SUBCEL
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	std::vector <glm::vec3> positions;
	std::vector <uint16_t> indices;
	std::vector <float> weights;

	int cvtx;
};

struct TWEF
{
	uint32_t aipos0;
	uint32_t aipos1;
	uint32_t aipos2;
	uint32_t aipos3;
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

struct BND
{
	ALO* palo;
	glm::mat4 dmatOrig;
	glm::mat4 dmat;
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

	std::vector <VERTICE> vertices;
	std::vector <INDICE> indices;

	int cvtx;

	GLuint celVAO;
	GLuint celVBO;
	GLuint celEBO;

	std::vector <glm::vec3> celPositions;
	std::vector <INDICE> celIndices;

	int celcvtx;
	int fCelBorder;

	glm::vec3 posCenter; // Submodel orgin
	float sRadius;

	// Object brightness
	float unSelfIllum;
	// Object shader property
	struct SHD* pshd;
	struct WRBSG *pwrbsg;
	int cibnd;
	std::vector <int> aibnd;
};

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
	std::vector <SUBCEL> asubcel;
	// Ptr to instanced object matrix
	std::shared_ptr <glm::mat4> pdmat;
	struct BLOT* pblot;
	OID oid;
	char* pchzName;
}; // NOT DONE 

struct GLOBI
{
	GRFZON grfzon;
	SUBGLOBI asubglobi;
	int cframeStaticLights;
	TWPS twps;
	float uAlpha;
	float tUnfade;
}; 

struct GLOBSET // NOT DONE
{
	int cbnd;
	std::vector <BND> abnd;
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

// Initializes glsl uniform locations
void InitGlslUniforms();
// Loads 3D model data from binary file
void LoadGlobsetFromBrx(GLOBSET* pglobset, short cid, ALO* palo ,CBinaryInputStream* pbis);
// Converts strips to tri lists and stores 3D sub model in VRAM
void BuildSubGlob(SUBGLOB *psubglob, SHD *pshd, std::vector <glm::vec3> &positions, std::vector <glm::vec3> &normals, std::vector <glm::vec4> &colors, std::vector <glm::vec2> &texcoords, std::vector <VTXFLG> &indexes, SUBPOSEF* subposef, std::vector <glm::vec3>& aposfPoses, std::vector <glm::vec3>& anormalfPoses, std::vector <float>& agWeights);
void BuildSubcel(GLOBSET *pglobset, SUBCEL *psubcel, int cposf, std::vector <glm::vec3> &aposf, int ctwef, std::vector <TWEF> &atwef, std::vector <SUBPOSEF> &asubposef, std::vector <glm::vec3> &aposfPoses, std::vector <float> &agWeights);

extern int  g_fogType;
extern bool g_fRenderModels;
extern bool g_fRenderCollision;
extern bool g_fRenderCelBorders;
extern bool g_fBsp;

extern GLuint glslNumLights;
extern GLuint glslmatWorldToClip;
extern GLuint glslCameraPos;
extern GLuint glslFogType;
extern GLuint glslFogNear;
extern GLuint glslFogFar;
extern GLuint glslFogMax;
extern GLuint glslFogColor;
extern GLuint glslLsmShadow;
extern GLuint glslLsmDiffuse;
extern GLuint glslRgbaCel;
extern GLuint glslModel;
extern GLuint glslUFog;
extern GLuint glslUAlpha;
extern GLuint glslRDarken;
extern GLuint glslRko;
extern GLuint glslusSelfIllum;
extern GLuint glslFDynamic;
extern GLuint glslPosCenter;
extern GLuint glslfAlphaTest;
extern GLuint glslfCull;
extern GLuint glslCollisionRgba;