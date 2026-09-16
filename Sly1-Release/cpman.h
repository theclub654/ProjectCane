#pragma once
#include "cplcy.h"
// Manual camera

struct GLFWwindow;

struct CPMAN : public CPLCY
{
    CPMT cpmt;
    class ALO* paloOrbit;
    int cframeStatus;
};

void SetCpmanCpmt(CPMAN* pcpman, CPMT cpmt);
// Update manual camera
void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt);

struct VTCPMAN
{
    void (*pfnActivateCplcy)() = nullptr;
    void (*pfnDeactivateCplcy)() = nullptr;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*UpdateCpman)(CPMAN*, CPDEFI*, float) = UpdateCpman;
};

inline VTCPMAN g_vtcpman;
