#include "screen.h"

void StartupScreen()
{
    StartupLetterbox(&g_letterbox);
    StartupBinoc(&g_binoc);
    StartupLgnr(&g_lgnr);
    StartupLifeCtr(&g_lifectr);
    StartupClueCtr(&g_cluectr);
    StartupKeyCtr(&g_keyctr);
    StartupGoldCtr(&g_goldctr);
    StartupCoinCtr(&g_coinctr);
    StartupTrunkCtr(&g_trunkctr);
    StartupCrusherCtr(&g_crusherctr);
    StartupLapCtr(&g_lapctr);
    StartupBoostCtr(&g_boostctr);
    StartupPlaceCtr(&g_placectr);
    StartupBossCtr(&g_bossctr);
    StartupPuffChargerCtr(&g_puffchargectr);
    StartupTimer(&g_timer);
    StartupNote(&g_note);
    StartupCredit(&g_credit);
    StartupTitle(&g_title);
    StartupTotals(&g_totals);
    StartupCall(&g_call);
    StartupWmc(&g_wmc);
    StartupPrompt(&g_prompt);
    StartupTvLeft(&g_tvLeft);
    StartupTvRight(&g_tvRight);
    StartupScores(&g_scores);
    StartupLogo(&g_logo);
    StartupAttract(&g_attract);
    StartupCredit(&g_credit);
    StartupDebugMenu(&g_debugmenu);

    for (int i = 0; i < 29; i++)
        s_apblot[i]->pvtblot->pfnInitBlot(s_apblot[i], (BLOTK)i);
}