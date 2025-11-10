#pragma once
#include <vector>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "shdanim.h"
#include "font.h"

void PostBlotsLoad();

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

// Render Priority
enum RP
{
    RP_Nil = -1,
    RP_DynamicTexture = 0,
    RP_Background = 1,
    RP_BlotContext = 2,
    RP_Opaque = 3,
    RP_Cutout = 4,
    RP_CelBorder = 5,
    RP_ProjVolume = 6,
    RP_OpaqueAfterProjVolume = 7,
    RP_CutoutAfterProjVolume = 8,
    RP_CelBorderAfterProjVolume = 9,
    RP_MurkClear = 10,
    RP_MurkOpaque = 11,
    RP_MurkFill = 12,
    RP_Translucent = 13,
    RP_TranslucentCelBorder = 14,
    RP_Blip = 15,
    RP_Foreground = 16,
    RP_WorldMap = 17,
    RP_Max = 18
};

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
    GRFZON grfzon;
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
    GRFZON grfzon;
    byte psm;
    byte cgsRow;
    short cgsPixels;
    // BMP size
    uint32_t cbPixels;
    // Offset to BMP in memory
    uint32_t baseOffset;

    std::vector <byte> shadowTexture;
    GLuint glShadowMap;

    std::vector <byte> diffuseTexture;
    GLuint glDiffuseMap;

    std::vector <byte> saturateTexture;
    GLuint glSaturateMap;
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
    glm::vec4 rgba;
    glm::vec4 rgbaVolume;
    uint32_t grfzon;
    OID oidAltSat;
    RP rp;
    byte ctex;
};

struct TEX : public TEXF
{
    struct SHD* pshd;
    std::vector <uint16_t> bmpIndex;
    std::vector <uint16_t> clutIndex;

    std::vector <BMP*>  abmp;
    std::vector <CLUT*> aclut;
};

// Shader property's
struct SHD : public SHDF
{
    std::vector <TEX> atex;
    int cshdp;
    int cframe;
    SAA *psaa;
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
void LoadTexFromBrx(TEX* ptex, CBinaryInputStream *pbis);
// Converts Custom Hue Saturation Value to RGBA Color
void ConvertUserHsvToUserRgb(glm::vec3 &pvecHSV, glm::vec3& pvecRGB);
// Returns a shader property from global shader vector
SHD* PshdFindShader(OID oid);
// Loads texture and shader property's from binary file
void LoadShadersFromBrx(CBinaryInputStream *pbis);
// Loads texture data from binary file
void LoadTexturesFromBrx(CBinaryInputStream *pbis);
// Make Texture
std::vector <byte> MakeBmp(BMP *pbmp, CBinaryInputStream* pbis);
// Make color pallete
std::vector <byte> MakePallete(CLUT *pclut, CBinaryInputStream* pbis);
// Make texture
void MakeTexture(GLuint& textureReference, TEX* ptex, BMP* pbmp, std::vector <byte>& texture, CLUT* pclut, bool fFlip, bool fMipMap, SHDK shdk, RP rp, CBinaryInputStream* pbis);

// Global variable which holds the number of CLUT's in a binary file
extern int g_cclut;
// Global vector for CLUT property's
extern std::vector <CLUT> g_aclut;
extern int g_grfzonShaders;
// Global variable which holds the number of BMP's in a binary file
extern int g_cbmp;
// Global variable which holds the number of shader's in a binary file
extern int g_cshd;
// Global vector for BMP property's
extern std::vector <BMP> g_abmp;
// Global vector for shader property's
extern std::vector <SHD> g_ashd;
// Global variable which holds the number of shader animation's in a binary file
extern int g_cpsaa;
// Global vector for shader animation property's
extern std::vector <SAA> g_apsaa;
// Table for texture property's
extern std::vector<TEX> g_atex;
// Start of texture data
extern size_t textureDataStart;
// Unswizzled CLUT indices
extern uint8_t csm1ClutIndices[256];