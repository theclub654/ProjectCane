#pragma once
#include "cplcy.h"

struct CPASEG : public CPLCY
{
    struct CAMERA* pcamera;
};

void ActivateCpaseg(CPASEG* pcpaseg, void* pv);
void DeactivateCpaseg(CPASEG* pcpaseg, void* pv);
void UpdateCpaseg(CPASEG* pcpaseg, CPDEFI* pcpdefi, JOY* pjoy, float dt);

struct VTCPASEG
{
    void (*pfnActivateCpaseg)(CPASEG*, void*) = ActivateCpaseg;
    void (*pfnDeactivateCpaseg)(CPASEG*, void*) = DeactivateCpaseg;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*pfnUpdateCpaseg)(CPASEG*, CPDEFI*, JOY*, float) = UpdateCpaseg;
};

inline VTCPASEG  g_vtcpaseg;