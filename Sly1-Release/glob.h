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

struct VERTICE
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec4 color;
	glm::vec2 uv;
};

struct INDICE
{
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;
};

struct RO
{
	GLuint *VAO;
	GLuint *VBO;
	GLuint *EBO;
	int cvtx;

	GLuint  edgeBuf;
	GLuint  edgeTex;
	GLsizei edgeCount;

	SHD *pshd;

	float unSelfIllum;
	glm::mat4 model;
	glm::vec3 posCenter;
	int fDynamic;
	float uFog;
	float darken;
	float uAlpha;
	float uAlphaCelBorder;
};

// Render Priority List
struct RPL
{
	RP rp;
	RO ro;
	float z;
	byte grfshd;
	float sRadius;
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

struct SUBCEL
{
	// Edge data as a texture buffer (4 vec4 per edge)
	GLuint  edgeBuf   = 0; // GL_TEXTURE_BUFFER's storage
	GLuint  edgeTex   = 0; // GL texture handle bound to edgeBuf
	GLsizei edgeCount = 0; // number of edges (== ctwef)

	std::vector <glm::vec3> positions;
	std::vector <uint16_t>  indices;
	std::vector <float>     weights;
};

struct SUBPOSEF
{
	std::vector <uint16_t> aiposf;
	std::vector <uint16_t> ainormalf;
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

struct FGFN
{
	float duFogBias;
	float ruFog;
	float sNearFog;
	float duFogPlusClipBias;
};

struct SUBGLOB
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	std::vector <VERTICE> vertices;
	std::vector <INDICE>  indices;
	int cvtx;

	glm::vec3 posCenter; // Submodel orgin
	float sRadius;
	// Object brightness
	float unSelfIllum;
	// Shader ID
	int shdID;
	// Object shader property
	struct SHD* pshd;
	struct WRBSG *pwrbsg;
	int cibnd;
	int aibnd[4];
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
	// Ptr to instance model matrix
	std::shared_ptr <glm::mat4> pdmat;
	short instanceIndex;
	struct BLOT* pblot;
	OID oid;
	char* pchzName;
};

struct GLOBI
{
	int grfzon;
	SUBGLOBI asubglobi;
	int cframeStaticLights;
	TWPS twps;
	float uAlpha;
	float tUnfade;
}; 

struct GLOBSET
{
	int cbnd;
	struct BND* abnd;
	std::vector <OID> mpibndoid;
	uint64_t cglob;
	std::vector <GLOB>  aglob;
	std::vector <GLOBI> aglobi;
	uint32_t grfglobset;
	glm::vec4 rgbaCel;
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
void BuildSubcel(GLOBSET *pglobset, SUBCEL *psubcel, int cposf, std::vector <glm::vec3> &aposf, int ctwef, std::vector <TWEF> &atwef, std::vector <SUBPOSEF> &asubposef, std::vector <glm::vec3> &aposfPoses, std::vector <float> &agWeights);

extern int  g_fogType;
extern bool g_fRenderModels;
extern bool g_fRenderCollision;
extern bool g_fRenderCelBorders;
extern bool g_fBsp;
extern float g_uAlpha;
extern GLuint gEmptyVAO;
extern glm::vec4 g_rgbaCel;