#pragma once
#include "lo.h"

class MAP : public LO
{
	public:
        int cpos;
        glm::vec3* apos;
        int ctri;
        struct TRI* atri;
        int cmbsp;
        struct MBSP* ambsp;
        int cbEach;
        char* mpiposab;
};

void* NewMap();
void DeleteMap(LO* plo);