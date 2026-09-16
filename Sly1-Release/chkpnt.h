#pragma once
#include "xform.h"
#include "tn.h"
#include <array>

struct CBitArray4096
{
    int m_cbitSet;
    std::array<uint8_t, 512> m_ab;
};

struct CS
{
    CBitArray4096 abitChk;
    OID oidWarp;
    OID oidWarpContext;
};

struct VTCHKMGR
{

};

struct CHKMGR
{
    VTCHKMGR* pvtchkmgr;
    int cbitChk;
    CBitArray4096 abitChk;
    bool fChkDirty;
    glm::vec3 posVolChkpnt;
    CS csSaved;
    bool fCheckpoint;
};

class VOL : public XFM
{
	public:
    int ctsurf;
    std::vector <TSURF> atsurf;
    int ctbsp;
    std::vector <TBSP> atbsp;
};

class CHKPNT : public SO
{
    public:
    int coidAseg;
    short aoidAseg[4];
    int cpaseg;
    struct ASEG* apaseg[4];
    int coidVol;
    short aoidVol[4];
    int cpvol;
    struct VOL* apvol[4];
    short oidWarp;
    int ichkTriggered;
    int ibitChkpnt;
    class WARP *pwarp;
    class SM* psm;
    class SMA* psma;
};

CHKPNT* NewChkpnt();
void InitChkpnt(CHKPNT* pchkpnt);
void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase);
int  GetChkpntSize();
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void BindChkpnt(CHKPNT* pchkpnt);
void PostChkpntLoad(CHKPNT* pchkpnt);
void UpdateChkpnt(CHKPNT* pchkpnt, float dt);
int  FAbsorbChkpntWkr(CHKPNT* pchkpnt, WKR* pwkr);
void TriggerChkpnt(CHKPNT* pchkpnt);
void AddChkpntVol(CHKPNT* pchkpnt, OID oidVol);
void AddChkpntAseg(CHKPNT* pchkpnt, OID oidAseg);
void SetChkpntWarp(CHKPNT* pchkpnt, OID oidWarp);
void DeleteChkpnt(CHKPNT *pchkpnt);

void ResetChkmgrCheckPoints(CHKMGR* pchkmgr);
void SaveChkmgrCheckpoint(CHKMGR* pchkmgr, OID oidWarp, OID oidWarpContext);
void ReturnChkmgrToCheckpoint(CHKMGR* pchkmgr);
void RestoreChkmgrFromCheckpoint(CHKMGR* pchkmgr);
int  IchkAllocChkmgr(CHKMGR* pchkmgr);
int  FGetChkmgrIchk(CHKMGR* pchkmgr, int ichk);
void SetChkmgrIchk(CHKMGR* pchkmgr, int ichk);
void ClearChkmgrIchk(CHKMGR* pchkmgr, int ichk);

VOL* NewVol();
int  GetVolSize();
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);
int  FCheckVolPoint(VOL* pvol, glm::vec3* ppos);
void CloneVol(VOL* pvol, VOL* pvolBase);
void DeleteVol(VOL* pvol);

extern CHKMGR g_chkmgr;
extern SNIP asnipChkpnt;
