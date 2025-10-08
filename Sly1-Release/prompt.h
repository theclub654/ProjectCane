#pragma once
#include "blot.h"
#include "resp.h"

struct PROMPT : public BLOT
{
    PRK prk;
    int irespk;
    PRK mpprpprk[3];
    int fReshow;
    PRK prkReshow;
};

void StartupPrompt(PROMPT* pprompt);
void PostPromptLoad(PROMPT* pprompt);
void SetPrompt(PROMPT* pprompt, PRP prp, PRK prk);
// GOTTA COME BACK TO THIS
void SetPromptPrk(PROMPT* pprompt, PRK prk);
void ExecutePrompt(PROMPT* pprompt);
void SetPromptBlots(PROMPT* pprompt, BLOTS blots);
const char* AchzFromRespk(RESPK respk);
void OnPromptActive(PROMPT* pprompt, int fActive);
void UpdatePromptActive(PROMPT* pprompt, JOY* pjoy);
// Cancels or transitions the given prompt depending on its type and timing
void CancelPrompt(PROMPT* pprompt);
void DrawPrompt(PROMPT* pprompt);

extern PROMPT g_prompt;
extern CTextEdge g_tePrompt;
extern float g_promptFade;