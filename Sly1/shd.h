#pragma once
#include <vector>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "shdanim.h"
#include "font.h"

enum SHDK 
{
    SHDK_Nil = -1,
    SHDK_ThreeWay = 0,
    SHDK_Prelit = 1,
    SHDK_Shadow = 2,
    SHDK_SpotLight = 3,
    SHDK_ProjectedVolume = 4,
    SHDK_CreateTexture = 5,
    SHDK_Background = 6,
    SHDK_Foreground = 7,
    SHDK_WorldMap = 8,
    SHDK_MurkClear = 9,
    SHDK_MurkFill = 10,
    SHDK_Max = 11
};

// Color property's
struct RGBA 
{
    uint16_t bRed;
    uint16_t bGreen;
    uint16_t bBlue;
    uint16_t bAlpha;
};

// CLUT property
struct CLUT
{
    uint32_t grfzon;
    // Number of colors in a CLUT
    short numColors;
    // Color size of the CLUT
    short colorSize;
    // Ptr to CLUT in memory
    uint32_t baseOffset;
};

// Texture property
struct BMP
{
    // BMP width
    short bmpWidth;
    // BMP height
    short bmpHeight;
    uint32_t grfzon;
    byte psm;
    byte cgsRow;
    short cgsPixels;
    // BMP size
    uint32_t cbPixels;
    // Offset to BMP in memory
    uint32_t baseOffset;
};

struct TEXF
{
    OID oid;
    short grftex;
    // Number of bmp's
    byte cibmp;
    // Number of CLUT's
    byte ciclut;
};

struct SHDF
{
    SHDK shdk;
    byte grfshd;
    OID oid;
    RGBA rgba;
    RGBA rgbaVolume;
    uint32_t grfzon;
    OID oidAltSat;
    byte rp;
    byte ctex;
};

struct TEX : public TEXF
{
    struct SHD* pshd;
    std::vector <uint16_t> bmpIndex;
    std::vector <uint16_t> clutIndex;
};

// Shader property's
struct SHD : public SHDF
{
    SHDF SHD;
    std::vector <TEX> atex;
    int cshdp;
    int cframe;
    SAA* psaa;

    GLuint glAmbientTexture;
    GLuint glDiffuseTexture;
    GLuint glSaturateTexture;
};

// Delete shader data
void UnloadShaders();
// Loads CLUT entry data from binary file
void LoadColorTablesFromBrx(CBinaryInputStream *pbis);
// Loads texture entry data from binary file
void LoadBitmapsFromBrx(CBinaryInputStream *pbis);
// Loads font property's from binary file
void LoadFontsFromBrx(CBinaryInputStream *pbis); // GOTTA COME BACK TO THIS
// Loads texture tables from binary file
void LoadTexFromBrx(TEX* ptex, CBinaryInputStream* pbis);
// Converts Custom Hue Saturation Value to RGBA Color
void ConvertUserHsvToUserRgb(glm::vec3& pvecHSV, glm::vec3& pvecRGB);
// Returns a shader property from global shader vector
SHD* PshdFindShader(OID oid);
// Loads texture and shader property's from binary file
void LoadShadersFromBrx(CBinaryInputStream *pbis);
// Loads texture data from binary file
void LoadTexturesFromBrx(CBinaryInputStream* pbis);
// Make Texture
std::vector <byte> MakeBmp(uint32_t bmpIndex, CBinaryInputStream* pbis);
// Make color pallete
std::vector <byte> MakePallete(uint32_t clutIndex, CBinaryInputStream* pbis);
void MakeTexture(GLuint &textureReference, int16_t clutIndex, int16_t bmpIndex, CBinaryInputStream* pbis);

// Global variable which holds the number of CLUT's in a binary file
static int g_cclut;
// Global vector for CLUT property's
static std::vector <CLUT> g_aclut;
static int g_grfzonShaders;
// Global variable which holds the number of BMP's in a binary file
static int g_cbmp;
// Global vector for BMP property's
static std::vector <BMP> g_abmp;
// Global variable which holds the number of shader's in a binary file
static int g_cshd;
// Global vector for shader property's
extern std::vector <SHD> g_ashd;
// Global variable which holds the number of shader animation's in a binary file
static int g_cpsaa;
// Global vector for shader animation property's
static std::vector <SAA> g_apsaa;
// Table for texture property's
static std::vector<TEX> g_atex;
// Unswizzled CLUT indices
static uint8_t csm1ClutIndices[256];
// Start of texture data
static size_t textureDataStart;