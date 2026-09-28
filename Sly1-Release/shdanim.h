#pragma once
#include "shd.h"
#include "clock.h"

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
