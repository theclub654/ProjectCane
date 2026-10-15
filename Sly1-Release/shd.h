#pragma once
#include <vector>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "spaprops.h"
#include "bis.h"
#include "font.h"
#include "util.h"
#include "shdanimdec.h"

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
enum RP : int
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

inline constexpr bool FIsCelBorderRenderPhase(RP rp)
{
    return rp == RP_CelBorder ||
        rp == RP_CelBorderAfterProjVolume ||
        rp == RP_TranslucentCelBorder;
}

// Shader animation type
enum SAAK
{
    SAAK_Nil = -1,
    SAAK_None = 0,
    SAAK_Loop = 1,
    SAAK_PingPong = 2,
    SAAK_Shuffle = 3,
    SAAK_Hologram = 4,
    SAAK_Eyes = 5,
    SAAK_Scroller = 6,
    SAAK_Circler = 7,
    SAAK_Looker = 8,
    SAAK_Max = 9
};

enum RKO
{
    RKO_OneWay = 0,
    RKO_ThreeWay = 1
};

// Color property's
struct RGBA
{
    byte bRed;
    byte bGreen;
    byte bBlue;
    byte bAlpha;
};

struct TCX
{
    float du;
    float dv;
};

struct SAI
{
    int grfsai;
    struct SHD* pshd;
    int iframe;
    TCX tcx;
    glm::vec2 uvOffset;
    SAI* psaiNext;
};

// Shader animation
struct SAA
{
    union
    {
        struct VTSAA* pvtsaa;
        struct VTLOOP* pvtloop;
        struct VTPINGPONG* pvtpingpong;
        struct VTSHUFFLE* pvtshuffle;
        struct VTHOLOGRAM* pvthologram;
        struct VTSCROLLER* pvtscroller;
        struct VTCIRCLER* pvtcircler;
        struct VTLOOKER* pvtlooker;
        struct VTEYES* pvteyes;
    };

    float tUpdated;
    SAAK saak;
    OID oid;
    SAI sai;
};

// Resolved PCBRX RGBA8 image. Palette lookup, GS alpha expansion and optional
// font flipping are performed offline by the file rebuilder.
struct BMP
{
    uint32_t bmpWidth;
    uint32_t bmpHeight;
    GRFZON grfzon;
    uint32_t mipCount;
    uint32_t cbPixels;
    uint32_t baseOffset;
    uint32_t textureFlags;
    uint32_t textureFormat;

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
    uint16_t cibmp;
};

struct SHDP 
{
    int cqwRegs;
    std::vector <uint16_t> aaqwRegs;
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
    std::vector <int32_t> ambientImageIndex;
    std::vector <int32_t> diffuseImageIndex;
    std::vector <int32_t> saturateImageIndex;

    std::vector <BMP*>  abmp;

    // Per-frame handles reference resolved images from the PCBRX image table.
    std::vector<GLuint> glDiffuseMap;
    std::vector<std::vector<byte>> diffuseTexture;

    // Three-way shaders retain distinct ambient, diffuse and saturate images.
    std::vector<GLuint> glShadowMap;
    std::vector<std::vector<byte>> shadowTexture;
    std::vector<GLuint> glSaturateMap;
    std::vector<std::vector<byte>> saturateTexture;
};

// Shader property's
struct SHD : public SHDF
{
    std::vector <TEX> atex;
    int cshdp;
    std::vector <SHDP> ashdp;
    int cframe;
    SAA* psaa;
};

// Delete shader data
void UnloadShaders();
// Loads resolved PC image descriptors from the rebuilt file.
void LoadBitmapsFromBrx(CBinaryInputStream* pbis);
// Loads font property's from binary file
void LoadFontsFromBrx(CBinaryInputStream* pbis); // GOTTA COME BACK TO THIS
// Loads texture tables from binary file
void LoadTexFromBrx(TEX* ptex, CBinaryInputStream* pbis);
// Converts Custom Hue Saturation Value to RGBA Color
void ConvertUserHsvToUserRgb(glm::vec3& pvecHSV, glm::vec3& pvecRGB);
// Returns a shader property from global shader vector
SHD* PshdFindShader(OID oid);
// Loads texture and shader property's from binary file
void LoadShadersFromBrx(CBinaryInputStream* pbis);
void SetSaiIframe(SAI* psai, int iframe);
// Loads texture data from binary file
void LoadTexturesFromBrx(CBinaryInputStream* pbis);
void ApplyAnisotropicFilteringSettings(int level);
void ApplyTextureFilteringSettings(int mode);
int MaxSupportedAnisotropicFiltering();
void UpdateShaders(float dt);

extern int g_grfzonShaders;
// Global variable which holds the number of BMP's in a binary file
extern int g_cbmp;
// Global variable which holds the number of shader's in a binary file
extern int g_cshd;
// Global vector for BMP property's
extern std::vector <BMP> g_abmp;
// Global vector for shader property's
extern std::vector <SHD> g_ashd;
// Table for texture property's
extern std::vector<TEX> g_atex;
// Global variable which holds the number of shader animation's in a binary file
extern int g_cpsaa;
// Global vector for shader animation property's
extern std::vector <SAA*> g_apsaa;
extern std::vector<std::shared_ptr<SAA>> g_apsaaSw;
extern SAI* g_psaiUpdate;
extern SAI* g_psaiUpdateTail;
extern int g_anisotropicFiltering;
