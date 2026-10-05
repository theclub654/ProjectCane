#pragma once
#include "shd.h"
#include "clock.h"

struct SAAF
{
    uint16_t oid;
    short fInstanced;
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
    };
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
    };
    union SHUTTLEF
    {
            float dtPauseMin;
            float dtPauseMax;
            uint32_t pad[4];
    };
    union HOLOGRAMF
    {
            float dradAdjust;
            uint32_t cSymmetry;
            uint32_t pad[4];
    };
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
    };
    union SCROLLERF
    {
            float svu;
            float svv;
            float duMod;
            float dvMod;
            uint32_t pad[2];
    };
    union CIRCLEF
    {
            float sw;
            float sRadius;
            float du;
            float dv;
            uint32_t pad[2];
    };
    union LOOKERF
    {
            float uCenter;
            float vCenter;
            float uMin;
            float uMax;
            float vMin;
            float vMax;
    };
    union
    {
        LOOPF loopf;
        PINGPONGF pingpongf;
        SHUTTLEF shufflef;
        HOLOGRAMF hologramf;
        EYESF eyesf;
        SCROLLERF scrollerf;
        CIRCLEF circlerf;
        LOOKERF lookerf;
    };
};

// Returns size and type of shader animation
std::shared_ptr<SAA> NewSaa(SAAK saak);
// Loads shader animation from binary file
std::shared_ptr<SAA> PsaaLoadFromBrx(CBinaryInputStream *pbis);
VTSAA* PvtsaaFromSaak(SAAK saak);
void  InitSaa(SAA* psaa, SAAF* psaaf);
void  PostSaaLoad(SAA* psaa);
float UCompleteSaa(SAA* psaa);
SAI*  PsaiFromSaaShd(SAA* psaa, SHD* pshd);
int   FUpdatableSaa(SAA* psaa);
void  SetSaiDuDv(SAI* psai, float du, float dv);
void  DeleteSaa(SAA* psaa);

extern SAI* g_psaiUpdate;
extern SAI* g_psaiUpdateTail;
