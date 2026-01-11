#pragma once
#include "lo.h"

enum WREK 
{
    WREK_Nil = -1,
    WREK_Circle = 0,
    WREK_Bend = 1,
    WREK_Max = 2
};

struct ONZE 
{
    float uAmpl;
    float gFreq;
    float gPhase;
    float uRandom;
};

struct ONZ 
{
    LM lm;
    float uBias;
    int conze;
    ONZE aonze[4];
};

struct WREF
{
    glm::vec4 dpos;
    glm::vec4 duv;
};

struct WRBSG 
{
    // Base vertex positions for this subglob (what PS2 used pposad for)
    // Could be a pointer/view if owned elsewhere.
    std::vector<glm::vec3> basePositions;
    int vertexCount = 0;

    // Bindings: which WR + which imat controls this subglob's matrices
    // If multiple WRs can affect one glob, store multiple bindings.
    struct Binding
    {
        std::weak_ptr <WR> wr;  // or raw WR* if lifetime is guaranteed
        int imat = 0;
    };
    std::vector<Binding> bindings; // size == cmat typically

    // GPU state (replacement for apqwState)
    GLuint ssboState = 0;                 // vec4[cmat * vertexCount]
    std::vector <glm::vec4> stateCPU;      // same layout for updates
};

struct WRE
{
    WREK wrek;
    int  imat;

    // IMPORTANT: VU loads are 16 bytes. Use vec4.
    struct CIRCLE
    {
        glm::vec4 normal;   // xyz used, w ignored
        WREF awref[2];
        int  is;        // lane index
        float rgDot;
        float gFrequency;
        float uPhase;
    } circle;

    struct BEND
    {
        // These must sit in the same storage region as circle data.
        // If PS2 bend code reads "circle.normal", you want the same 16 bytes here.
        glm::vec4 normalBend;
        glm::vec4 normalSwivel;
        ONZ onzRadBend;
        ONZ onzRadSwivel;
    } bend;
};

class WR : public LO
{
	public:
        float tLastUpdate;
        int cwre;
        int cmat;
        int imatHalf;
        struct WRE awre[8];
        int iwreCur;
        glm::mat4 amatDpos[4];
        glm::mat4 amatDuv[4];
        std::shared_ptr<WRBG> pwrbgFirst;
        int fValuesChanged;
};

WR*  NewWr();
void InitWr(WR* pwr);
void CloneWr(WR* pwr, WR* pwrBase);
void AddWrCircleWarp(WR* pwr);
WRE* PwreGetWrCircle(WR* pwr, ENSK ensk);
glm::vec4* PwreGetWrCircleNormalVector(WR* pwr, ENSK ensk);
float* PwreGetWrCircleFrequency(WR* pwr, ENSK ensk);
float* PwreGetWrCirclePhase(WR* pwr, ENSK ensk);
glm::vec4* PwreGetWrCircleDpos0(WR* pwr, ENSK ensk);
glm::vec4* PwreGetWrCircleDpos1(WR* pwr, ENSK ensk);
glm::vec4* PwreGetWrCircleDuv0(WR* pwr, ENSK ensk);
glm::vec4* PwreGetWrCircleDuv1(WR* pwr, ENSK ensk);
void AddWrBendWarp(WR* pwr);
WRE* PwreGetWrBend(WR* pwr, ENSK ensk);
glm::vec4* PwreGetWrBendNormalBend(WR* pwr, ENSK ensk);
glm::vec2* PwreGetWrBendonzRadBendLM(WR* pwr, ENSK ensk);
float* PwreGetWrBendBenduBias(WR* pwr, ENSK ensk);
void AddWrBendNoise(WR* pwr, float uAmpl, float gFreq, float gPhase, float uRandom);
void AddOnzOnze(ONZ* ponz, float uAmpl, float gFreq, float gPhase, float uRandom);
glm::vec4* PwreGetWrBendNormalSwivel(WR* pwr, ENSK ensk);
glm::vec2* PwreGetWrBendonzRadSwivelLM(WR* pwr, ENSK ensk);
float* PwreGetWrBenduSwiveluBias(WR* pwr, ENSK ensk);
void AddWrSwivelNoise(WR* pwr, float uAmpl, float gFreq, float gPhase, float uRandom);
WRE* PwreGetWrCur(WR* pwr, ENSK ensk, WREK wrek);
void SetWrWavelength(WR* pwr, float sWavelength);
void GetWrWavelength(WR* pwr, float* psWavelength);
int  GetWrSize();
void ApplyWrGlob(WR* pwr, ALO* palo, GLOB* pglob);
float GFromOnz(ONZ* ponz);
float UBias(float u, float uBias);
float UNoise(float gFreq, float gPhase, float uRandom);
float Hash01(uint32_t n);
void UpdateWrStateVectors(WR* pwr);
void UpdateWrMatrixes(WR* pwr);
void DeleteWr(WR* pwr);

extern std::vector <WR*>g_pwrs;