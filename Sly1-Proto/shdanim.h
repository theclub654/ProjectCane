#pragma once
#include "spaprops.h"
#include "bis.h"

struct VTSAA
{

};

struct VTLOOP
{
    
};

struct VTPINGPONG
{

};

struct VTSHUFFLE
{

};

struct VTHOLOGRAM
{

};

struct VTSCROLLER
{

};

struct VTCIRCLER
{

};

struct VTLOOKER
{

};

struct VTEYES
{

};

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
enum EYESS
{
    EYESS_Nil = -1,
    EYESS_Open = 0,
    EYESS_Closing = 1,
    EYESS_Closed = 2,
    EYESS_Opening = 3,
    EYESS_Max = 4
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

struct LOOP : public SAA
{
    float dtLoopMin;
    float dtLoopMax;
    float dtPauseMin;
    float dtPauseMax;
    float sviframe;
    float gframe;
    float dtPauseRequested;
    float dtPause;
};

struct PINGPONG : public SAA
{
    float dtPingpongMin;
    float dtPingpongMax;
    float dtPauseMin;
    float dtPauseMax;
    float sviframe;
    float gframe;
    float dtPauseRequested;
    float dtPause;
};

struct SHUFFLE : public SAA
{
    float dtPauseMin;
    float dtPauseMax;
    float dtPause;
};

struct HOLOGRAM : public SAA
{
    float dradAdjust;
    float dradSymmetry;
    float dradFrame;
};

struct EYES : public SAA
{
    float dtBlink;
    float dtOpenMin;
    float dtOpenMax;
    float uDoubleBlink;
    OID oidOther;
    SAI saiOther;
    int cframe;
    EYESS eyess;
    float tEyess;
    float dtOpen;
    float sviframe;
    float gframe;
    float uClosed;
};

struct SCROLLER : public SAA
{
    float svu;
    float svv;
    float duMod;
    float dvMod;
    float svuMaster;
    float svvMaster;
};

struct CIRCLER : public SAA
{
    float sw;
    float sRadius;
    float du;
    float dv;
};

struct LOOKER
{
    float uCenter;
    float vCenter;
    float duMin;
    float duMax;
    float dvMin;
    float dvMax;
    int cvtx;
    struct UVQ* puvqd;
    struct POSAD* pposad;
};

// Shader animation face
struct SAAF
{
    uint16_t oid;
    short fInstanced;
    union
    {
        LOOP loopf;
        PINGPONG pingpongf;
        SHUFFLE shufflef;
        HOLOGRAM hologramf;
        EYES eyesf;
        SCROLLER scrollerf;
        CIRCLER circlerf;
        LOOKER lookerf;
    };
};

// Returns size and type of shader animation
int CbFromSaak(SAAK saak);
// Loads shader animation from binary file
void PsaaLoadFromBrx(CBinaryInputStream *pbis); // GOTTA COME BACK TO THIS