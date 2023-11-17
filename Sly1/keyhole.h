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
		glm::vec4 apos;
		glm::vec4 posMin;
		glm::vec4 posMax;
		float dx;
		KS mpkpks[5];
};

void InitKeyhole(KEYHOLE *pkeyhole);
void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis);

static KEYHOLE *g_pkeyhole;