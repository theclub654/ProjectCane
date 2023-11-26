#pragma once
#include "lo.h"
#include "vec.h"

struct TRI 
{
	int aipos[3];
};
struct KS 
{
	int ctri;
	TRI *atri;
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

void* NewKeyhole();
void InitKeyhole(KEYHOLE *pkeyhole);
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);
void DeleteKeyhole(LO* plo);

static KEYHOLE *g_pkeyhole;