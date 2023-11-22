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

void* NewDialog();
void InitDialog(DIALOG* pdialog);
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void LoadDialogEventsFromBrx(CBinaryInputStream *pbis);
void DeleteDialog(LO* plo);