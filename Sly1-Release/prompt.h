#pragma once
#include "blot.h"
#include "resp.h"
#include "wipe.h"

struct PROMPT : public BLOT
{
    PRK prk;
    int irespk;
    PRK mpprpprk[3];
    int fReshow;
    PRK prkReshow;
    PRK prkSaveContext;
    float alpha;
    int fVagPlaying;
    class SMA* psma;
};

void StartupPrompt(PROMPT* pprompt);
void PostPromptLoad(PROMPT* pprompt);
void HandlePromptPrkTransition(PROMPT* pprompt, PRK prkOld, PRK prkNew);
void SetPrompt(PROMPT* pprompt, int prp, PRK prk);
SMA* PsmaFindSpecialPromptStateMachine();
void SetPromptPrk(PROMPT* pprompt);
void ChangePromptPrk(PROMPT* pprompt, PRK prk);
void ExecutePrompt(PROMPT* pprompt);
void SetPromptBlots(PROMPT* pprompt, BLOTS blots);
void ShowPrompt(PROMPT* pprompt);
void HidePrompt(PROMPT* pprompt);
const char* AchzFromRespk(RESPK respk);
void BuildSaveSlotText(GS* pgs, char* pchzDst);
void OnPromptActive(PROMPT* pprompt, int fActive);
void ExitOrRestartLevelFromPrompt(PROMPT* pprompt, int fRestartLevel, WIPEK wipek);
void WipeToTitleScreen();
void UpdatePromptActive(PROMPT* pprompt, JOY* pjoy);
void CancelPrompt(PROMPT* pprompt);
void DrawPrompt(PROMPT* pprompt);

char GetAnimatedPromptCharacter();

extern PROMPT g_prompt;
extern CTextEdge g_tePrompt;
extern float g_promptFade;
inline constexpr char s_achzAnimatedPromptFrames[] = "ab";