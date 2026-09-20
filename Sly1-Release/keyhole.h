#pragma once
#include "map2d.h"

struct KEYHOLEGL
{
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;

	~KEYHOLEGL();
};

struct KS 
{
	int ctri;
	std::vector <TRI> atri;
	glm::vec4 rgba;

	std::shared_ptr<KEYHOLEGL> pgl;
};

class KEYHOLE : public LO
{
	public:
	int cpos;
	std::vector <glm::vec4> apos;
	glm::vec4 posMin;
	glm::vec4 posMax;
	float dx;
	KS mpkpks[5];
};

// GOTTA COME BACK TO THIS
KEYHOLE*NewKeyhole();
void InitKeyhole(KEYHOLE *pkeyhole);
int  GetKeyholeSize();
void LoadKeyholeFromBrx(KEYHOLE *pkeyhole, CBinaryInputStream *pbis);
void CloneKeyhole(KEYHOLE *pkeyhole, KEYHOLE *pkeyholeBase);
void DrawKeyholeMask(KEYHOLE* pkeyhole, float x, float y, float rScale, float uAlpha);
void DrawKeyhole(KEYHOLE* pkeyhole, float uClosed);
void DeleteKeyhole(KEYHOLE* pkeyhole);

extern KEYHOLE* g_pkeyhole;
extern glm::vec4 g_rgbaKeyholeEyes;
extern glm::vec4 g_rgbaKeyholeMask;
