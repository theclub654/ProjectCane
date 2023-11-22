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
    struct ALO* paloOwner;
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
    OID oidDialog;
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

void* NewButton();
void InitButton(BUTTON* pbutton);
void DeleteButton(LO* plo);
void* NewVolbtn();
void InitVolbtn(VOLBTN* pvolbtn);
void LoadButtonFromBrx(BUTTON *pbutton, CBinaryInputStream *pbis);
void LoadVolbtnFromBrx(VOLBTN *pvolbtn, CBinaryInputStream *pbis);
void DeleteVolbtn(LO* plo);