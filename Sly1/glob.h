#pragma once
#include "shd.h"
#include "bis.h"

// Glob is just another word for model.
static int GlobCounter = 0;
class ALO;

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

struct SUBGLOB // NOT DONE
{
	GLuint VAO;
	GLuint VBO;
	GLuint VNO;
	GLuint VCB;
	GLuint TCB;
	GLuint EBO;
	GLuint gl_texture;

	glm::vec3 posCenter; // Submodel orgin
	float sRadius;
	std::vector <glm::vec3> vertexes;
	std::vector <glm::vec3> normals;
	std::vector <RGBA> vertexColors;
	std::vector <glm::vec2> texcoords;
	std::vector <VTXFLG> indexes;
	std::vector <byte> indices;
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
	std::vector<SUBGLOB> asubglob;
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
	std::vector <OID> mpibndoid;
	// Number of submodeles for a model
	int cglob;
	std::vector <GLOB> aglob;
	std::vector <GLOBI> aglobi;
	//LTFN ltfn;
	uint32_t grfglobset;
	RGBA rgbaCel;
	int cpose;
	std::vector <float> agPoses;
	std::vector <float> agPosesOrig;
	//WRBG *pwrbgFirst;
	int cpsaa;
	struct SAA** apsaa;
}; // NOT DONE

// Loads 3D models from binary file
void LoadGlobsetFromBrx(GLOBSET *pglobset, CBinaryInputStream *pbis, ALO* palo); // NOT FINISHED
// Converts tri strips to tri list
std::vector <byte> ConvertStripsToTriLists(std::vector <VTXFLG> indexes);
// Storing 3D models in VRAM
std::vector <SUBGLOB> MakeGLBuffers(std::vector<SUBGLOB> asubglob);