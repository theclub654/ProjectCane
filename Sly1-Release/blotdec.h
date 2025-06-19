#pragma once
#include <iostream>

struct BLOT;
struct CTR;
struct UI;
struct KEYCTR;
struct LIFECTR;
struct COINCTR;
struct GOLDCTR;
struct TRUNKCTR;
struct CRUSHERCTR;
struct LAPCTR;
struct BOOSTCTR;
struct PLACECTR;
struct CLUECTR;
struct BOSSCTR;
struct TV;
struct PUFFCHARGERCTR;
struct TIMER;
struct NOTE;
struct TITLE;
struct TOTALS;
struct CALL;
struct WMC;
struct PROMPT;
struct BINOC;
struct LGNR;
struct SCORES;
struct LOGO;
struct ATTRACT;
struct LETTERBOX;
struct CREDIT;
struct DEBUGMENU;
struct JOY;
enum PRP;
enum PRK;
enum RESPK;
enum BLOTS;
enum BLOTK;
enum UIS;

using PFNSMACK = void(*)(void);
using PFNTN = void(*)(void);

void PostCtrLoad(CTR* pctr);
void UpdateCtr(CTR* pctr);
float DtVisibleCtr(CTR* pctr);
void DrawCtr(CTR* pctr);

void StartupLetterbox(LETTERBOX* pletterbox);
void DrawLetterBox(LETTERBOX* pletterbox);

void StartupBinoc(BINOC* pbinoc);
void InitBinoc(BINOC* pbinoc, BLOTK blotk);
void PostBinocLoad(BINOC* pbinoc);
void OnBinocActive(BINOC* pbinoc, int fActive);
void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy);
void OnBinocReset(BINOC* pbinoc);
void OnBinocPush(BINOC* pbinoc);
void OnBinocPop(BINOC* pbinoc);
void SetBinocAchzDraw(BINOC* pbinoc, char *pchz);
float DtAppearBinoc(BINOC* pbinoc);
float DtDisappearBinoc(BINOC* pbinoc);
void SetBinocBlots(BINOC* pbinoc, BLOTS blots);
void DrawBinoc(BINOC* pbinoc);

void StartupLgnr(LGNR* plgnr);
void DrawLgnr(LGNR* plgnr);

void StartupLifeCtr(LIFECTR* plifectr);

void StartupClueCtr(CLUECTR* pcluectr);

void StartupKeyCtr(KEYCTR* pkeyctr);

void StartupGoldCtr(GOLDCTR* pgoldctr);

void StartupCoinCtr(COINCTR* pcoinctr);

void StartupTrunkCtr(TRUNKCTR* ptrunkctr);
float DtVisibleTrunkctr(TRUNKCTR* ptrunkctr);

void StartupCrusherCtr(CRUSHERCTR* pcrusherctr);
float DtVisibleCrusherctr(CRUSHERCTR* pcrusherctr);

void StartupLapCtr(LAPCTR* plapctr);

void StartupBoostCtr(BOOSTCTR* pboostctr);

void StartupPlaceCtr(PLACECTR* placectr);

void StartupBossCtr(BOSSCTR* bossctr);
void PostBossctrLoad(BOSSCTR* pbossctr);
void DrawBossCtr(BOSSCTR* pbossctr);

void StartupPuffChargerCtr(PUFFCHARGERCTR* ppuffchargerctr);
float DtVisiblePuffchargerctr(PUFFCHARGERCTR* ppuffchargectr);

void StartupTimer(TIMER* ptimer);
void PostTimerLoad(TIMER* ptimer);
void UpdateTimer(TIMER* ptimer);
void DrawTimer(TIMER* ptimer);

void StartupTitle(TITLE* ptitle);
void PostTitleLoad(TITLE* ptitle);
int  FIncludeTitleForPeg(TITLE* ptitle, BLOT* pblotOther);
void SetTitleAchzDraw(TITLE* ptitle, char* pchz);
void SetTitleBlots(TITLE* ptitle, BLOTS blots);
void DrawTitle(TITLE* ptitle);

void StartupTotals(TOTALS* ptotals);
void PostTotalsLoad(TOTALS* ptotals);
void SetTotalsBlots(TOTALS* ptotals, BLOTS blots);
void DrawTotals(TOTALS* ptotals);

void StartupCall(CALL* pcall);
void PostCallLoad(CALL* pcall);
void DrawCall(CALL* pcall);

void StartupWmc(WMC* pwmc);
void PostWmcLoad(WMC* pwmc);
void OnWmcActive(WMC* pwmc, int fActive);
void UpdateWmcActive(WMC* pwmc, JOY* pjoy);
void DrawWmc(WMC* pwmc);

void StartupPrompt(PROMPT* pprompt);
void PostPromptLoad(PROMPT* pprompt);
void SetPrompt(PROMPT* pprompt, PRP prp, PRK prk);
void SetPromptPrk(PROMPT* pprompt, PRK prk);
void ExecutePrompt(PROMPT* pprompt);
void SetPromptBlots(PROMPT* pprompt, BLOTS blots);
const char* AchzFromRespk(RESPK respk);
void OnPromptActive(PROMPT* pprompt, int fActive);
void UpdatePromptActive(PROMPT* pprompt, JOY* pjoy);
void CancelPrompt(PROMPT* pprompt);
void DrawPrompt(PROMPT* pprompt);

void StartupTvLeft(TV* ptvleft);
void StartupTvRight(TV* ptvright);
void InitTv(TV* ptv, BLOTK blotk);
void PostTvLoad(TV* ptv);
void OnTvReset(TV* ptv);
void SetTvBlots(TV* ptv, BLOTS blots);
void UpdateTv(TV* ptv);
void DrawTv(TV* ptv);

void StartupScores(SCORES* pscores);
void PostScoresLoad(SCORES* pscores);
void UpdateScores(SCORES* pscores);
void DrawScores(SCORES* pscores);

void StartupLogo(LOGO* plogo);
void PostLogoLoad(LOGO* plogo);
void DrawLogo(LOGO* plogo);

void StartupAttract(ATTRACT* pattract);
void PostAttractLoad(ATTRACT* pattract);
void SetAttractAchzDraw(ATTRACT* pattract, char* pchz);
void SetAttractBlots(ATTRACT* pattract, BLOTS blots);
void UpdateAttract(ATTRACT* pattract);
void DrawAttract(ATTRACT* pattract);

void StartupNote(NOTE* pnote);
void PostNoteLoad(NOTE* pnote);
void DrawNote(NOTE* pnote);

void StartupCredit(CREDIT* pcredit);
void InitCredit(CREDIT* pcredit, BLOTK blotk);
void PostCreditLoad(CREDIT* pcredit);
void SetCreditClock(CREDIT* pcredit, float* pt);
void UpdateCredit(CREDIT* pcredit);
void DrawCredit(CREDIT* pcredit);

void StartupDebugMenu(DEBUGMENU* pdebugmenu);
void OnDebugmenuActive(DEBUGMENU* pdebugmenu, int fActive);
void UpdateDebugmenuActive(DEBUGMENU* pdebugmenu, JOY* pjoy);
void OnDebugmenuReset(DEBUGMENU* pdebugmenu);
void DrawDebugMenu(DEBUGMENU* pdebugmenu);

void StartupUi();
void InitUi(UI* pui);
void PostUiLoad(UI* pui);
void SetUiUis(UI* pui, UIS uis);
void SetUiUPause(UI* pui, float uPause);
void PushUiActiveBlot(UI* pui, BLOT* pblot);
void PopUiActiveBlot(UI* pui);
void UpdateUi(UI* pui);

extern KEYCTR g_keyctr;
extern LIFECTR g_lifectr;
extern COINCTR g_coinctr;
extern GOLDCTR g_goldctr;
extern TRUNKCTR g_trunkctr;
extern CRUSHERCTR g_crusherctr;
extern LAPCTR g_lapctr;
extern BOOSTCTR g_boostctr;
extern PLACECTR g_placectr;
extern CLUECTR g_cluectr;
extern BOSSCTR g_bossctr;
extern TV g_tvRight;
extern PUFFCHARGERCTR g_puffchargectr;
extern TIMER g_timer;
extern NOTE g_note;
extern TITLE g_title;
extern TOTALS g_totals;
extern CALL g_call;
extern WMC g_wmc;
extern PROMPT g_prompt;
extern BINOC g_binoc;
extern LGNR g_lgnr;
extern SCORES g_scores;
extern LOGO g_logo;
extern ATTRACT g_attract;
extern LETTERBOX g_letterbox;
extern CREDIT g_credit;
extern DEBUGMENU g_debugmenu;
extern TV g_tvLeft;

extern UI g_ui;

extern bool fQuitGame;