#pragma once
#include <unordered_map>
#include "shd.h"
#include "gl.h"

enum JH 
{
    JH_Nil = -1,
    JH_Left = 0,
    JH_Center = 1,
    JH_Right = 2,
    JH_Max = 3
};
enum JV 
{
    JV_Nil = -1,
    JV_Top = 0,
    JV_Center = 1,
    JV_Bottom = 2,
    JV_Max = 3
};

struct SFR
{
    float rx;
    float ry;
};

struct CTextBox
{
    float m_x;
    float m_y;
    float m_dx;
    float m_dy;
    glm::vec4 m_rgba;
    JH m_jh;
    JV m_jv;

    void SetPos(float x, float y);
    void SetSize(float dx, float dy);
    void SetTextColor(glm::vec4 *rgba);
    void SetHorizontalJust(JH jh);
    void SetVerticalJust(JV jv);
};

class CTextEdge
{
    public:

    // Pointer to the font used for rendering the edge (glyph outlines or shadows)
    struct CFontBrx *m_pfont;
    // Character to draw the edge for (usually a single glyph)
    char  m_ch;
    // Extra horizontal offset (in pixels or units) to apply to the glyph's position for the edge effect
    float m_dxExtra;
    // Extra vertical offset to apply to the glyph's position for the edge effect
    float m_dyExtra;
    // Horizontal scaling factor for the edge glyph (e.g., to enlarge or shrink the edge)
    float m_rxScaling;
    // Vertical scaling factor for the edge glyph
    float m_ryScaling;
    // Color used for text edge
    glm::vec4 m_rgba;

    void SetPos(float x, float y);
    void SetSize(float dx, float dy);
    void SetTextColor(glm::vec4 *rgba);
    void SetHorizontalJust(JH jh);
    void SetVerticalJust(JV jv);
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

struct GLYFF
{
    // Character code
    short wch;
    // Glyph x offset in texture (in pixels)
    short x;
    // Glyph y offset in texture (in pixels)
    short y;
    // Glyph width in pixels
    short dx;
};

struct CFont
{
    // Default character width unscaled
    int m_dxCharUnscaled;
    // Default space width unscaled
    int m_dxSpaceUnscaled;
    // Default font height unscaled
    int m_dyUnscaled;
    int m_csfr;
    SFR m_asfr[4];
    int m_fGstest;
    uint64_t m_gstest;
    uint32_t m_z;

    // Scale X
    float m_rxScale;
    // Scale Y
    float m_ryScale;
};

class CFontBrx : public CFont
{
        public:

        // Ptr to font texture
        struct BMP  *m_pbmp;
        struct CLUT *m_pclut;

        // Number of glyffs
        int m_cglyff;
        std::unordered_map<uint16_t, GLYFF> m_aglyff;

        uint32_t m_grffont;

        // Loads glyff data
        void LoadFromBrx(CBinaryInputStream *pbis);
        void PostLoad();
        // Returns a glyff from glyff buffer
        GLYFF* PglyffFromCh(char ch);
        // Calculates and returns the total width of a string
        float DxFromPchz(char *pchz);
        float DxFromCh(char ch);
        CFontBrx* PfontClone(float rx, float ry);
        void FValid(char ch);
        void SetupDraw();
        float DxDrawCh(char ch, float xChar, float yChar, glm::vec4 &rgba);
        void EdgeRect(CTextEdge* pte, CTextBox* ptbx);
        void PushScaling(float rx, float ry);
        void CopyTo(CFontBrx *pfontDst);
        void GetExtents(char *pchz, float* pdx, float* pdy, float dxMax);
        void PopScaling();
        int  ClineWrapPchz(char *pchz, float dx);
        float DxMaxLine(char *pchz);
        float DyWrapPchz(char* pchz, float dx);
        void DrawPart(float x0, float y0, float x1, float y1, float s0, float t0, float s1, float t1, glm::vec4 &color);
        void DrawPchz(char* pchz, CTextBox* ptbx);
        void CleanUpDraw();
};

class CRichText
{
    public:

    char* m_achz;
    char* m_pchCur;
    CFontBrx* m_pfontCur;
    CFontBrx* m_pfontBase;
    CFontBrx  m_fontOther;
    glm::vec4 m_rgbaCur;
    glm::vec4 m_rgbaSet;
    glm::vec4 m_rgbaBase;
    glm::vec4 m_rgbaOther;

    CRichText(char* achz, CFontBrx* pfont);

    void  GetExtents(float* pdx, float* pdy, float dxMax);
    int   ClineWrap(float dx);
    float DxMaxLine();
    char  ChNext();
    void  SetBaseColor(glm::vec4* rgba);
    int   Cch();
    void  Trim(int cch);
    void  Reset();
    void  Draw(CTextBox* ptbx);
};

void RenderGlyphQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, const glm::vec4 &color);

// Number of fonts in binary file
extern int g_cfontBrx;
// Font property's
extern std::vector <CFontBrx> g_afontBrx;

extern CFontBrx *g_pfont;
extern CFontBrx *g_pfontScreenCounters;
extern CFontBrx *g_pfontJoy;

extern SFR g_sfrOne;

extern GLuint u_projectionLoc;
extern GLuint u_modelLoc;
extern GLuint uvRectLoc;
extern GLuint blotColorLoc;
extern GLuint whiteTex;