#pragma once
#include "alo.h"

enum DIALOGK 
{
    DIALOGK_Nil = -1,
    DIALOGK_Binoc = 0,
    DIALOGK_Instruct = 1,
    DIALOGK_Confront = 2,
    DIALOGK_Max = 3
};
enum DIALOGS
{
    DIALOGS_Nil = -1,
    DIALOGS_Enabled = 0,
    DIALOGS_Triggered = 1,
    DIALOGS_Disabled = 2,
    DIALOGS_Playing = 3,
    DIALOGS_Calling = 4,
    DIALOGS_UnableToCall = 5,
    DIALOGS_Max = 6
};
enum DEK
{
    DEK_Nil = -1,
    DEK_PreloadVag = 0,
    DEK_SpeakerLeft = 1,
    DEK_SpeakerRight = 2,
    DEK_Talk = 3,
    DEK_CallSplice = 4,
    DEK_ShowLetterbox = 5,
    DEK_HideLetterbox = 6,
    DEK_ShowMovie = 7,
    DEK_Max = 8
};
enum DPK
{
    DPK_Nil = -1,
    DPK_None = 0,
    DPK_X = 1,
    DPK_Square = 2,
    DPK_Circle = 3,
    DPK_Triangle = 4,
    DPK_L1 = 5,
    DPK_L2 = 6,
    DPK_R1 = 7,
    DPK_R2 = 8,
    DPK_JoyLeft = 9,
    DPK_JoyRight = 10,
    DPK_JoyLeftX = 11,
    DPK_JoyLeftY = 12,
    DPK_DoubleJump = 13,
    DPK_VaultOpen = 14,
    DPK_Max = 15
};

struct DP 
{
    uint32_t grfdp;
    DPK dpk;
    struct SPEAKER* pspeakerLeft;
    struct SPEAKER* pspeakerRight;
    struct ASEGA* pasegaLipsync;
    float tPauseMin;
};

// DIALOG EVENT
struct DE 
{
    DEK dek;
    union
    {
        union
        {
            char* pchz;
        };
        union
        {
            OID oid;
        };
        union
        {
            OID oid;
            OID oidReplace;
        };
        union
        {
            float uZoom;
            float dtPause;
            int fUser;
            DPK dpk;
            char* pchzText;
            OID oidSpeaker;
            OID oidLipsAseg;
            OID oidFocus;
            OID oidPntAnchor;
        };
        union
        {
            OID oidLabel;
        };
    };
};

class DIALOG : public ALO
{
	public:
        DIALOGK dialogk;
        DIALOGS dialogs;
        float tDialogs;
        int cde;
        struct DE* ade;
        int ideCur;
        DP dp;
        OID oidPntAnchor;
        int* pfPlayed;
        int fPlayed;
        int cpfEquivalence;
        int* apfEquivalence[4];
        int fDefault;
        int fCrucial;
};

DIALOG*NewDialog();
void InitDialog(DIALOG* pdialog);
int  GetDialogSize();
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void CloneDialog(DIALOG* pdialog, DIALOG* pdialogBase);
void LoadDialogEventsFromBrx(DIALOG* pdialog, CBinaryInputStream *pbis, int* pcde, DE** pade);
void DeleteDialog(DIALOG* pdialog);

inline DIALOG *g_pdialogPlaying = nullptr;
inline DIALOG *g_pdialogCalling = nullptr;
inline DIALOG* g_pdialogTriggered = nullptr;