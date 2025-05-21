#include "credit.h"

void StartupNote(NOTE* pnote)
{
    pnote->pvtnote = &g_vtnote;
}

void PostNoteLoad(NOTE* pnote)
{
    PostBlotLoad(pnote);

    if (g_pfontScreenCounters != nullptr)
    {
        pnote->pte = &g_teNote;
        //g_teNote.m_pfont = pCVar2;
    }
}

void DrawNote(NOTE* pnote)
{

}

void StartupCredit(CREDIT* pcredit)
{
    pcredit->pvtcredit = &g_vtcredit;
}

void InitCredit(CREDIT* pcredit, BLOTK blotk)
{
    InitBlot(pcredit, BLOTK_Credit);

    for (int i = 0; i < 4; i++)
    {
        g_vtnote.pfnInitBlot(&pcredit->anote[i], BLOTK_Note);
        pcredit->anote[i].pvtnote = &g_vtnote;
    }
}

void PostCreditLoad(CREDIT* pcredit)
{
    PostBlotLoad(pcredit);

    for (int i = 0; i < 4; i++)
    {
        pcredit->anote[i].pvtblot->pfnPostBlotLoad(&pcredit->anote[i]);

        pcredit->anote[i].pte = nullptr;
        pcredit->pfont->m_rxScale = 0.8;
        pcredit->pfont->m_ryScale = 0.8;

        SetBlotDtAppear(&pcredit->anote[i], 1.0);
        SetBlotDtDisappear(&pcredit->anote[i], 1.0);
        SetBlotDtVisible(&pcredit->anote[i], 0.0);
    }
}

void SetCreditClock(CREDIT* pcredit, float* pt)
{

}

void UpdateCredit(CREDIT* pcredit)
{

}

void DrawCredit(CREDIT* pcredit)
{

}

CREDIT g_credit;
NOTE g_note;