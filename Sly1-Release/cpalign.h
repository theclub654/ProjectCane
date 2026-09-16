#pragma once
#include "cplcy.h"

struct CPALIGN : public CPLCY
{
    glm::vec3 posLocal;
    glm::mat3 matLocal;
};

void InitCpalign(CPALIGN* pcpalign, CM* pcm);
void ActivateCpalign(CPALIGN* pcpalign, void* pv);
void UpdateCpalign(CPALIGN* pcpalign, CPDEFI* pcpdefi, JOY* pjoy, float dt);

struct VTCPALIGN
{
    void (*pfnActivateCpalign)(CPALIGN*, void*) = ActivateCpalign;
    void (*pfnDeactivateCplcy)() = nullptr;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*pfnUpdateCpalign)(CPALIGN*, CPDEFI*, JOY*, float) = UpdateCpalign;
};

inline VTCPALIGN g_vtcpalign;
