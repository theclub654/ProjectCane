#pragma once
#include "alo.h"

class HUBSEL : public ALO
{
	public:
    GAMEWORLD gameWorldSelected;
    int32_t fSelectionAccepted;
    float tSelectionChange;     
    SM* psmWorld;              
    SMA* psmaWorld;            
    SM* psmSecondary;          
    SMA* psmaSecondary;         
    SM* psmDisplay0;           
    SMA* psmaDisplay0;          
    SM* psmDisplay1;           
    SMA* psmaDisplay1;
    GAMEWORLD gameWorldMax;          
    GAMEWORLD gameWorldDisplay;      
    uint32_t  field_304;             
    DIALOG* pdialogPending;        
    uint32_t  field_30C;             
    ALO* apaloWorld[5];         
    ASEG* apasegWorld[6];        
    DIALOG* apdialogProgress[4];   
    uint32_t  field_34C;             

};

HUBSEL*NewHubSel();
void CloneHubSel(HUBSEL* phubsel, HUBSEL* phubselBase);
void PostHubSelLoad(HUBSEL* phubsel);
void UpdateHubselUi(HUBSEL* phubsel);
void UpdateHubselSelection(HUBSEL* phubsel, JOY* pjoy);
void ConfirmHubselSelection(HUBSEL* phubsel);
bool FHubselWorldAvailable(GAMEWORLD gameWorld);
void SetupHubsel(HUBSEL* phubsel);
//GOTTA COME BACK TO THIS WHEN DONE SOUND
void SetHubselSelection(HUBSEL* phubsel, GAMEWORLD gameworld, int fAccepted);
void UnlockAllHubWorlds();
void DeleteHubSel(HUBSEL* phubsel);

inline SNIP s_asnipHubsel[18] =
{
    { 0x00, (OID)0x4F9, offsetof(HUBSEL, psmWorld) },
    { 0x00, (OID)0x4FA, offsetof(HUBSEL, psmSecondary) },
    { 0x00, (OID)0x4FB, offsetof(HUBSEL, psmDisplay0) },
    { 0x01, (OID)0x4FC, offsetof(HUBSEL, psmDisplay1) },
    { 0x00, (OID)0x50B, offsetof(HUBSEL, apasegWorld[1]) },
    { 0x00, (OID)0x50C, offsetof(HUBSEL, apasegWorld[2]) },
    { 0x00, (OID)0x50D, offsetof(HUBSEL, apasegWorld[3]) },
    { 0x00, (OID)0x50E, offsetof(HUBSEL, apasegWorld[4]) },
    { 0x00, (OID)0x50F, offsetof(HUBSEL, apasegWorld[5]) },
    { 0x01, (OID)0x510, offsetof(HUBSEL, apaloWorld[0]) },
    { 0x01, (OID)0x511, offsetof(HUBSEL, apaloWorld[1]) },
    { 0x01, (OID)0x512, offsetof(HUBSEL, apaloWorld[2]) },
    { 0x01, (OID)0x513, offsetof(HUBSEL, apaloWorld[3]) },
    { 0x01, (OID)0x514, offsetof(HUBSEL, apaloWorld[4]) },
    { 0x07, (OID)0x33B, offsetof(HUBSEL, apdialogProgress[0]) },
    { 0x07, (OID)0x33C, offsetof(HUBSEL, apdialogProgress[1]) },
    { 0x07, (OID)0x33D, offsetof(HUBSEL, apdialogProgress[2]) },
    { 0x07, (OID)0x33E, offsetof(HUBSEL, apdialogProgress[3]) },
};

static const int g_aoidHubselState[GAMEWORLD_Max][2] =
{
    { 1279, 1285 }, // Intro
    { 1280, 1286 }, // Underwater
    { 1281, 1287 }, // Muggshot
    { 1282, 1288 }, // Voodoo
    { 1283, 1289 }, // Snow
    { 1284, 1290 }, // Clockwerk
};

static constexpr bool g_fUnlockAllHubWorlds = false;