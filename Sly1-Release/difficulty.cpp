#include "difficulty.h"

void OnDifficultyWorldPreLoad(DIFFICULTY* pdifficulty)
{
	pdifficulty->ccoinBrkPoorMax = 3;
	pdifficulty->ccoinBrkRichLowMax = 0x14;
	pdifficulty->uBrkRichLow = 0.4;
	pdifficulty->ccoinBrkRichHighMin = 0x46;
	pdifficulty->uBrkRichHigh = 0.15;
	pdifficulty->ccoinNpcMin = 4;
	pdifficulty->ccoinNpcMax = 6;
	pdifficulty->uSuckCharmLow = 0.399;
	pdifficulty->ccharmLow = 1;
	pdifficulty->uSuckCharmHigh = 0.799;
	pdifficulty->ccharmHigh = 2;
	pdifficulty->duSuckDeath = 0.1;
	pdifficulty->duSuckChkpntTrigger = -0.8;
	pdifficulty->dps = DPS_Normal;
	pdifficulty->ccoinBrkRichMin = 4;
	pdifficulty->ccoinBrkRichMax = 6;
	pdifficulty->ccoinBrkPoorMin = 1;
	pdifficulty->ccoinMiniNpcMin = 1;
	pdifficulty->ccoinMiniNpcMax = 2;
	pdifficulty->clifeMaxExtraCcoin = 0;
	pdifficulty->duSuckChkpntRetrigger = 0.0;
}
