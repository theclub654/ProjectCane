#pragma once

struct SAA;
struct SAAF;
struct SAI;
struct SHD;
class  CBinaryInputStream;

std::shared_ptr<SAA> PsaaLoadFromBrx(CBinaryInputStream* pbis);
void  InitSaa(SAA* psaa, SAAF* psaaf);
void  PostSaaLoad(SAA* psaa);
float UCompleteSaa(SAA* psaa);
SAI*  PsaiFromSaaShd(SAA* psaa, SHD* pshd);
int   FUpdatableSaa(SAA* psaa);
void  DeleteSaa(SAA*);

struct LOOP;
std::shared_ptr<LOOP> NewLoop();
void  LoadLoopFromBrx(LOOP* ploop, CBinaryInputStream* pbis);
void  InitLoop(LOOP* ploop, SAAF* psaaf);
void  PostLoopLoad(LOOP* ploop);
void  UpdateLoop(LOOP* ploop, float dt);
float UCompleteLoop(LOOP* ploop);
void  DeleteLoop(LOOP* ploop);

struct PINGPONG;
std::shared_ptr<PINGPONG> NewPingPong();
void  LoadPingPongFromBrx(PINGPONG* ppingpong, CBinaryInputStream* pbis);
void  InitPingpong(PINGPONG* ppingpong, SAAF* psaaf);
void  PostPingpongLoad(PINGPONG* ppingpong);
void  UpdatePingpong(PINGPONG* ppingpong, float dt);
float UCompletePingpong(PINGPONG* ppingpong);
void  DeletePingpong(PINGPONG* ppingpong);

struct SHUFFLE;
std::shared_ptr<SHUFFLE> NewShuffle();
void  LoadShuffleFromBrx(SHUFFLE* pshuffle, CBinaryInputStream* pbis);
void  InitShuffle(SHUFFLE* pshuffle, SAAF* psaaf);
void  UpdateShuffle(SHUFFLE* pshuffle, float dt);
void  DeleteShuffle(SHUFFLE* pshuffle);

struct HOLOGRAM;
std::shared_ptr<HOLOGRAM> NewHologram();
void  LoadHologramFromBrx(HOLOGRAM* phologram, CBinaryInputStream* pbis);
void  InitHologram(HOLOGRAM* phologram, SAAF* psaaf);
void  PostHologramLoad(HOLOGRAM* phologram);
void  NotifyHologramRender(HOLOGRAM* phologram, ALO* palo, RPL* prpl);
void  DeleteHologram(HOLOGRAM* phologram);

struct EYES;
std::shared_ptr<EYES> NewEyes();
void  LoadEyesFromBrx(EYES* peyes, CBinaryInputStream* pbis);
void  InitEyes(EYES* peyes, SAAF* psaaf);
void  PostEyesLoad(EYES* peyes);
void  UpdateEyes(EYES* peyes, float dt);
void  SetEyesClosed(EYES* peyes, float uClosed);
SAI*  PsaiFromEyesShd(EYES* peyes, SHD* pshd);
void  DeleteEyes(EYES* peyes);

struct SCROLLER;
std::shared_ptr<SCROLLER> NewScroller();
void  LoadScrollerFromBrx(SCROLLER* pscroller, CBinaryInputStream* pbis);
void  SetScrollerMasterSpeeds(SCROLLER* pscroller, float svu, float svv);
void  InitScroller(SCROLLER* pscroller, SAAF* psaaf);
void  UpdateScroller(SCROLLER* pscroller, float dt);
float UCompleteScroller(SCROLLER* pscroller);
void  DeleteScroller(SCROLLER*);

struct CIRCLER;
std::shared_ptr<CIRCLER> NewCircler();
void  LoadCirclerFromBrx(CIRCLER* pcircler, CBinaryInputStream* pbis);
void  InitCircler(CIRCLER* pcircler, SAAF* psaaf);
void  UpdateCircler(CIRCLER* pcircler, float dt);
float UCompleteCircler(CIRCLER* pcircler);
void  DeleteCircler(CIRCLER* pcircler);

struct LOOKER;
std::shared_ptr<LOOKER> NewLooker();
void  LoadLookerFromBrx(LOOKER* plooker, CBinaryInputStream* pbis);
void  InitLooker(LOOKER* plooker, SAAF* psaaf);
void  NotifyLookerRender(LOOKER* plooker, ALO* palo, RPL* prpl);
void  DeleteLooker(LOOKER* plooker);

struct VTSAA
{
    std::shared_ptr<SAA>(*pfnNewSaa)() = nullptr;
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
    std::shared_ptr<LOOP>(*pfnNewLoop)() = NewLoop;
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
    std::shared_ptr<PINGPONG>(*pfnNewPingPong)() = NewPingPong;
    void(*pfnLoadPingPongFromBrx)(PINGPONG*, CBinaryInputStream*) = LoadPingPongFromBrx;
    void(*pfnInitPingPong)(PINGPONG*, SAAF*) = InitPingpong;
    void(*pfnPostPingPongLoad)(PINGPONG*) = PostPingpongLoad;
    void(*pfnUpdatePingPong)(PINGPONG*, float) = UpdatePingpong;
    float(*pfnUCompletePingpong)(PINGPONG*) = UCompletePingpong;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeletePingPong)(PINGPONG*) = DeletePingpong;
};
static VTPINGPONG g_vtpingpong;

struct VTSHUFFLE
{
    std::shared_ptr<SHUFFLE>(*pfnNewShuffle)() = NewShuffle;
    void(*pfnLoadShuffleFromBrx)(SHUFFLE*, CBinaryInputStream*) = LoadShuffleFromBrx;
    void(*pfnInitShuffle)(SHUFFLE*, SAAF*) = InitShuffle;
    void(*pfnPostSaaLoad)(SAA*) = PostSaaLoad;
    void(*pfnUpdateShuffle)(SHUFFLE*, float) = UpdateShuffle;
    float(*UCompleteSaa)(SAA*) = UCompleteSaa;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteShuffle)(SHUFFLE*) = DeleteShuffle;
};
static VTSHUFFLE g_vtshuffle;

struct VTHOLOGRAM
{
    std::shared_ptr<HOLOGRAM>(*pfnNewHologram)() = NewHologram;
    void(*pfnLoadHologramFromBrx)(HOLOGRAM*, CBinaryInputStream*) = LoadHologramFromBrx;
    void(*pfnInitHologram)(HOLOGRAM*, SAAF*) = InitHologram;
    void(*pfnPostHologramLoad)(HOLOGRAM*) = PostHologramLoad;
    void(*pfnUpdateSaa) = nullptr;
    float(*pfnUCompleteSaa)(SAA*) = UCompleteSaa;
    void(*pfnNotifyHologramRender)(HOLOGRAM*, ALO*, RPL*) = NotifyHologramRender;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteHologram)(HOLOGRAM*) = DeleteHologram;
};
static VTHOLOGRAM g_vthologram;

struct VTSCROLLER
{
    std::shared_ptr<SCROLLER>(*pfnNewScroller)() = NewScroller;
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
    std::shared_ptr<CIRCLER>(*pfnNewCircler)() = NewCircler;
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
    std::shared_ptr<LOOKER>(*pfnNewLooker)() = NewLooker;
    void(*pfnLoadLookerFromBrx)(LOOKER*, CBinaryInputStream*) = LoadLookerFromBrx;
    void(*pfnInitLooker)(LOOKER*, SAAF*) = InitLooker;
    void(*pfnPostSaaLoad)(SAA*) = PostSaaLoad;
    void(*pfnUpdateSaa) = nullptr;
    float(*pfnUCompleteSaa)(SAA*) = UCompleteSaa;
    void(*pfnNotifyLookerRender)(LOOKER*, ALO*, RPL*) = NotifyLookerRender;
    SAI*(*pfnPsaiFromSaaShd)(SAA*, SHD*) = PsaiFromSaaShd;
    void(*pfnSetLookerSgvr) = nullptr;
    void(*pfnDeleteScroller)(LOOKER*) = DeleteLooker;
};
static VTLOOKER g_vtlooker;

struct VTEYES
{
    std::shared_ptr<EYES>(*pfnNewEyes)() = NewEyes;
    void(*pfnLoadEyesFromBrx)(EYES*, CBinaryInputStream*) = LoadEyesFromBrx;
    void(*pfnInitEyes)(EYES*, SAAF*) = InitEyes;
    void(*pfnPostEyesLoad)(EYES*) = PostEyesLoad;
    void(*pfnUpdateEyes)(EYES*, float) = UpdateEyes;
    float(*pfnUCompleteSaa)(SAA*) = UCompleteSaa;
    void(*pfnNotifySaaRender) = nullptr;
    SAI*(*pfnPsaiFromEyesShd)(EYES*, SHD*) = PsaiFromEyesShd;
    void(*pfnSetSaaSgvr) = nullptr;
    void(*pfnDeleteScroller)(EYES*) = DeleteEyes;
};
static VTEYES g_vteyes;
