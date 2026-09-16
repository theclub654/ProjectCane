#pragma once
#include "so.h"

enum BTNT 
{
    BTNT_Nil = -1,
    BTNT_Push = 0,
    BTNT_Lever = 1,
    BTNT_Contact = 2,
    BTNT_Whack = 3,
    BTNT_Hang = 4,
    BTNT_Max = 5
};

enum IASH 
{
    IASH_On = 0,
    IASH_Off = 1,
    IASH_Max = 2
};

struct ASH 
{
    int coid;
    OID aoid[16];
    int cpaseg;
    struct ASEG* apaseg[16];
};
enum BUTTONS 
{
    BUTTONS_Nil = -1,
    BUTTONS_NoContact = 0,
    BUTTONS_Contact = 1,
    BUTTONS_Pushed = 2,
    BUTTONS_Disabled = 3,
    BUTTONS_Reset = 4,
    BUTTONS_Max = 5
};

struct BTN 
{
    struct VTBTN* pvtbtn;
    class ALO* paloOwner;
    ASH aash[2];
    float svtAnimation;
    int fCheckpointed;
    int ichkPushed;
    float tButtons;
    float dtRepush;
    BUTTONS buttons;
    int fEnabled;
    int fSilent;
    int fReapplyAseg;
    int fManualReset;
    short oidDialog;
    struct DIALOG* pdialog;
    int crsmg;
    RSMG arsmg[8];
};

class BUTTON : public SO
{
	public:
    BTNT btnt;
    BTN btn;
    int coidPush;
    OID aoidPush[8];
    int coidNoPush;
    OID aoidNoPush[8];
    int ccidPush;
    CID acidPush[8];
    int ccidNoPush;
    CID acidNoPush[8];
    int istopClosestCur;
    int cposStop;
    glm::vec3 aposStop[2];
    int cmatStop;
    glm::mat3 amatStop[2];
    glm::vec3 posLever;
    glm::vec3 anormLimits[2];
    struct HND* phndSwitchHook;
};

class VOLBTN : public SO
{
	public:
    int coidPush;
    OID aoidPush[8];
    int cploPush;
    struct LO* aploPush[8];
    int cploPushWithin;
    struct LO* aploPushWithin[8];
    int fPpoPush;
    BTN btn;
    int fMustExit;
    int fSendChangeEvents;
};

BUTTON*NewButton();
void InitButton(BUTTON* pbutton);
void*GetButtonDtRepush(BUTTON* pbutton);
void SetButtonDtRepush(BUTTON* pbutton, float dtRepush);
void*GetButtonBtnt(BUTTON* pbutton);
void SetButtonBtnt(BUTTON* pbutton, BTNT btnt);
void*GetButtonSvtAnimation(BUTTON* pbutton);
void SetButtonSvtAnimation(BUTTON* pbutton, float svtAnimation);
void*GetButtonFSilent(BUTTON* pbutton);
void SetButtonFSilent(BUTTON* pbutton, int fSilent);
void*GetButtonFReapplyAseg(BUTTON* pbutton);
void SetButtonFReapplyAseg(BUTTON* pbutton, int fReapplyAseg);
void*GetButtonFManualReset(BUTTON* pbutton);
void SetButtonFManualReset(BUTTON* pbutton, int fManualReset);
void*GetButtonFCheckpointed(BUTTON* pbutton);
void SetButtonFCheckpointed(BUTTON* pbutton, int fCheckpointed);
void*GetButtonOidDialog(BUTTON* pbutton);
void SetButtonOidDialog(BUTTON* pbutton, OID oidDialog);
void*GetButtonButtons(BUTTON* pbutton);
int  GetButtonSize();
void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis);
void InsertButtonPos(BUTTON* pbutton, PNT* ppnt, glm::vec3* ppos);
void InsertButtonMat(BUTTON* pbutton, XFM* pxfm, glm::mat3* pmat);
void CloneButton(BUTTON* pbutton, BUTTON* pbuttonBase);
void PostButtonLoad(BUTTON* pbutton);
void SetButtonButtons(BUTTON* pbutton, BUTTONS buttons);
int  FCheckButtonObject(BUTTON* pbutton, SO* psoOther);
int  IposFindButtonClosest(BUTTON* pbutton, float* psDist);
int  ImatFindButtonClosest(BUTTON* pbutton, float* prad);
void PresetButtonAccel(BUTTON* pbutton, float dt);
void UpdateButtonInternalXps(BUTTON* pbutton);
void UpdateButton(BUTTON* pbutton, float dt);
int  FAbsorbButtonWkr(BUTTON* pbutton, WKR* pwkr);
void DeleteButton(BUTTON *pbutton);

void PostAshLoad(SW* psw, ASH* pash, ALO* paloOwner);
int  FFoundAshAseg(ASH* pash, ASEG* paseg);
int  FAddAshAseg(ASH* pash, ASEG* paseg);
int  FAddAshOid(ASH* pash, OID oid);

void InitBtn(BTN* pbtn);
void LoadBtn(BTN* pbtn, ALO* paloOwner);
void PostBtnLoad(BTN* pbtn);
void RestoreBtnFromCheckpointCallback(BTN* pbtn, MSGID msgid, void* pv);
void SetBtnRsmg(BTN* pbtn, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal);
void SetBtnButtons(BTN* pbtn, BUTTONS buttons);
int  FAddRsmg(RSMG* arsmg, int crsmgMax, int* pcrsmg, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal);
void TriggerRsmg(SW* psw, int crsmg, RSMG* arsmg, LO* ploContext, int fTrigger);
void RunBtnAsegs(BTN* pbtn, IASH asht, int fSeekToEnd, int fChkTrigger);
void TriggerBtn(BTN* pbtn, int fSeekToEnd, int fChkTrigger);
void UntriggerBtn(BTN* pbtn, int fSeekToEnd);

VOLBTN*NewVolbtn();
void InitVolbtn(VOLBTN* pvolbtn);
void*GetVolbtnDtRepush(VOLBTN* pvolbtn);
void SetVolbtnDtRepush(VOLBTN* pvolbtn, float dtRepush);
void*GetVolbtnSvtAnimation(VOLBTN* pvolbtn);
void SetVolbtnSvtAnimation(VOLBTN* pvolbtn, float svtAnimation);
void*GetVolbtnFSilent(VOLBTN* pvolbtn);
void SetVolbtnFSilent(VOLBTN* pvolbtn, int fSilent);
void*GetVolbtnFReapplyAseg(VOLBTN* pvolbtn);
void SetVolbtnFReapplyAseg(VOLBTN* pvolbtn, int fReapplyAseg);
void*GetVolbtnFCheckpointed(VOLBTN* pvolbtn);
void SetVolbtnFCheckpointed(VOLBTN* pvolbtn, int fCheckpointed);
void*GetVolbtnFMusicExit(VOLBTN* pvolbtn);
void SetVolbtnFMusicExit(VOLBTN* pvolbtn, int fMusicExit);
void*GetVolbtnFSendChangeEvents(VOLBTN* pvolbtn);
void SetVolbtnFSendChangeEvents(VOLBTN* pvolbtn, int fSendChangeEvents);
void*GetVolbtnOidDialog(VOLBTN* pvolbtn);
void SetVolbtnOidDialog(VOLBTN* pvolbtn, OID oidDialog);
void*GetVolbtnButtons(VOLBTN* pvolbtn);
int  GetVolbtnSize();
void CloneVolbtn(VOLBTN* pvolbtn, VOLBTN* pvolbtnBase);
void LoadVolbtnFromBrx(VOLBTN *pvolbtn, CBinaryInputStream *pbis);
void PostVolbtnLoad(VOLBTN* pvolbtn);
void SetVolbtnButtons(VOLBTN* pvolbtn, BUTTONS buttons);
void UpdateVolbtn(VOLBTN* pvolbtn, float dt);
int  FGetVolbtnPushObjectsWithinList(VOLBTN* pvolbtn, void* pvstate);
void DeleteVolbtn(VOLBTN* pvolbtn);

void AddBtnAseg(BTN* pbtn, ALO* palo, OID oid);
void AddButtonAseg(BUTTON* pbutton, int oid);
void AddVolbtnAseg(VOLBTN* pvolbtn, OID oid);
void AddBtnOffAseg(BTN* pbtn, ALO* palo, OID oid);
void AddButtonOffAseg(BUTTON* pbutton, int oid);
void AddVolbtnOffAseg(VOLBTN* pvolbtn, OID oid);
void AddButtonPushObject(BUTTON* pbutton, OID oid);
void AddButtonNoPushObject(BUTTON* pbutton, OID oid);
void AddButtonPushClass(BUTTON* pbutton, CID cid);
void AddButtonNoPushClass(BUTTON* pbutton, CID cid);
void AddVolbtnPushObject(VOLBTN* pvolbtn, OID oid);
void SetButtonRsmg(BUTTON* pbutton, int fOnTrigger, int oidRoot, int oidSm, int oidGoal);
void SetVolbtnRsmg(VOLBTN* pvolbtn, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal);