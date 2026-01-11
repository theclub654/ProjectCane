#pragma once

struct SAA;
struct SAAF;
struct SAI;
struct SHD;
class  CBinaryInputStream;

SAA*  PsaaLoadFromBrx(CBinaryInputStream* pbis);
void  InitSaa(SAA* psaa, SAAF* psaaf);
void  PostSaaLoad(SAA* psaa);
float UCompleteSaa(SAA* psaa);
SAI*  PsaiFromSaaShd(SAA* psaa, SHD* pshd);
int   FUpdatableSaa(SAA* psaa);
void  DeleteSaa(SAA*);

struct LOOP;
void  LoadLoopFromBrx(LOOP* ploop, CBinaryInputStream* pbis);
void  InitLoop(LOOP* ploop, SAAF* psaaf);
void  PostLoopLoad(LOOP* ploop);
void  UpdateLoop(LOOP* ploop, float dt);
float UCompleteLoop(LOOP* ploop);
void  DeleteLoop(LOOP* ploop);

struct SCROLLER;
void  LoadScrollerFromBrx(SCROLLER* pscroller, CBinaryInputStream* pbis);
void  InitScroller(SCROLLER* pscroller, SAAF* psaaf);
void  UpdateScroller(SCROLLER* pscroller, float dt);
float UCompleteScroller(SCROLLER* scroller);
void  DeleteScroller(SCROLLER*);

struct CIRCLER;
void  LoadCirclerFromBrx(CIRCLER* pcircler, CBinaryInputStream* pbis);
void  InitCircler(CIRCLER* pcircler, SAAF* psaaf);
void  UpdateCircler(CIRCLER* pcircler, float dt);
float UCompleteCircler(CIRCLER* pcircler);
void  DeleteCircler(CIRCLER* pcircler);

struct VTSAA
{
    void(*pfnLoadSaaFromBrx) = nullptr;
    void(*pfnInitSaa)(SAA*, SAAF*) = InitSaa;
    void(*pfnPostSaaLoad)(SAA*) = PostSaaLoad;
    void(*pfnUpdateSaa) = nullptr;
    float(*pfnUCompleteSaa)(SAA*) = UCompleteSaa;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteSaa)(SAA*) = DeleteSaa;
};

struct VTLOOP
{
    void(*pfnLoadLoopFromBrx)(LOOP*, CBinaryInputStream*) = LoadLoopFromBrx;
    void(*pfnInitLoop)(LOOP*, SAAF*) = InitLoop;
    void(*pfnLoopSaaLoad)(LOOP*) = PostLoopLoad;
    void(*pfnUpdateLoop)(LOOP*, float) = UpdateLoop;
    float(*pfnUCompleteLoop)(LOOP*) = UCompleteLoop;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteLoop)(LOOP*) = DeleteLoop;
};
static VTLOOP g_vtloop;

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
    void(*pfnLoadScrollerFromBrx)(SCROLLER*, CBinaryInputStream*) = LoadScrollerFromBrx;
    void(*pfnInitScroller)(SCROLLER*, SAAF*) = InitScroller;
    void(*pfnPostSaaLoad)(SAA*) = PostSaaLoad;
    void(*pfnUpdateScroller)(SCROLLER*, float) = UpdateScroller;
    float(*pfnUCompleteScroller)(SCROLLER*) = UCompleteScroller;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteScroller)(SCROLLER*) = DeleteScroller;
};
static VTSCROLLER g_vtscroller;

struct VTCIRCLER
{
    void(*pfnLoadCirclerFromBrx)(CIRCLER*, CBinaryInputStream*) = LoadCirclerFromBrx;
    void(*pfnInitCircler)(CIRCLER*, SAAF*) = InitCircler;
    void(*pfnPostSaaLoad)(SAA*) = PostSaaLoad;
    void(*pfnUpdateCircler)(CIRCLER*, float) = UpdateCircler;
    float(*pfnUCompleteCircler)(CIRCLER*) = UCompleteCircler;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteScroller)(CIRCLER*) = DeleteCircler;
};
static VTCIRCLER g_vtcircler;

struct VTLOOKER
{

};

struct VTEYES
{

};