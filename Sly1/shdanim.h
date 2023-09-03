#pragma once
#include "spaprops.h"
#include "bis.h"

// Shader animation type
enum SAAK {
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
struct loopf {
    float dtLoopMin;
    float dtLoopMax;
    float dtPauseMin;
    float dtPauseMax;
    short iframeStart;
};
struct pingpongf {
    float dtPingpongMin;
    float dtPingpongMax;
    float dtPauseMin;
    float dtPauseMax;
    short iframeStart;
};
struct shufflef {
    float dtPauseMin;
    float dtPauseMax;
};
struct hologramf {
    float dradAdjust;
    uint32_t cSymmetry;
};
struct eyesf {
    float dtBlink;
    float dtOpenMin;
    float dtOpenMax;
    float uDoubleBlink;
    uint16_t oidOther;
};
struct scrollerf {
    float svu;
    float svv;
    float duMod;
    float dvMod;
};
struct circlerf {
    float sw;
    float sRadius;
    float du;
    float dv;
};
struct lookerf {
    float uCenter;
    float vCenter;
    float uMin;
    float uMax;
    float vMin;
    float vMax;
};
// Shader animation face
struct SAAF
{
    uint16_t oid;
    short fInstanced;
    loopf loopf;
    pingpongf pingpongf;
    shufflef shufflef;
    hologramf hologramf;
    eyesf eyesf;
    scrollerf scrollerf;
    circlerf circlerf;
    lookerf lookerf;
};
struct TCX
{
    float du;
    float dv;
};
struct SAI
{
    int grfsai;
    struct SHD *pshd;
    int iframe;
    TCX tcx;
    SAI *psaiNext;
};

// Shader animation
struct SAA
{
	float tUpdated;
    SAAK saak;
    OID oid;
    SAI sai;
};

// Returns size and type of shader animation
int CbFromSaak(SAAK saak);
// Loads shader animation from binary file
void PsaaLoadFromBrx(CBinaryInputStream *pbis); // GOTTA COME BACK TO THIS