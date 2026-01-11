#pragma once
#include "shd.h"
#include "clock.h"

enum EYESS
{
    EYESS_Nil = -1,
    EYESS_Open = 0,
    EYESS_Closing = 1,
    EYESS_Closed = 2,
    EYESS_Opening = 3,
    EYESS_Max = 4
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

struct SAAF
{
    uint16_t oid;
    short fInstanced;
    union
    {
        union LOOPF
        {
            float dtLoopMin;
            float dtLoopMax;
            float dtPauseMin;
            float dtPauseMax;
            short iframeStart;
            uint8_t  pad0x12;
            uint8_t  pad0x13;
            uint32_t padTail;
        }loopf;
        union PINGPONGF
        {
            float dtPingpongMin;
            float dtPingpongMax;
            float dtPauseMin;
            float dtPauseMax;
            short iframeStart;
            uint8_t  pad0x12;
            uint8_t  pad0x13;
            uint32_t padTail;
        }pingpongf;
        union SHUTTLEF
        {
            float dtPauseMin;
            float dtPauseMax;
            uint32_t pad[4];
        }shufflef;
        union HOLOGRAMF
        {
            float dradAdjust;
            uint32_t cSymmetry;
            uint32_t pad[4];
        }hologramf;
        union EYESF
        {
            float dtBlink;
            float dtOpenMin;
            float dtOpenMax;
            float uDoubleBlink;
            short oidOther;
            uint8_t pad0x12;
            uint8_t pad0x13;
            uint32_t padTail;
        }eyesf;
        union SCROLLERF
        {
            float svu;
            float svv;
            float duMod;
            float dvMod;
            uint32_t pad[2];
        }scrollerf;
        union CIRCLEF
        {
            float sw;
            float sRadius;
            float du;
            float dv;
            uint32_t pad[2];
        }circlerf;
        union LOOKERF
        {
            float uCenter;
            float vCenter;
            float uMin;
            float uMax;
            float vMin;
            float vMax;
        }lookerf;
    };
};

// Returns size and type of shader animation
void* NewSaa(SAAK saak);
// Loads shader animation from binary file
SAA* PsaaLoadFromBrx(CBinaryInputStream *pbis);
VTSAA* PvtsaaFromSaak(SAAK saak);
void  InitSaa(SAA* psaa, SAAF* psaaf);
void  PostSaaLoad(SAA* psaa);
float UCompleteSaa(SAA* psaa);
SAI*  PsaiFromSaaShd(SAA* psaa, SHD* pshd);
int   FUpdatableSaa(SAA* psaa);
void  DeleteSaa(SAA* psaa);

void  LoadLoopFromBrx(LOOP *ploop, CBinaryInputStream *pbis);
void  InitLoop(LOOP* ploop, SAAF* psaaf);
void  PostLoopLoad(LOOP* ploop);
void  UpdateLoop(LOOP* ploop, float dt);
float UCompleteLoop(LOOP* ploop);
void  DeleteLoop(LOOP* ploop);

void  LoadScrollerFromBrx(SCROLLER* pscroller, CBinaryInputStream* pbis);
void  InitScroller(SCROLLER* pscroller, SAAF* psaaf);
void  UpdateScroller(SCROLLER* pscroller, float dt);
float UCompleteScroller(SCROLLER* scroller);
void  DeleteScroller(SCROLLER* pscroller);

void  LoadCirclerFromBrx(CIRCLER* pcircler, CBinaryInputStream* pbis);
void  InitCircler(CIRCLER* pcircler, SAAF* psaaf);
void  UpdateCircler(CIRCLER* pcircler, float dt);
float UCompleteCircler(CIRCLER* pcircler);
void  DeleteCircler(CIRCLER* pcircler);

void SetSaiDuDv(SAI* psai, float du, float dv);

extern SAI* g_psaiUpdate;
extern SAI* g_psaiUpdateTail;