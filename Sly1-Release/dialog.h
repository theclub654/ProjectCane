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
    DIALOGS_Playing = 2,
    DIALOGS_Calling = 3,
    DIALOGS_UnableToCall = 4,
    DIALOGS_Unavailable = 5,
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

enum GRFDP
{
    GRFDP_VagPreloaded = 1 << 0,
    GRFDP_SpeakerLeft = 1 << 1,
    GRFDP_SpeakerRight = 1 << 2,
    GRFDP_Talk = 1 << 3,
    GRFDP_ExplicitLipsync = 1 << 4,
    GRFDP_Text = 1 << 5,
    GRFDP_Zoom = 1 << 6,
    GRFDP_Pause = 1 << 7,
    GRFDP_UserInput = 1 << 8
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

    struct
    {
        std::shared_ptr<char[]> pchz;
    } string;

    struct
    {
        OID oid;
    } object;

    struct
    {
        OID oid;
        OID oidReplace;
    } speaker;

    struct
    {
        float uZoom;
        float dtPause;
        int fUser;
        DPK dpk;
        std::shared_ptr<char[]> pchzText;
        OID oidSpeaker;
        OID oidLipsAseg;
        OID oidFocus;
        OID oidPntAnchor;
    } talk;

    struct
    {
        OID oidLabel;
    } splice;
};

class DIALOG : public ALO
{
	public:
    DIALOGK dialogk;
    DIALOGS dialogs;
    float tDialogs;
    int cde;
    std::vector <DE> ade;
    int ideCur;
    DP dp;
    short oidPntAnchor;
    int* pfPlayed;
    int fPlayed;
    int cpfEquivalence;
    int* apfEquivalence[4];
    int fDefault;
    int fCrucial;
    int oidPntAnchorDefault;
    int fNoSkip;
    int fConfrontAudio;
};

// GOTTA COME BACK TO THIS LATER
DIALOG*NewDialog();
void InitDialog(DIALOG* pdialog);
void*GetDialogDialogs(DIALOG* pdialog);
void*GetDialogOidPntAnchorDefault(DIALOG* pdialog);
void SetDialogOidPntAnchorDefault(DIALOG* pdialog, int oidPntAnchorDefault);
void*GetDialogFDefault(DIALOG* pdialog);
void SetDialogFDefault(DIALOG* pdialog, int fDefault);
void*GetDialogFCrucial(DIALOG* pdialog);
void SetDialogFCrucial(DIALOG* pdialog, int fCrucial);
void*GetDialogFNoSkip(DIALOG* pdialog);
void SetDialogFNoSkip(DIALOG* pdialog, int fNoSkip);
void*GetDialogOidPntAnchor(DIALOG* pdialog);
void SetDialogOidPntAnchor(DIALOG* pdialog, int oidPntAnchor);
void*GetDialogFConfrontAudio(DIALOG* pdialog);
void SetDialogFConfrontAudio(DIALOG* pdialog, int fConfrontAudio);
int  GetDialogSize();
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void CloneDialog(DIALOG* pdialog, DIALOG* pdialogBase);
void LoadDialogEventsFromBrx(DIALOG* pdialog, CBinaryInputStream *pbis, int* pcde, std::vector<DE>& ade);
void PostDialog(DIALOG* pdialog);
void SetDialogInstruct(DIALOG* pdialog);
void SetDialogConfront(DIALOG* dialog);
void GetDialogPlayed(DIALOG* pdialog, int* pfPlayed);
void SetDialogPlayed(DIALOG* pdialog, int fPlayed);
void SetDialogDialogs(DIALOG* pdialog, DIALOGS dialogs);
void HandleDialogMessage(DIALOG* pdialog, MSGID msgid, void* pv);
bool FPauseDialog(DIALOG* pdialog);
void FinishDialogEvents(DIALOG* pdialog);
void HandleDialogEvents(DIALOG* pdialog);
void StartDialog(DIALOG* pdialog, PO* ppo);
void TriggerDialog(DIALOG* pdialog);
void UntriggerDialog(DIALOG* pdialog);
void AddDialogEquivalence(DIALOG* pdialog, int wid, int oid);
void UpdateDialog(DIALOG* pdialog, float dt);
void HandleDialogEvent(DIALOG* pdialog, DE* pde);
void DeleteDialog(DIALOG* pdialog);

inline DIALOG* g_pdialogPlaying = nullptr;
inline DIALOG* g_pdialogCalling = nullptr;
static constexpr char s_mpdpkchPause[16] =
{
    'X',  // DPK 0: Cross
    'X',  // DPK 1: Cross/default
    'S',  // DPK 2: Square
    'O',  // DPK 3: Circle
    'T',  // DPK 4: Triangle
    'L',  // DPK 5: L1
    'l',  // DPK 6: L2
    'R',  // DPK 7: R1
    'r',  // DPK 8: R2
    '(',  // DPK 9: left-stick glyph
    ')',  // DPK 10: right-stick glyph
    '(',  // DPK 11: left-stick X glyph
    '(',  // DPK 12: left-stick Y glyph
    'X',  // DPK 13: double-jump/Cross glyph
    '\0', // DPK 14: no prompt glyph
    '\0'  // DPK 15: nil/max
};

static constexpr uint16_t s_mpdpkgrfbtn[16] =
{
    0x0040,
    0x0040,
    0x0080,
    0x0020,
    0x0010,
    0x0004,
    0x0001,
    0x0008,
    0x0002,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000
};

extern int g_cdialogTriggered;