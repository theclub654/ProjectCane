#pragma once
#include "lo.h"
#include "vec.h"

class SHADOW
{
	public:
        struct SHD* pshd;
        float sNearRadius;
        float sFarRadius;
        glm::vec3 posCast;
        glm::vec3 normalCast;
        glm::vec3 vecUp;
        float sNearCast;
        float sFarCast;
        glm::vec3 posEffect;
        float sRadiusEffect;
        glm::mat4 matWorldToUv;
        OID oidDysh;
        struct DYSH* pdysh;
        DLE dle;
        int cqwVifs;
        struct SUR* psur;
};