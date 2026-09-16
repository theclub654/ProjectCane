#pragma once
#include "aseg.h"
#include "blend.h"

struct BL
{
	float u;
	struct ASEG* paseg;
	struct CHN* pchn;
};

struct BLSS : public BL
{
	glm::vec3 v;
};
struct BLCL : public BL
{
	float sv;
};
struct BLRUN : public BL
{
	glm::vec3 v;
	glm::vec3 posPlant;
	MRSG mpfootsmrsgLeft[2];
	MRSG mpfootsmrsgRight[2];
};

class ASEGBL : public ASEG
{
    public:
    int cbBl;
    int cbl;
	std::vector<std::byte> abl;
    int cmrsgc;
    std::vector <MRSGC> amrsgc;
};

ASEGBL* NewAsegbl();
void CloneAsegbl(ASEGBL* pasegbl, ASEGBL* pasegblBase);
void DeleteAsegbl(ASEGBL* pasegbl);