#pragma once
#include <iostream>
#include <vector>
#include "bis.h"
#include "shdanim.h"
#include "font.h"

// Color property's
struct RGBA 
{
    byte bRed;
    byte bGreen;
    byte bBlue;
    byte bAlpha;
};
// CLUT property
struct CLUT
{
    uint32_t grfzon;
    // Number of colors in a CLUT
    short numColors;
    // Color size of the CLUT
    short colorSize;
    // Ptr to CLUT
    uint32_t baseOffset;
};
// Texture property
struct BMP
{
    // BMP width
    short bmpWidth;
    // BMP height
    short bmpheight;
    uint32_t grfzon;
    byte psm;
    byte cgsRow;
    short cgsPixels;
    // BMP size
    uint32_t cbPixels;
    // Offset to BMP in memory MIGHT NOT BE NEEDED
    uint32_t baseOffset;
};
struct TEXF
{
    uint16_t oid;
    short grftex;
    // Number of bmp's
    byte cibmp;
    // Number of CLUT's
    byte ciclut;
    std::vector<uint16_t> bmpIndex;
    std::vector<uint16_t> clutIndex;
};
struct SHDF
{
    byte shdk;
    byte grfshd;
    uint16_t oid;
    RGBA rgba;
    RGBA rgbaVolume;
    uint32_t grfzon;
    uint16_t oidAltSat;
    byte rp;
    byte ctex;
};
// Shader property's
struct SHD : public SHDF
{
    SHDF SHD;
    struct TEX *atex;
    int cshdp;
    int cframe;
    SAA *psaa;
};
struct TEX : public TEXF
{
    TEXF texf;
    SHD pshd;
    BMP **apbmp;
    CLUT **apclut;
};


// Loads CLUT entry data from binary file
void LoadColorTablesFromBrx(CBinaryInputStream *pbis);
// Loads texture entry data from binary file
void LoadBitmapsFromBrx(CBinaryInputStream *pbis);
// Loads font property's from binary file
void LoadFontsFromBrx(CBinaryInputStream *pbis);// GOTTA COME BACK TO THIS
// Loads texture tables from binary file
void LoadTexFromBrx(CBinaryInputStream *pbis, SHD shader);
// Loads texture and shader data from binary file
void LoadShadersFromBrx(CBinaryInputStream *pbis);


// Global variable which holds the number of CLUT's in a binary file
static int g_cclut;
// Global vector for CLUT property's
static std::vector <CLUT> g_aclut;
// Global variable which holds the number of BMP's in a binary file
static int g_cbmp;
// Global vector for BMP property's
static std::vector <BMP> g_abmp;
// Global variable which holds the number of shader's in a binary file
static int g_cshd;
// Global vector for shader property's
static std::vector <SHD> g_ashd;
// Global variable which holds the number of shader animation's in a binary file
static int g_cpsaa;
// Global vector for shader animation property's
static std::vector <SAA> g_apsaa;
// Table for texture property's
static std::vector<TEX> g_atex;