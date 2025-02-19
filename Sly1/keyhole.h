#pragma once
#include "lo.h"

struct TRI 
{
	int aipos[3];
};

struct KS 
{
	int ctri;
	std::vector <TRI> atri;
	glm::vec4 rgba;
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

KEYHOLE*NewKeyhole();
void InitKeyhole(KEYHOLE *pkeyhole);
int  GetKeyholeSize();
void LoadKeyholeFromBrx(KEYHOLE *pkeyhole, CBinaryInputStream *pbis);
void CloneKeyhole(KEYHOLE *pkeyhole, KEYHOLE *pkeyholeBase);
void DeleteKeyhole(KEYHOLE* pkeyhole);

static KEYHOLE *g_pkeyhole;