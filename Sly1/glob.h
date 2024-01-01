#pragma once
#include "bis.h"
#include "gl.h"

class ALO;
// Glob is just another word for model.

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

struct LTFN 
{
	float ruShadow;
	float ruMidtone;
	float ruHighlight;
	float ruUnused;
	float duShadow;
	float duMidtone;
	float duHighlight;
	float duUnused;
};

struct VERTICE
{
	glm::vec3 pos;
	glm::vec3 normal;
	RGBA color;
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
	struct ALO* palo;
	struct GLOB* pglob;
	OID oid;
	struct WR* pwr;
	int cmat;
	int fDpos;
	int fDuv;
	WEKI weki;
	struct WRBG* pwrbgNextGlobset;
	struct WRBG* pwrbgNextWr;
};

struct SUBGLOB // NOT DONE
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	glm::vec3 posCenter; // Submodel orgin
	float sRadius;
	std::vector <VERTICE> vertices;
	std::vector <glm::vec3> vertexes;
	std::vector <glm::vec3> normals;
	std::vector <RGBA> vertexColors;
	std::vector <glm::vec2> texcoords;
	std::vector <VTXFLG> indexes;
	std::vector <uint16_t> indices;

	uint32_t unSelfIllum;
	struct SHD* pshd;
	struct RSGLD *prsgld;
	struct WRBSG *pwrbsg;
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
	RTCK rtck;
	struct SAA* psaa;
	float uFog;
	//FGFN fgfn;
	float rSubglobRadius;
	struct WRBG *pwrbg;
	// Number of submodels for model
	int csubglob;
	std::vector<SUBGLOB> asubglob;
	int csubcel;
	//SUBCEL *asubcel;
	// Object world space coordinates
	std::vector <glm::mat4> pdmat;
	//BLOT *pblot;
	OID oid;
	char* pchzName;
}; // NOT DONE

struct GLOBI // NOT DONE
{
	int grfzon;
	//SUBGLOBI *asubglobi;
	int cframeStaticLights;
	TWPS twps;
	float uAlpha;
	float tUnfade;
}; // NOT DONE

struct GLOBSET // NOT DONE
{
	int cbnd;
	struct BND *abnd;
	std::vector <OID> mpibndoid;
	uint64_t cglob;
	std::vector <GLOB> aglob;
	std::vector <GLOBI> aglobi;
	LTFN ltfn;
	uint32_t grfglobset;
	RGBA rgbaCel;
	int cpose;
	std::vector <float> agPoses;
	std::vector <float> agPosesOrig;
	struct WRBG *pwrbgFirst;
	int cpsaa;
	struct SAA** apsaa;
};

// Loads 3D model data from binary file
void LoadGlobsetFromBrx(GLOBSET* pglobset, CBinaryInputStream* pbis, ALO* palo); // NOT FINISHED
// Converts strips to tri lists
void BuildSubGlob(GLOBSET *pglobset, SHD* pshd, std::vector<VERTICE> &vertices , std::vector <glm::vec3> &vertexes, std::vector <glm::vec3> &normals, std::vector <RGBA> &vertexColors, std::vector <glm::vec2> &texcoords, std::vector <VTXFLG> &indexes, std::vector<uint16_t> &indices);
// Storing 3D models in VRAM
void MakeGLBuffers(SUBGLOB* subglob);
// Draws Model
void DrawGlob(GLOBSET* pglobset, glm::mat3 mat, glm::vec3 pos); // NOT FINISHED