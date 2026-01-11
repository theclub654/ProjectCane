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

enum WARPTYPE
{
	WARP_NONE = 0,
	WARP_POS  = 1,
	WARP_UV   = 2,
	WARP_BOTH = 3
};

struct VERTICE
{
	glm::vec3  pos;
	glm::vec3  normal;
	glm::vec4  color;
	glm::vec2  uv;
	glm::uvec4 boneIndices;
	glm::vec4  boneWeights;
};

struct INDICE
{
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;
};

struct alignas(16) RO
{
	glm::mat4 model;       //  0 -  63
	int       rko;         //  64
	float     uAlpha;      //  68
	float     uFog;        //  72
	float     darken;      //  76
	glm::vec2 uvOffsets;   //  80 -  87
	int       _pad0;       //  88
	int       _pad1;       //  92
	int       warpType;    //  96
	int       warpCmat;    // 100
	int       warpCvtx;    // 104
	int       _padWarp0;   // 108
	int       _padWarp1;   // 112
	int       _padWarp2;   // 116
	int       _padWarp3;   // 120
	int       _padWarp4;   // 124
	glm::mat4 amatDpos[4]; // 128 - 383
	glm::mat4 amatDuv[4];  // 384 - 639
	int       fDynamic;    // 640
	float     unSelfIllum; // 644
	float     sRadius;     // 648
	int       _pad2;       // 652
	glm::vec4 posCenter;   // 656 - 671
	float     uAlphaCelBorder; 
};

// Render Priority List
struct RPL
{
	RP rp;

	GLuint  VAO;
	GLsizei cvtx;

	GLuint ssboWarpState;

	SHD* pshd;

	float z;

	void (*PFNBIND)(RPL*);

	RO ro;
};

struct RPLCEL
{
	RP rp;
	GLuint  edgeSSBO;
	GLsizei edgeCount;

	ROCEL rocel;
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
	// SSBO: 4 vec4 per edge (E0, E1, OA, OB)
	GLuint  edgeSSBO;  // GL_SHADER_STORAGE_BUFFER
	GLsizei edgeCount; // number of edges (== ctwef)

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
	WARPTYPE warpType;
	WEKI weki;
	std::shared_ptr <WRBG> pwrbgNextGlobset;
	std::shared_ptr <WRBG> pwrbgNextWr;
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

struct WRBSG_GL 
{
	std::vector <glm::vec4> basePos; // xyz1 in render-vertex order

	std::vector <glm::vec4> state;   // [imat*vertexCount + v]
	WR* pwr = nullptr;

	GLuint ssboState;
	GLsizeiptr ssboStateBytes;

	// NEW: per-WR matrix buffers (or CPU pointers if you prefer)
	GLuint ssboDpos; // WR's amatDpos array uploaded
	GLuint ssboDuv; // WR's amatDuv array uploaded

	int vertexCount;
	int cmat;
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
	struct WRBSG* pwrbsg;
	int cibnd;
	std::vector <int> aibnd;
	SAI *uvSai;     
	bool usesUvAnim;
	std::shared_ptr <WRBSG_GL> pwarp;
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
	float sMRD;
	float sCelBorderMRD;
	float gZOrder;
	int fDynamic;
	GRFGLOB grfglob;
	std::shared_ptr <GLEAM> gleam;
	RTCK rtck;
	struct SAA* psaa;
	// Object fog intensity
	float uFog;
	FGFN fgfn;
	float rSubglobRadius;
	WR* pwr = nullptr;
	std::shared_ptr <WRBG> pwrbg;
	// Number of submodels for model
	int csubglob;
	std::vector <SUBGLOB> asubglob;
	int csubcel;
	std::vector <SUBCEL> asubcel;
	// Ptr to instance model matrix
	std::shared_ptr <glm::mat4> pdmat;
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
	std::shared_ptr <WRBG> pwrbgFirst;
	int cpsaa;
	std::vector <SAA*> apsaa;
};

// Loads 3D model data from binary file
void LoadGlobsetFromBrx(GLOBSET* pglobset, ALO* palo, CBinaryInputStream* pbis);
// Converts strips to tri lists and stores 3D sub model in VRAM
void BuildSubGlob(GLOB* pglob, SUBGLOB* psubglob, SHD* pshd, std::vector <glm::vec3>& positions, std::vector <glm::vec3>& normals, std::vector <glm::vec4>& colors, std::vector <glm::vec2>& texcoords, std::vector <VTXFLG>& indexes, SUBPOSEF* subposef, std::vector <glm::vec3>& aposfPoses, std::vector <glm::vec3>& anormalfPoses, std::vector <float>& agWeights);
void BuildSubcel(GLOBSET* pglobset, SUBCEL* psubcel, int cposf, std::vector <glm::vec3>& aposf, int ctwef, std::vector <TWEF>& atwef, std::vector <SUBPOSEF>& asubposef, std::vector <glm::vec3>& aposfPoses, std::vector <float>& agWeights);
void BuildGlobsetSaaArray(GLOBSET* pglobset);
void PostGlobsetLoad(GLOBSET* pglobset, ALO* palo);
void UpdateGlobset(GLOBSET* pglobset, ALO* palo, float dt);

extern int  g_fogType;
extern bool g_fRenderModels;
extern bool g_fRenderCollision;
extern bool g_fRenderCelBorders;
extern bool g_fBsp;
extern float g_uAlpha;
extern SMP s_smpFade;
extern SMP g_smpAlphaFade;
extern glm::vec4 g_rgbaCel;