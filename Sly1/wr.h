#pragma once
#include "lo.h"

enum WREK 
{
    WREK_Nil = -1,
    WREK_Circle = 0,
    WREK_Bend = 1,
    WREK_Max = 2
};

struct WRE
{
    WREK wrek;
    int imat;
    union
    {

    };
};

class WR : public LO
{
	public:
        float tLastUpdate;
        int cwre;
        int cmat;
        int imatHalf;
        struct WRE awre[8];
        int iwreCur;
        glm::mat4 amatDpos[4];
        glm::mat4 amatDuv[4];
        struct WRBG* pwrbgFirst;
        int fValuesChanged;
};

void* NewWr();
void InitWr(WR* pwr);
void DeleteWr(LO* plo);