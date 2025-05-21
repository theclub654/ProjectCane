#pragma once
#include "blot.h"

struct NOTE : public BLOT
{

};

struct CREDIT : public BLOT
{
    int cnote;
    NOTE anote[4];
};

void StartupNote(NOTE* pnote);
void PostNoteLoad(NOTE* pnote);
void DrawNote(NOTE* pnote);

void StartupCredit(CREDIT* pcredit);
void InitCredit(CREDIT* pcredit, BLOTK blotk);
void PostCreditLoad(CREDIT* pcredit);
void SetCreditClock(CREDIT* pcredit, float* pt);
void UpdateCredit(CREDIT* pcredit);
void DrawCredit(CREDIT* pcredit);

extern CREDIT g_credit;
extern NOTE g_note;