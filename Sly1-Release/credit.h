#pragma once
#include "blot.h"

enum MTSSTATE 
{
    MTSSTATE_None = 0,
    MTSSTATE_Eligible = 1,
    MTSSTATE_Active = 2,
    MTSSTATE_Finished = 3,
    MTSSTATE_Declined = 4
};

struct COMMENTARY
{
    int levelId;
    float audioDuration;
    const char* pchzAudio;
    const char* pchzSpeakerNames;
};

struct NOTE : public BLOT
{
    MTSSTATE mtsState;
    float tMtsState;
    int fGoldEnabled;
    int fCommentaryPromptActive;
    class GOLD* pgold;
};

struct CREDIT : public BLOT
{
    int cnote;
    NOTE anote[4];
};

COMMENTARY*PcommentaryFromLevelId(int levelId);
void StartupNote(NOTE* pnote);
void PostNoteLoad(NOTE* pnote);
void UpdateNote(NOTE* pnote);
void SetNoteMtsState(NOTE* pnote, MTSSTATE mtsState);
void DrawNote(NOTE* pnote);
void FinishNoteSprint(NOTE* pnote);

void StartupCredit(CREDIT* pcredit);
void InitCredit(CREDIT* pcredit, BLOTK blotk);
void PostCreditLoad(CREDIT* pcredit);
void SetCreditClock(CREDIT* pcredit, float* pt);
void UpdateCredit(CREDIT* pcredit);
void DrawCredit(CREDIT* pcredit);
void PlaceCredit(CREDIT* pcredit, float x, float y, int cline);
void VacateCredit(CREDIT* pcredit);
void SetCreditLine(CREDIT* pcredit, int iline, char* pchz, float dtVisible);

extern CREDIT g_credit;
extern NOTE g_note;
extern CTextEdge g_teNote;
extern NOTE *g_pnote;
extern COMMENTARY g_aCommentaryLoadData[33];
static const char* s_pchzBonusCommentaryPrompt = "Press &2L&. for Bonus Commentary";
static constexpr char s_achzCommentaryTimeTenths[] = "Best time: &10.%d";
static constexpr char s_achzCommentaryTimeMinutes[] = "Best time: &1%d:%02d";
static constexpr char s_pchzCommentaryTimer[] = "Beginning Master Thief Sprint!"; 
static char s_achzCommentaryTime[64] = {};
