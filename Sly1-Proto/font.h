#pragma once
#include "shd.h"

struct SFR
{
    float rx;
    float ry;
};

struct CFont
{
    int m_dxCharUnscaled;
    int m_dxSpaceUnscaled;
    int m_dyUnscaled;
    int m_csfr;
    SFR m_asfr[4];
    int m_fGstest;
    uint64_t m_gstest;
    uint32_t m_z;
    float m_rxScale;
    float m_ryScale;
};

struct GLYFF
{
    short wch;
    short x;
    short y;
    short dx;
};

struct FONTF 
{
    // Texture ID
    short ibmp;
    // CLUT ID
    short iclut;
    byte dxChar;
    byte dxSpace;
    byte dy;
    byte bUnused;
    float rScale;
    uint32_t cglyff;
};

class CFontBrx
{
    public:
        CFont CFont;
        struct BMP* m_pbmp;
        struct CLUT* m_pclut;
        int m_cglyff;
        uint32_t m_grffont;

        void LoadFromBrx(CBinaryInputStream *pbis);
};

// Number of fonts in binary file
extern int g_cfontBrx;
// Font property's
extern std::vector <CFontBrx> g_afontBrx;
extern std::vector <CFontBrx> g_pfont;
extern std::vector <CFontBrx> g_pfontScreenCounters;
extern std::vector <CFontBrx> g_pfontJoy;
extern std::vector <GLYFF> g_aglyff;