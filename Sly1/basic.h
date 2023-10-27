#pragma once
#include <unordered_map>
#include "spaprops.h"
#include "sidebag.h"
#include "dec.h"

struct BASIC
{
    union
    {
        struct VTBASIC *pvtbasic;
        struct VTLO *pvtlo;
        struct VTALO *pvtalo;
        struct VTSO *pvtso;
        struct VTSHAPE *pvtshape;
        struct VTPNT *pvtpnt;
        struct VTPNTVEC *pvtpntvec;
        struct VTPNTSV *pvtpntsv;
        struct VTPNTS *pvtpnts;
        struct VTSCAN *pvtscan;
        struct VTXFM *pvtxfm;
        struct VTWARP *pvtwarp;
        struct VTEXIT *pvtexit;
        struct VTCAMERA *pvtcamera;
        struct VTLIGHT *pvtlight;
        struct VTPO *pvtpo;
        struct VTKEYHOLE *pvtkeyhole;
        struct VTWATER *pvtwater;
        struct VTTN *pvttn;
        struct VTBRK *pvtbrk;
        struct VTBRKP *pvtbrkp;
        struct VTBREAK *pvtbreak;
        struct VTFRAGILE *pvtfragile;
        struct VTZAPBREAK *pvtzapbreak;
        struct VTVOLBTN *pvtvolbtn;
        struct VTBUTTON *pvtbutton;
        struct VTSQUISH *pvtsquish;
        struct VTFRZG *pvtfrzg;
        struct VTBONE *pvtbone;
        struct VTLBONE *pvtlbone;
        struct VTMAP *pvtmap;
        struct VTVISMAP *pvtvismap;
        struct VT_VISZONE *pvt_viszone;
        struct VTASEG *pvtaseg;
        struct VTASEGA *pvtasega;
        struct VTASEGBL *pvtasegbl;
        struct VTSM *pvtsm;
        struct VTSMA *pvtsma;
        struct VTIKH *pvtikh;
        struct VTLIKH *pvtlikh;
        struct VTSCH *pvtsch;
        struct VTRIPG *pvtripg;
        struct VTBLIPG *pvtblipg;
        struct VTEMITTER *pvtemitter;
        struct VTEXPL *pvtexpl;
        struct VTEXPLG *pvtexplg;
        struct VTEXPLO *pvtexplo;
        struct VTEXPLS *pvtexpls;
        struct VTPROXY *pvtproxy;
        struct VTDIALOG *pvtdialog;
        struct VTHSHAPE *pvthshape;
        struct VTHPNT *pvthpnt;
        struct VTHBSK *pvthbsk;
        struct VTRCHM *pvtrchm;
        struct VTSTEP *pvtstep;
        struct VTSPRIZE *pvtsprize;
        struct VTSCPRIZE *pvtscprize;
        struct VTCLUE *pvtclue;
        struct VTLIFETKN *pvtlifetkn;
        struct VTLOCK *pvtlock;
        struct VTLOCKG *pvtlockg;
        struct VTVAULT *pvtvault;
        struct VTALBRK *pvtalbrk;
        struct VTALARM *pvtalarm;
        struct VTSENSOR *pvtsensor;
        struct VTLASEN *pvtlasen;
        struct VTCAMSEN *pvtcamsen;
        struct VTPRSEN *pvtprsen;
        struct VTTARGET *pvttarget;
        struct VTHND *pvthnd;
        struct VTPIPE *pvtpipe;
        struct VTBARRIER *pvtbarrier;
        struct VTTAIL *pvttail;
        struct VTJSG *pvtjsg;
        struct VTJT *pvtjt;
        struct VTJMT *pvtjmt;
        struct VTSGG *pvtsgg;
        struct VTSTEPGUARD *pvtstepguard;
        struct VTGOMER *pvtgomer;
        struct VTSMARTGUARD *pvtsmartguard;
        struct VTTZP *pvttzp;
        struct VTVOLZP *pvtvolzp;
        struct VTVOL *pvtvol;
        struct VTCHKPNT *pvtchkpnt;
        struct VTPATHZONE *pvtpathzone;
        struct VTBOMB *pvtbomb;
        struct VTCAN *pvtcan;
        struct VTSKY *pvtsky;
        struct VTFLY *pvtfly;
        struct VTDPRIZE *pvtdprize;
        struct VTCOIN *pvtcoin;
        struct VTCHARM *pvtcharm;
        struct VTKEY *pvtkey;
        struct VTGOLD *pvtgold;
        struct VTRATHOLE *pvtrathole;
        struct VTRAT *pvtrat;
        struct VTROB *pvtrob;
        struct VTROV *pvtrov;
        struct VTROH *pvtroh;
        struct VTROC *pvtroc;
        struct VTROST *pvtrost;
        struct VTROP *pvtrop;
        struct VTDART *pvtdart;
        struct VTDARTGUN *pvtdartgun;
        struct VTFLASH *pvtflash;
        struct VTMISSILE *pvtmissile;
        struct VTACCMISS *pvtaccmiss;
        struct VTTARMISS *pvttarmiss;
        struct VTSPLMISS *pvtsplmiss;
        struct VTGROUNDMISS *pvtgroundmiss;
        struct VTRWM *pvtrwm;
        struct VTCNVO *pvtcnvo;
        struct VTDYSH *pvtdysh;
        struct VTDSP *pvtdsp;
        struct VTWAYPOINT *pvtwaypoint;
        struct VTUBV *pvtubv;
        struct VTUBP *pvtubp;
        struct VTUBG *pvtubg;
        struct VTMBG *pvtmbg;
        struct VTSCENTMAP *pvtscentmap;
        struct VTBHG *pvtbhg;
        struct VTJLOC *pvtjloc;
        struct VTJLOVOL *pvtjlovol;
        struct VTJLO *pvtjlo;
        struct VTSPIRE *pvtspire;
        struct VTRAIL *pvtrail;
        struct VTLANDING *pvtlanding;
        struct VTROPE *pvtrope;
        struct VTWR *pvtwr;
        struct VTTANK *pvttank;
        struct VTTURRET *pvtturret;
        struct VTMURRAY *pvtmurray;
        struct VTPUFFER *pvtpuffer;
        struct VTPUFFC *pvtpuffc;
        struct VTPUFFB *pvtpuffb;
        struct VTPUFFT *pvtpufft;
        struct VTPUFFV *pvtpuffv;
        struct VTMRKV *pvtmrkv;
        struct VTCRFOD *pvtcrfod;
        struct VTCRFODB *pvtcrfodb;
        struct VTCRFODK *pvtcrfodk;
        struct VTCRBRAIN *pvtcrbrain;
        struct VTMGV *pvtmgv;
        struct VTMGC *pvtmgc;
        struct VTSUV *pvtsuv;
        struct VTJP *pvtjp;
        struct VTCYCLE *pvtcycle;
        struct VTHG *pvthg;
        struct VTMECHA *pvtmecha;
        struct VTSWP *pvtswp;
        struct VTLGN *pvtlgn;
        struct VTLGNB *pvtlgnb;
        struct VTJACK *pvtjack;
        struct VTJACKB *pvtjackb;
        struct VTJACKN *pvtjackn;
        struct VTJACKF *pvtjackf;
        struct VTMS *pvtms;
        struct VTSPEAKER *pvtspeaker;
        struct VTWM *pvtwm;
        struct VTSW *pvtsw;
        struct VTCM *pvtcm;
    };
    class CSidebag *psidebag;
};

struct VT 
{
    VT *pvtSuper;
    CID cid;
    int grfcid;
    int cb;
};

struct VTBASIC
{
    VT *pvtSuper = nullptr;
    CID cid = CID_BASIC;
};

static VTBASIC g_vtbasic;

struct VTLO 
{
    VT *pvtSuper = g_vtbasic.pvtSuper;
    CID cid = CID_LO;
    int grfcid = 0;
    int cb = 0x40;

    void (*pfnInitLo) (LO*) = InitLo;
    void (*pfnSetLoDefaults) (LO*) = SetLoDefaults;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)(LO*) = RemoveLo;
    void (*pfnAddLoHierarchy)(LO*) = AddLoHierarchy;
    void (*pfnRemoveLoHierarchy)(LO*) = RemoveLoHierarchy;
    void (*pfnOnLoAdd)(LO*) = OnLoAdd;
    void (*pfnOnLoRemove)(LO*) = OnLoRemove;
    void (*pfnCloneLoHierarchy)(LO*, LO*) = CloneLoHierarchy;
    void (*pfnCloneLo)(LO*, LO*) = CloneLo;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)(LO*, MSGID, void*) = SendLoMessage;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)(LO*) = PostLoLoad;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)(LO*, ALO*) = SetLoParent;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)(LO*, LO*) = SubscribeLoObject;
    void (*pfnUnsubscribeLoObject)(LO*, LO*) = UnsubscribeLoObject;
    void (*pfnSubscribeLoStruct)(LO*, void*, void*) = SubscribeLoStruct;
    void (*pfnUnsubscribeLoStruct)(LO*, void*, void*) = UnsubscribeLoStruct;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLO g_vtlo;

struct VTALO
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_ALO;
    int grfcid = 1;
    int cb = 0x2E0;

    void (*pfnInitLo) (ALO*) = InitAlo;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)(LO*) = RemoveLo;
    void (*pfnAddLoHierarchy)(ALO*) = AddAloHierarchy;
    void (*pfnRemoveLoHierarchy)(ALO*) = RemoveAloHierarchy;
    void (*pfnOnLoAdd)(ALO*) = OnAloAdd;
    void (*pfnOnLoRemove)(ALO*) = OnAloRemove;
    void (*pfnCloneLoHierarchy)(ALO*) = CloneAloHierarchy;
    void (*pfnCloneLo)(ALO*) = CloneAlo;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo)(ALO*, float) = UpdateAlo;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTALO g_vtalo;

struct VTSO
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_SO;
    int grfcid = 3;
    int cb = 0x580;

    void (*pfnInitLo) (SO*) = InitSo;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)(LO*) = RemoveLo;
    void (*pfnAddLoHierarchy)(ALO*) = AddAloHierarchy;
    void (*pfnRemoveLoHierarchy)(ALO*) = RemoveAloHierarchy;
    void (*pfnOnLoAdd) (SO*) = OnSoAdd;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo)(SO*, float) = UpdateSo;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSO g_vtso;

struct VTMS
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_MS;
    int grfcid = 3;
    int cb = 0x580;

    void (*pfnInitLo) (SO*) = InitSo;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)(LO*) = RemoveLo;
    void (*pfnAddLoHierarchy)(ALO*) = AddAloHierarchy;
    void (*pfnRemoveLoHierarchy)(ALO*) = RemoveAloHierarchy;
    void (*pfnOnLoAdd) (SO*) = OnSoAdd;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo)(SO*, float) = UpdateSo;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMS g_vtms;

struct VTPO
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_PO;
    int grfcid = 0xB;
    int cb = 0x5D0;

    void (*pfnInitLo) (PO*) = InitPo;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPO g_vtpo;

struct VTSTEP
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_STEP;
    int grfcid = 0xB;
    int cb = 0x6E0;

    void (*pfnInitLo) (STEP*) = InitStep;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSTEP g_vtstep;

struct VTJT
{
    VT *pvtSuper = g_vtstep.pvtSuper;
    CID cid = CID_JT;
    int grfcid = 0xB;
    int cb = 0x2730;

    void (*pfnInitLo) (JT*) = InitJt;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJT g_vtjt;

struct VTSTEPGUARD
{
    VT *pvtSuper = g_vtstep.pvtSuper;
    CID cid = CID_STEPGUARD;
    int grfcid = 0xB;
    int cb = 0xB90;

    void (*pfnInitLo) (STEPGUARD*) = InitStepGuard;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSTEPGUARD g_vtstepguard;

struct VTSMARTGUARD
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_SMARTGUARD;
    int grfcid = 0xB;
    int cb = 0xC50;

    void (*pfnInitLo) (SMARTGUARD*) = InitSmartGuard;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSMARTGUARD g_vtsmartguard;

struct VTGOMER
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_GOMER;
    int grfcid = 0xB;
    int cb = 0xBD0;

    void (*pfnInitLo) (GOMER*) = InitGomer;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTGOMER g_vtgomer;

struct VTUBG
{
    VT *pvtSuper = g_vtgomer.pvtSuper;
    CID cid = CID_UBG;
    int grfcid = 0xB;
    int cb = 0xC20;

    void (*pfnInitLo) (UBG*) = InitUbg;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTUBG g_vtubg;

struct VTMBG
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_MBG;
    int grfcid = 0xB;
    int cb = 0xDC0;

    void (*pfnInitLo) (MBG*) = InitMbg;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MBG*, CBinaryInputStream*) = LoadMbgFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMBG g_vtmbg;

struct VTBHG
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_BHG;
    int grfcid = 0xB;
    int cb = 0xBA0;

    void (*pfnInitLo) (BHG*) = InitBhg;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBHG g_vtbhg;

struct VTMURRAY
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_MURRAY;
    int grfcid = 0xB;
    int cb = 0xBC0;

    void (*pfnInitLo) (MURRAY*) = InitMurray;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMURRAY g_vtmurray;

struct VTPUFFC
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_PUFFC;
    int grfcid = 0xB;
    int cb = 0xBC0;

    void (*pfnInitLo) (STEPGUARD*) = InitStepGuard;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPUFFC g_vtpuffc;

struct VTCRFOD
{
    VT *pvtSuper = g_vtstepguard.pvtSuper;
    CID cid = CID_CRFOD;
    int grfcid = 0xB;
    int cb = 0xBA0;

    void (*pfnInitLo) (STEPGUARD*) = InitStepGuard;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCRFOD g_vtcrfod;

struct VTCRFODB
{
    VT *pvtSuper = g_vtcrfod.pvtSuper;
    CID cid = CID_CRFODB;
    int grfcid = 0xB;
    int cb = 0xBB0;

    void (*pfnInitLo) (CRFODB*) = InitCrfodb;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCRFODB g_vtcrfodb;

struct VTCRFODK
{
    VT *pvtSuper = g_vtcrfod.pvtSuper;
    CID cid = CID_CRFODK;
    int grfcid = 0xB;
    int cb = 0xBA0;

    void (*pfnInitLo) (STEPGUARD*) = InitStepGuard;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(STEPGUARD*, CBinaryInputStream*) = LoadStepGuardFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCRFODK g_vtcrfodk;

struct VTTANK
{
    VT *pvtSuper = g_vtstep.pvtSuper;
    CID cid = CID_TANK;
    int grfcid = 0xB;
    int cb = 0x730;

    void (*pfnInitLo) (TANK*) = InitTank;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTANK g_vttank;

struct VTJP
{
    VT *pvtSuper = g_vtstep.pvtSuper;
    CID cid = CID_JP;
    int grfcid = 0xB;
    int cb = 0x950;

    void (*pfnInitLo) (JP*) = InitJp;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJP g_vtjp;

struct VTHG
{
    VT *pvtSuper = g_vtstep.pvtSuper;
    CID cid = CID_HG;
    int grfcid = 0xB;
    int cb = 0x900;

    void (*pfnInitLo) (HG*) = InitHg;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTHG g_vthg;

struct VTMECHA
{
    VT *pvtSuper = g_vtstep.pvtSuper;
    CID cid = CID_MECHA;
    int grfcid = 0xB;
    int cb = 0x970;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMECHA g_vtmecha;

struct VTROV
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_ROV;
    int grfcid = 0xB;
    int cb = 0x650;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ROV*, CBinaryInputStream*) = LoadRovFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROV g_vtrov;

struct VTTURRET
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_TURRET;
    int grfcid = 0xB;
    int cb = 0x5F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTURRET g_vtturret;

struct VTVAULT
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_VAULT;
    int grfcid = 0xB;
    int cb = 0x7A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTVAULT g_vtvault;

struct VTPUFFER
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_PUFFER;
    int grfcid = 0xB;
    int cb = 0x660;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PUFFER*, CBinaryInputStream*) = LoadPufferFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPUFFER g_vtpuffer;

struct VTMGV
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_MGV;
    int grfcid = 0xB;
    int cb = 0x620;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MGV*, CBinaryInputStream*) = LoadMgvFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMGV g_vtmgv;

struct VTSUV
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_SUV;
    int grfcid = 0xB;
    int cb = 0xBE0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSUV g_vtsuv;

struct VTCYCLE
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_CYCLE;
    int grfcid = 0xB;
    int cb = 0x6E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCYCLE g_vtcycle;

struct VTLGN
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_LGN;
    int grfcid = 0xB;
    int cb = 0x630;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLGN g_vtlgn;

struct VTJACK
{
    VT *pvtSuper = g_vtpo.pvtSuper;
    CID cid = CID_JACK;
    int grfcid = 0xB;
    int cb = 0x750;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJACK g_vtjack;

struct VTRIPG
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_RIPG;
    int grfcid = 3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTRIPG g_vtripg;

struct VTWATER
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_WATER;
    int grfcid = 0x83;
    int cb = 0x6D0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTWATER g_vtwater;

struct VTBRK
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_BRK;
    int grfcid = 0x3;
    int cb = 0x6E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBRK g_vtbrk;

struct VTBREAK
{
    VT *pvtSuper = g_vtbrk.pvtSuper;
    CID cid = CID_BREAK;
    int grfcid = 0x13;
    int cb = 0x6E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBREAK g_vtbreak;

struct VTALBRK
{
    VT *pvtSuper = g_vtbreak.pvtSuper;
    CID cid = CID_ALBRK;
    int grfcid = 0x13;
    int cb = 0x6F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTALBRK g_vtalbrk;

struct VTCAN
{
    VT *pvtSuper = g_vtbreak.pvtSuper;
    CID cid = CID_CAN;
    int grfcid = 0x13;
    int cb = 0x770;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCAN g_vtcan;

struct VTDARTGUN
{
    VT *pvtSuper = g_vtbreak.pvtSuper;
    CID cid = CID_DARTGUN;
    int grfcid = 0x13;
    int cb = 0x7E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTDARTGUN g_vtdartgun;

struct VTSWP
{
    VT *pvtSuper = g_vtbreak.pvtSuper;
    CID cid = CID_SWP;
    int grfcid = 0x13;
    int cb = 0x710;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSWP g_vtswp;

struct VTFRAGILE
{
    VT *pvtSuper = g_vtbrk.pvtSuper;
    CID cid = CID_FRAGILE;
    int grfcid = 0x3;
    int cb = 0x6F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTFRAGILE g_vtfragile;

struct VTZAPBREAK
{
    VT *pvtSuper = g_vtfragile.pvtSuper;
    CID cid = CID_ZAPBREAK;
    int grfcid = 0x3;
    int cb = 0x700;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BRK*, CBinaryInputStream*) = LoadBrkFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTZAPBREAK g_vtzapbreak;

struct VTBRKP
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_BRKP;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBRKP g_vtbrkp;

struct VTBUTTON
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_BUTTON;
    int grfcid = 0x3;
    int cb = 0x8C0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BUTTON*, CBinaryInputStream*) = LoadButtonFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBUTTON g_vtbutton;

struct VTVOLBTN
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_VOLBTN;
    int grfcid = 0x3;
    int cb = 0x7D0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(VOLBTN*, CBinaryInputStream*) = LoadVolbtnFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTVOLBTN g_vtvolbtn;

struct VTJLOVOL
{
    VT *pvtSuper = g_vtvolbtn.pvtSuper;
    CID cid = CID_JLOVOL;
    int grfcid = 0x3;
    int cb = 0x7F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(VOLBTN*, CBinaryInputStream*) = LoadVolbtnFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJLOVOL g_vtjlovol;

struct VTSQUISH
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_SQUISH;
    int grfcid = 0x3;
    int cb = 0x650;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SQUISH*, CBinaryInputStream*) = LoadSquishFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSQUISH g_vtsquish;

struct VTBONE
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_BONE;
    int grfcid = 0x3;
    int cb = 0x580;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBONE g_vtbone;

struct VTSPRIZE
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_SPRIZE;
    int grfcid = 0x3;
    int cb = 0x5D0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SPRIZE*, CBinaryInputStream*) = LoadSprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSPRIZE g_vtsprize;

struct VTSCPRIZE
{
    VT *pvtSuper = g_vtsprize.pvtSuper;
    CID cid = CID_SCPRIZE;
    int grfcid = 0x3;
    int cb = 0x5E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SPRIZE*, CBinaryInputStream*) = LoadSprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSCPRIZE g_vtscprize;

struct VTLIFETKN
{
    VT *pvtSuper = g_vtscprize.pvtSuper;
    CID cid = CID_LIFETKN;
    int grfcid = 0x3;
    int cb = 0x5E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SPRIZE*, CBinaryInputStream*) = LoadSprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLIFETKN g_vtlifetkn;

struct VTCLUE
{
    VT *pvtSuper = g_vtsprize.pvtSuper;
    CID cid = CID_CLUE;
    int grfcid = 0x3;
    int cb = 0x5F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(CLUE*, CBinaryInputStream*) = LoadClueFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCLUE g_vtclue;

struct VTALARM
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_ALARM;
    int grfcid = 0x3;
    int cb = 0x6F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTALARM g_vtalarm;

struct VTSENSOR
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_SENSOR;
    int grfcid = 0x3;
    int cb = 0x600;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSENSOR g_vtsensor;

struct VTLASEN
{
    VT *pvtSuper = g_vtsensor.pvtSuper;
    CID cid = CID_LASEN;
    int grfcid = 0x3;
    int cb = 0xB40;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LASEN*, CBinaryInputStream*) = LoadLasenFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLASEN g_vtlasen;

struct VTCAMSEN
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_CAMSEN;
    int grfcid = 0x3;
    int cb = 0x620;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCAMSEN g_vtcamsen;

struct VTPRSEN
{
    VT *pvtSuper = g_vtsensor.pvtSuper;
    CID cid = CID_PRSEN;
    int grfcid = 0x3;
    int cb = 0x650;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPRSEN g_vtprsen;

struct VTBARRIER
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_BARRIER;
    int grfcid = 0x3;
    int cb = 0x5E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBARRIER g_vtbarrier;

struct VTIKH
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_IKH;
    int grfcid = 0x3;
    int cb = 0x580;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTIKH g_vtikh;

struct VTTZP
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_TZP;
    int grfcid = 0x3;
    int cb = 0x890;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTZP g_vttzp;

struct VTVOLZP
{
    VT *pvtSuper = g_vttzp.pvtSuper;
    CID cid = CID_VOLZP;
    int grfcid = 0x3;
    int cb = 0x890;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTVOLZP g_vtvolzp;

struct VTCNVO
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_CNVO;
    int grfcid = 0x3;
    int cb = 0x590;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(CNVO*, CBinaryInputStream*) = LoadCnvoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCNVO g_vtcnvo;

struct VTHBSK
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_HBSK;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(HBSK*, CBinaryInputStream*) = LoadHbskFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTHBSK g_vthbsk;

struct VTBOMB
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_BOMB;
    int grfcid = 0x23;
    int cb = 0x670;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(BOMB*, CBinaryInputStream*) = LoadBombFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBOMB g_vtbomb;

struct VTMISSILE
{
    VT *pvtSuper = g_vtbomb.pvtSuper;
    CID cid = CID_MISSILE;
    int grfcid = 0x23;
    int cb = 0x6A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MISSILE*, CBinaryInputStream*) = LoadMissileFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMISSILE g_vtmissile;

struct VTACCMISS
{
    VT *pvtSuper = g_vtmissile.pvtSuper;
    CID cid = CID_ACCMISS;
    int grfcid = 0x23;
    int cb = 0x6C0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MISSILE*, CBinaryInputStream*) = LoadMissileFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTACCMISS g_vtaccmiss;

struct VTTARMISS
{
    VT *pvtSuper = g_vtaccmiss.pvtSuper;
    CID cid = CID_TARMISS;
    int grfcid = 0x23;
    int cb = 0x6E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MISSILE*, CBinaryInputStream*) = LoadMissileFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTARMISS g_vttarmiss;

struct VTSPLMISS
{
    VT *pvtSuper = g_vtmissile.pvtSuper;
    CID cid = CID_SPLMISS;
    int grfcid = 0x23;
    int cb = 0x6C0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MISSILE*, CBinaryInputStream*) = LoadMissileFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSPLMISS g_vtsplmiss;

struct VTGROUNDMISS
{
    VT *pvtSuper = g_vtmissile.pvtSuper;
    CID cid = CID_GROUNDMISS;
    int grfcid = 0x23;
    int cb = 0x6B0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MISSILE*, CBinaryInputStream*) = LoadMissileFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTGROUNDMISS g_vtgroundmiss;

struct VTFLY
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_FLY;
    int grfcid = 0x3;
    int cb = 0x6D0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(FLY*, CBinaryInputStream*) = LoadFlyFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTFLY g_vtfly;

struct VTRAT
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_RAT;
    int grfcid = 0x3;
    int cb = 0x6A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(RAT*, CBinaryInputStream*) = LoadRatFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTRAT g_vtrat;

struct VTROH
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_ROH;
    int grfcid = 0x3;
    int cb = 0x5F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ROH*, CBinaryInputStream*) = LoadRohFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROH g_vtroh;

struct VTROC
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_ROC;
    int grfcid = 0x3;
    int cb = 0x5E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ROC*, CBinaryInputStream*) = LoadRocFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROC g_vtroc;

struct VTROST
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_ROST;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ROST*, CBinaryInputStream*) = LoadRostFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROST g_vtrost;

struct VTROP
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_ROP;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ROP*, CBinaryInputStream*) = LoadRopFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROP g_vtrop;

struct VTDART
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_DART;
    int grfcid = 0x3;
    int cb = 0x5C0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DART*, CBinaryInputStream*) = LoadDartFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTDART g_vtdart;

struct VTUBV
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_UBV;
    int grfcid = 0x3;
    int cb = 0x730;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTUBV g_vtubv;

struct VTUBP
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_UBP;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTUBP g_vtubp;

struct VTDSP
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_DSP;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTDSP g_vtdsp;

struct VTJLO
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_JLO;
    int grfcid = 0x3;
    int cb = 0x5F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(JLO*, CBinaryInputStream*) = LoadJloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJLO g_vtjlo;

struct VTPUFFT
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_PUFFT;
    int grfcid = 0x3;
    int cb = 0x590;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPUFFT g_vtpufft;

struct VTMRKV
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_MRKV;
    int grfcid = 0x3;
    int cb = 0x5A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMRKV g_vtmrkv;

struct VTLGNB
{
    VT *pvtSuper = g_vtso.pvtSuper;
    CID cid = CID_LGNB;
    int grfcid = 0x3;
    int cb = 0x5C0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SO*, CBinaryInputStream*) = LoadSoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLGNB g_vtlgnb;

struct VTBLIPG
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_BLIPG;
    int grfcid = 0x1;
    int cb = 0x650;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTBLIPG g_vtblipg;

struct VTCAMERA
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_CAMERA;
    int grfcid = 0x1;
    int cb = 0x330;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCAMERA g_vtcamera;

struct VTLBONE
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_LBONE;
    int grfcid = 0x1;
    int cb = 0x2E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLBONE g_vtlbone;

struct VTEMITTER
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_EMITTER;
    int grfcid = 0x1;
    int cb = 0x360;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(EMITTER*, CBinaryInputStream*) = LoadEmitterFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTEMITTER g_vtemitter;

struct VTLIGHT
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_LIGHT;
    int grfcid = 0x1;
    int cb = 0x630;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLIGHT g_vtlight;

struct VTSCH
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_SCH;
    int grfcid = 0x1;
    int cb = 0x2E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSCH g_vtsch;

struct VTLIKH
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_LIKH;
    int grfcid = 0x1;
    int cb = 0x2E0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLIKH g_vtlikh;

struct VTCHKPNT
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_CHKPNT;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(CHKPNT*, CBinaryInputStream*) = LoadChkpntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCHKPNT g_vtchkpnt;

struct VTPROXY
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_PROXY;
    int grfcid = 0x101;
    int cb = 0x300;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PROXY*, CBinaryInputStream*) = LoadProxyFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPROXY g_vtproxy;

struct VTSKY
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_SKY;
    int grfcid = 0x1;
    int cb = 0x2F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSKY g_vtsky;

struct VTDPRIZE
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_DPRIZE;
    int grfcid = 0x1;
    int cb = 0x3A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DPRIZE*, CBinaryInputStream*) = LoadDprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTDPRIZE g_vtdprize;

struct VTCHARM
{
    VT *pvtSuper = g_vtdprize.pvtSuper;
    CID cid = CID_CHARM;
    int grfcid = 0x1;
    int cb = 0x3A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DPRIZE*, CBinaryInputStream*) = LoadDprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCHARM g_vtcharm;

struct VTCOIN
{
    VT *pvtSuper = g_vtdprize.pvtSuper;
    CID cid = CID_COIN;
    int grfcid = 0x1;
    int cb = 0x3B0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DPRIZE*, CBinaryInputStream*) = LoadDprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCOIN g_vtcoin;

struct VTKEY
{
    VT *pvtSuper = g_vtdprize.pvtSuper;
    CID cid = CID_KEY;
    int grfcid = 0x1;
    int cb = 0x3A0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DPRIZE*, CBinaryInputStream*) = LoadDprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTKEY g_vtkey;

struct VTGOLD
{
    VT *pvtSuper = g_vtdprize.pvtSuper;
    CID cid = CID_GOLD;
    int grfcid = 0x1;
    int cb = 0x3B0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DPRIZE*, CBinaryInputStream*) = LoadDprizeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTGOLD g_vtgold;

struct VTLOCK
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_LOCK;
    int grfcid = 0x1;
    int cb = 0x2F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LOCK*, CBinaryInputStream*) = LoadLockFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLOCK g_vtlock;

struct VTLOCKG
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_LOCKG;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LOCKG*, CBinaryInputStream*) = LoadLockgFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLOCKG g_vtlockg;

struct VTTAIL
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_TAIL;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTAIL g_vttail;

struct VTROB
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_ROB;
    int grfcid = 0x1;
    int cb = 0x670;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROB g_vtrob;

struct VTFLASH
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_FLASH;
    int grfcid = 0x1;
    int cb = 0x310;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(FLASH*, CBinaryInputStream*) = LoadFlashFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTFLASH g_vtflash;

struct VTDYSH
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_DYSH;
    int grfcid = 0x1;
    int cb = 0x300;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTDYSH g_vtdysh;

struct VTSCENTMAP
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_SCENTMAP;
    int grfcid = 0x1;
    int cb = 0x320;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSCENTMAP g_vtscentmap;

struct VTWAYPOINT
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_WAYPOINT;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTWAYPOINT g_vtwaypoint;

struct VTTN
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_TN;
    int grfcid = 0x1;
    int cb = 0x440;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(TN*, CBinaryInputStream*) = LoadTnFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTN g_vttn;

struct VTJLOC
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_JLOC;
    int grfcid = 0x1;
    int cb = 0x350;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(JLOC*, CBinaryInputStream*) = LoadJlocFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJLOC g_vtjloc;

struct VTDIALOG
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_DIALOG;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(DIALOG*, CBinaryInputStream*) = LoadDialogFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTDIALOG g_vtdialog;

struct VTSPEAKER
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_SPEAKER;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSPEAKER g_vtspeaker;

struct VTROPE
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_ROPE;
    int grfcid = 0x1;
    int cb = 0x320;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTROPE g_vtrope;

struct VTWM
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_WM;
    int grfcid = 0x1;
    int cb = 0x4B0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTWM g_vtwm;

struct VTPUFFB
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_PUFFB;
    int grfcid = 0x1;
    int cb = 0x300;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPUFFB g_vtpuffb;

struct VTCRBRAIN
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_CRBRAIN;
    int grfcid = 0x1;
    int cb = 0x2F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCRBRAIN g_vtcrbrain;

struct VTMGC
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_MGC;
    int grfcid = 0x1;
    int cb = 0x330;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(MGC*, CBinaryInputStream*) = LoadMgcFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMGC g_vtmgc;

struct VTJACKB
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_JACKB;
    int grfcid = 0x1;
    int cb = 0x350;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJACKB g_vtjackb;

struct VTJACKN
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_JACKN;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(JACKN*, CBinaryInputStream*) = LoadJacknFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJACKN g_vtjackn;

struct VTJACKF
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_JACKF;
    int grfcid = 0x1;
    int cb = 0x320;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ALO*, CBinaryInputStream*) = LoadAloFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJACKF g_vtjackf;

struct VTSW
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_SW;
    int grfcid = 0;
    int cb = 0x2420;

    void (*pfnInitLo) (SW*) = InitSw;
    void (*pfnSetLoDefaults) (LO*) = SetLoDefaults;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)(LO*) = RemoveLo;
    void (*pfnAddLoHierarchy)(LO*) = AddLoHierarchy;
    void (*pfnRemoveLoHierarchy)(LO*) = RemoveLoHierarchy;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)(LO*, LO*) = CloneLoHierarchy;
    void (*pfnCloneLo)(LO*, LO*) = CloneLo;
    void (*pfnLoadLoFromBrx)(SW*, CBinaryInputStream*) = LoadSwFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)(LO*, MSGID, void*) = SendLoMessage;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)(LO*) = PostLoLoad;
    void (*pfnUpdateLo)(SW*, float) = UpdateSw;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)(LO*, ALO*) = SetLoParent;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)(LO*, LO*) = SubscribeLoObject;
    void (*pfnUnsubscribeLoObject)(LO*, LO*) = UnsubscribeLoObject;
    void (*pfnSubscribeLoStruct)(LO*, void*, void*) = SubscribeLoStruct;
    void (*pfnUnsubscribeLoStruct)(LO*, void*, void*) = UnsubscribeLoStruct;
    void (*pfnGetLoParams)(SW*, SOP**) = GetSwParams;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSW g_vtsw;

struct VTCM
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_CM;
    int grfcid = 0;
    int cb = 0x5D0;

    void (*pfnInitLo) (CM*) = InitCm;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)(LO*) = AddLoHierarchy;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTCM g_vtcm;

struct VTSHAPE
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_SHAPE;
    int grfcid = 0;
    int cb = 0x50;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SHAPE*, CBinaryInputStream*) = LoadShapeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSHAPE g_vtshape;

struct VTHSHAPE
{
    VT *pvtSuper = g_vtshape.pvtSuper;
    CID cid = CID_HSHAPE;
    int grfcid = 0;
    int cb = 0x70;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SHAPE*, CBinaryInputStream*) = LoadShapeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTHSHAPE g_vthshape;

struct VTPIPE
{
    VT *pvtSuper = g_vtshape.pvtSuper;
    CID cid = CID_PIPE;
    int grfcid = 0;
    int cb = 0x90;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SHAPE*, CBinaryInputStream*) = LoadShapeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPIPE g_vtpipe;

struct VTRAIL
{
    VT *pvtSuper = g_vtshape.pvtSuper;
    CID cid = CID_RAIL;
    int grfcid = 0;
    int cb = 0x68;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SHAPE*, CBinaryInputStream*) = LoadShapeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTRAIL g_vtrail;

struct VTLANDING
{
    VT *pvtSuper = g_vtshape.pvtSuper;
    CID cid = CID_LANDING;
    int grfcid = 0;
    int cb = 0x58;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SHAPE*, CBinaryInputStream*) = LoadShapeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTLANDING g_vtlanding;

struct VTXFM
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_XFM;
    int grfcid = 0;
    int cb = 0x80;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(XFM*, CBinaryInputStream*) = LoadXfmFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTXFM g_vtxfm;

struct VTWARP
{
    VT *pvtSuper = g_vtxfm.pvtSuper;
    CID cid = CID_WARP;
    int grfcid = 0;
    int cb = 0x120;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(WARP*, CBinaryInputStream*) = LoadWarpFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTWARP g_vtwarp;

struct VTTARGET
{
    VT *pvtSuper = g_vtxfm.pvtSuper;
    CID cid = CID_TARGET;
    int grfcid = 0;
    int cb = 0xA;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(XFM*, CBinaryInputStream*) = LoadXfmFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTTARGET g_vttarget;

struct VTHND
{
    VT *pvtSuper = g_vttarget.pvtSuper;
    CID cid = CID_HND;
    int grfcid = 0;
    int cb = 0xC0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(HND*, CBinaryInputStream*) = LoadHndFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTHND g_vthnd;

struct VTEXPL
{
    VT *pvtSuper = g_vtxfm.pvtSuper;
    CID cid = CID_EXPL;
    int grfcid = 0;
    int cb = 0x90;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(XFM*, CBinaryInputStream*) = LoadXfmFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTEXPL g_vtexpl;

struct VTEXPLG
{
    VT *pvtSuper = g_vtexpl.pvtSuper;
    CID cid = CID_EXPLG;
    int grfcid = 0;
    int cb = 0xC0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(EXPLG*, CBinaryInputStream*) = LoadExplgFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTEXPLG g_vtexplg;

struct VTEXPLO
{
    VT *pvtSuper = g_vtexpl.pvtSuper;
    CID cid = CID_EXPLO;
    int grfcid = 0;
    int cb = 0xA0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(EXPLO*, CBinaryInputStream*) = LoadExploFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTEXPLO g_vtexplo;

struct VTEXPLS
{
    VT *pvtSuper = g_vtexplo.pvtSuper;
    CID cid = CID_EXPLS;
    int grfcid = 0;
    int cb = 0xD0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(EXPLO*, CBinaryInputStream*) = LoadExploFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTEXPLS g_vtexpls;

struct VTVOL
{
    VT *pvtSuper = g_vtxfm.pvtSuper;
    CID cid = CID_VOL;
    int grfcid = 0;
    int cb = 0x90;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(VOL*, CBinaryInputStream*) = LoadVolFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTVOL g_vtvol;

struct VTRATHOLE
{
    VT *pvtSuper = g_vtvol.pvtSuper;
    CID cid = CID_RATHOLE;
    int grfcid = 0;
    int cb = 0xA0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(VOL*, CBinaryInputStream*) = LoadVolFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTRATHOLE g_vtrathole;

struct VTPUFFV
{
    VT *pvtSuper = g_vtvol.pvtSuper;
    CID cid = CID_PUFFV;
    int grfcid = 0;
    int cb = 0xD0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(VOL*, CBinaryInputStream*) = LoadVolFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPUFFV g_vtpuffv;

struct VTEXIT
{
    VT *pvtSuper = g_vtalo.pvtSuper;
    CID cid = CID_EXIT;
    int grfcid = 0x1;
    int cb = 0x340;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(EXIT*, CBinaryInputStream*) = LoadExitFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTEXIT g_vtexit;

struct VTPNT
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_PNT;
    int grfcid = 0;
    int cb = 0x50;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPNT g_vtpnt;

struct VTPNTSV
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_PNTSV;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPNTSV g_vtpntsv;

struct VTPNTS
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_PNTS;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPNTS g_vtpnts;

struct VTPNTVEC
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_PNTVEC;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPNTVEC g_vtpntvec;

struct VTHPNT
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_HPNT;
    int grfcid = 0;
    int cb = 0x80;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTHPNT g_vthpnt;

struct VTJMT
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_JMT;
    int grfcid = 0;
    int cb = 0x70;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJMT g_vtjmt;

struct VTSPIRE
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_SPIRE;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSPIRE g_vtspire;

struct VTSCAN
{
    VT *pvtSuper = g_vtpnt.pvtSuper;
    CID cid = CID_SCAN;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PNT*, CBinaryInputStream*) = LoadPntFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSCAN g_vtscan;

struct VTASEG
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_ASEG;
    int grfcid = 0x40;
    int cb = 0xC0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(ASEG*, CBinaryInputStream*) = LoadAsegFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTASEG g_vtaseg;

struct VTMAP
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_MAP;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTMAP g_vtmap;

struct VT_VISZONE
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID__VISZONE;
    int grfcid = 0;
    int cb = 0x40;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VT_VISZONE g_vt_viszone;

struct VTVISMAP
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_VISMAP;
    int grfcid = 0;
    int cb = 0x50;

    void (*pfnInitLo) (VISMAP*) = InitVismap;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) (LO*) = AddLo;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)(LO*) = AddLoHierarchy;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(VISMAP*, CBinaryInputStream*) = LoadVismapFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTVISMAP g_vismap;

struct VTFRZG
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_FRZG;
    int grfcid = 0;
    int cb = 0x78;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTFRZG g_vtfrzg;

struct VTSM
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_SM;
    int grfcid = 0;
    int cb = 0x60;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(SM*, CBinaryInputStream*) = LoadSmFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSM g_vtsm;

struct VTSGG
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_SGG;
    int grfcid = 0;
    int cb = 0x190;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTSGG g_vtsgg;

struct VTPATHZONE
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_PATHZONE;
    int grfcid = 0;
    int cb = 0x70;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(PATHZONE*, CBinaryInputStream*) = LoadPathZoneFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTPATHZONE g_vtpathzone;

struct VTRCHM
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_RCHM;
    int grfcid = 0;
    int cb = 0x400;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(RCHM*, CBinaryInputStream*) = LoadRchmFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTRCHM g_vtrchm;

struct VTRWM
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_RWM;
    int grfcid = 0;
    int cb = 0x1D0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTRWM g_vtrwm;

struct VTWR
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_WR;
    int grfcid = 0;
    int cb = 0x8F0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTWR g_vtwr;

struct VTKEYHOLE
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_KEYHOLE;
    int grfcid = 0;
    int cb = 0xB0;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(KEYHOLE*, CBinaryInputStream*) = LoadKeyholeFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTKEYHOLE g_vtkeyhole;

struct VTJSG
{
    VT *pvtSuper = g_vtlo.pvtSuper;
    CID cid = CID_JSG;
    int grfcid = 0;
    int cb = 0x70;

    void (*pfnInitLo) () = nullptr;
    void (*pfnSetLoDefaults) () = nullptr;
    void (*pfnAddLo) () = nullptr;
    void (*pfnRemoveLo)() = nullptr;
    void (*pfnAddLoHierarchy)() = nullptr;
    void (*pfnRemoveLoHierarchy)() = nullptr;
    void (*pfnOnLoAdd) = nullptr;
    void (*pfnOnLoRemove) = nullptr;
    void (*pfnCloneLoHierarchy)() = nullptr;
    void (*pfnCloneLo)() = nullptr;
    void (*pfnLoadLoFromBrx)(LO*, CBinaryInputStream*) = LoadLoFromBrx;
    void (*pfnAddLoRecursive) = nullptr;
    void (*pfnRemoveLoRecursive) = nullptr;
    void (*pfnHandleLoMessage) = nullptr;
    void (*pfnSendLoMessage)() = nullptr;
    void (*pfnBindLo) = nullptr;
    void (*pfnPostLoLoad)() = nullptr;
    void (*pfnUpdateLo) = nullptr;
    void (*pfnUpdateLoXfWorld) = nullptr;
    void (*pfnUpdateLoXfWorldHierarchy) = nullptr;
    void (*pfnFreezeLo) = nullptr;
    void (*pfnSetLoParent)() = nullptr;
    void (*pfnApplyLoProxy) = nullptr;
    void (*pfnSubscribeLoObject)() = nullptr;
    void (*pfnUnsubscribeLoObject)() = nullptr;
    void (*pfnSubscribeLoStruct)() = nullptr;
    void (*pfnUnsubscribeLoStruct)() = nullptr;
    void (*pfnGetLoParams) = nullptr;
    void (*pfnUpdateLoLiveEdit) = nullptr;
};

static VTJSG g_vtjsg;

// This is used to store the information about all the Local Objects
static std::unordered_map <CID, void*> g_mpcidpvt =
{
    { CID_BASIC, &g_vtbasic },
    { CID_LO, &g_vtlo},
    { CID_ALO, &g_vtalo},
    { CID_SO, &g_vtso},
    { CID_MS, &g_vtms},
    { CID_PO, &g_vtpo},
    { CID_STEP, &g_vtstep},
    { CID_JT, &g_vtjt},
    { CID_STEPGUARD, &g_vtstepguard},
    { CID_SMARTGUARD, &g_vtsmartguard},
    { CID_GOMER, &g_vtgomer},
    { CID_UBG, &g_vtubg},
    { CID_MBG, &g_vtmbg},
    { CID_BHG, &g_vtbhg},
    { CID_MURRAY, &g_vtmurray},
    { CID_PUFFC, &g_vtpuffc},
    { CID_CRFOD, &g_vtcrfod},
    { CID_CRFODB, &g_vtcrfodb},
    { CID_CRFODK, &g_vtcrfodk},
    { CID_TANK, &g_vttank},
    { CID_JP, &g_vtjp},
    { CID_HG, &g_vthg},
    { CID_MECHA, &g_vtmecha},
    { CID_ROV, &g_vtrov},
    { CID_TURRET, &g_vtturret},
    { CID_VAULT, &g_vtvault},
    { CID_PUFFER, &g_vtpuffer},
    { CID_MGV, &g_vtmgv},
    { CID_SUV, &g_vtsuv},
    { CID_CYCLE, &g_vtcycle},
    { CID_LGN, &g_vtlgn},
    { CID_JACK, &g_vtjack},
    { CID_RIPG, &g_vtripg},
    { CID_WATER, &g_vtwater},
    { CID_BRK, &g_vtbrk},
    { CID_BREAK, &g_vtbreak},
    { CID_ALBRK, &g_vtalbrk},
    { CID_CAN, &g_vtcan},
    { CID_DARTGUN, &g_vtdartgun},
    { CID_SWP, &g_vtswp},
    { CID_FRAGILE, &g_vtfragile},
    { CID_ZAPBREAK, &g_vtzapbreak},
    { CID_BRKP, &g_vtbrkp},
    { CID_BUTTON, &g_vtbutton},
    { CID_VOLBTN, &g_vtvolbtn},
    { CID_JLOVOL, &g_vtjlovol},
    { CID_SQUISH, &g_vtsquish},
    { CID_BONE, &g_vtbone},
    { CID_SPRIZE, &g_vtsprize},
    { CID_SCPRIZE, &g_vtscprize},
    { CID_LIFETKN, &g_vtlifetkn},
    { CID_CLUE, &g_vtclue},
    { CID_ALARM, &g_vtalarm},
    { CID_SENSOR, &g_vtsensor},
    { CID_LASEN, &g_vtlasen},
    { CID_CAMSEN, &g_vtcamsen},
    { CID_PRSEN, &g_vtprsen},
    { CID_BARRIER, &g_vtbarrier},
    { CID_IKH, &g_vtikh },
    { CID_TZP, &g_vttzp },
    { CID_VOLZP, &g_vtvolzp },
    { CID_CNVO, &g_vtcnvo },
    { CID_HBSK, &g_vthbsk },
    { CID_BOMB, &g_vtbomb },
    { CID_MISSILE, &g_vtmissile },
    { CID_ACCMISS, &g_vtaccmiss },
    { CID_TARMISS, &g_vttarmiss },
    { CID_SPLMISS, &g_vtsplmiss },
    { CID_GROUNDMISS, &g_vtgroundmiss },
    { CID_FLY, &g_vtfly },
    { CID_RAT, &g_vtrat },
    { CID_ROH, &g_vtroh },
    { CID_ROC, &g_vtroc },
    { CID_ROST, &g_vtrost },
    { CID_ROP, &g_vtrop },
    { CID_DART, &g_vtdart },
    { CID_UBV, &g_vtubv },
    { CID_UBP, &g_vtubp },
    { CID_DSP, &g_vtdsp },
    { CID_JLO, &g_vtjlo },
    { CID_PUFFT, &g_vtpufft },
    { CID_MRKV, &g_vtmrkv },
    { CID_LGNB, &g_vtlgnb },
    { CID_BLIPG, &g_vtblipg },
    { CID_CAMERA, &g_vtcamera },
    { CID_LBONE, &g_vtlbone },
    { CID_EMITTER, &g_vtemitter },
    { CID_LIGHT, &g_vtlight },
    { CID_SCH, &g_vtsch },
    { CID_LIKH, &g_vtlikh },
    { CID_CHKPNT, &g_vtchkpnt },
    { CID_PROXY, &g_vtproxy },
    { CID_SKY, &g_vtsky },
    { CID_DPRIZE, &g_vtdprize },
    { CID_CHARM, &g_vtcharm },
    { CID_COIN, &g_vtcoin },
    { CID_KEY, &g_vtkey },
    { CID_GOLD, &g_vtgold },
    { CID_LOCK, &g_vtlock },
    { CID_LOCKG, &g_vtlockg },
    { CID_TAIL, &g_vttail },
    { CID_ROB, &g_vtrob },
    { CID_FLASH, &g_vtflash },
    { CID_DYSH , &g_vtdysh },
    { CID_SCENTMAP , &g_vtscentmap },
    { CID_WAYPOINT , &g_vtwaypoint },
    { CID_TN , &g_vttn },
    { CID_JLOC , &g_vtjloc },
    { CID_DIALOG , &g_vtdialog },
    { CID_SPEAKER , &g_vtspeaker },
    { CID_ROPE , &g_vtrope },
    { CID_WM , &g_vtwm },
    { CID_PUFFB , &g_vtpuffb },
    { CID_CRBRAIN , &g_vtcrbrain },
    { CID_MGC , &g_vtmgc },
    { CID_JACKB , &g_vtjackb },
    { CID_JACKN , &g_vtjackn },
    { CID_JACKF , &g_vtjackf },
    { CID_SW, &g_vtsw },
    { CID_CM, &g_vtcm },
    { CID_SHAPE , &g_vtshape },
    { CID_HSHAPE , &g_vthshape },
    { CID_PIPE , &g_vtpipe },
    { CID_RAIL , &g_vtrail },
    { CID_LANDING , &g_vtlanding },
    { CID_XFM , &g_vtxfm },
    { CID_WARP , &g_vtwarp },
    { CID_TARGET , &g_vttarget },
    { CID_HND , &g_vthnd },
    { CID_EXPL , &g_vtexpl },
    { CID_EXPLG , &g_vtexplg },
    { CID_EXPLO , &g_vtexplo },
    { CID_EXPLS, &g_vtexpls },
    { CID_VOL, &g_vtvol },
    { CID_RATHOLE, &g_vtrathole },
    { CID_PUFFV, &g_vtpuffv },
    { CID_EXIT, &g_vtexit },
    { CID_PNT, &g_vtpnt },
    { CID_PNTSV, &g_vtpntsv },
    { CID_PNTS, &g_vtpnts },
    { CID_PNTVEC, &g_vtpntvec },
    { CID_HPNT, &g_vthpnt },
    { CID_JMT, &g_vtjmt },
    { CID_SPIRE, &g_vtspire },
    { CID_SCAN, &g_vtscan },
    { CID_ASEG, &g_vtaseg },
    { CID_MAP, &g_vtmap },
    { CID__VISZONE, &g_vt_viszone },
    { CID_VISMAP, &g_vismap },
    { CID_FRZG, &g_vtfrzg },
    { CID_SM, &g_vtsm },
    { CID_SGG, &g_vtsgg },
    { CID_PATHZONE, &g_vtpathzone },
    { CID_RCHM, &g_vtrchm },
    { CID_RWM, &g_vtrwm },
    { CID_WR, &g_vtwr },
    { CID_KEYHOLE, &g_vtkeyhole },
    { CID_JSG, &g_vtjsg }
};