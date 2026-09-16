#include "difficulty.h"
#include "chkpnt.h"
#include "game.h"
#include "coin.h"

DIFFICULTY g_difficulty{};

DIFFICULTY* PdifficultyEnsureSw()
{
	return &g_difficulty;
}

void OnDifficultyGameLoad(DIFFICULTY* pdifficulty)
{
	memset(pdifficulty, 0, sizeof(*pdifficulty));
}

void OnDifficultyWorldPreLoad(DIFFICULTY* pdifficulty)
{
	pdifficulty->ccoinRichMin = 4;
	pdifficulty->ccoinRichMax = 6;
	pdifficulty->ccoinPoorMin = 1;
	pdifficulty->ccoinPoorMax = 3;

	if ((g_pgsCur->gameWorldCur == GAMEWORLD_Intro) || (g_pgsCur->gameWorldCur == GAMEWORLD_Clockwerk) || (g_pgsCur->worldLevelCur == WORLDLEVEL_Hub))
		pdifficulty->pdifficultyconfig = &g_difficultyEasy;
	else if ((g_plsCur->grfls & FLS_KeyCollected) == 0)
		pdifficulty->pdifficultyconfig = &g_difficultyMedium;
	else
		pdifficulty->pdifficultyconfig = &g_difficultyHard;

	ChangeSuck(pdifficulty, g_plsCur->uSuck);

	DIFFICULTYCONFIG* pconfig = pdifficulty->pdifficultyconfig;

	auto InterpolateRichValue = [](int valueNormal, int valueHard, float uSuck)
	{
		if (valueNormal < valueHard)
			return valueNormal + static_cast<int>(uSuck * (static_cast<float>(valueHard) + 0.999f - static_cast<float>(valueNormal)));

		if (valueNormal > valueHard)
			return valueHard + static_cast<int>((1.0f - uSuck) * (static_cast<float>(valueNormal) + 0.999f - static_cast<float>(valueHard)));

		return valueNormal;
	};

	int uRichLowMin = InterpolateRichValue(pconfig->uRichLowMin, pconfig->uRichLowMinHard, g_plsCur->uSuck);
	int uRichLowMax = InterpolateRichValue(pconfig->uRichLowMax, pconfig->uRichLowMaxHard, g_plsCur->uSuck);

	pdifficulty->uRichLow = NRandInRange(uRichLowMin, uRichLowMax);

	DIFFICULTYRICH* prich = pconfig->aRichHighRange;

	for (int i = 1; i < pconfig->cRichHighRange && g_plsCur->uSuckRaw < prich->uSuckRaw; ++i)
		prich = &pconfig->aRichHighRange[i];

	pdifficulty->uRichHigh = NRandInRange(prich->uRichMin, prich->uRichMax);
	pdifficulty->dps = DPS_Normal;
}

void OnDifficultyWorldPostLoad(DIFFICULTY* pdifficulty)
{
	DIFFICULTYCONFIG* pconfig = pdifficulty->pdifficultyconfig;

	if ((g_transition.m_grftrans & 8) == 0)
	{
		float uSuck = GLimitLm(&pconfig->lmSuck, g_plsCur->uSuck);
		ChangeSuck(pdifficulty, uSuck);
	}
	else if (g_plsCur->uSuck >= pconfig->uSuckCharmLow)
	{
		int ccharm;

		if (g_plsCur->uSuck < pconfig->uSuckCharmHigh)
			ccharm = pconfig->ccharmLow;
		else
			ccharm = pconfig->ccharmHigh;

		if (g_pgsCur->ccharm < ccharm)
		{
			SetCcharm(ccharm);
			pdifficulty->dps = DPS_GivenCharms;
		}
	}

	RemoveSwExtraneousCharms(g_psw);
}

void OnDifficultyPlayerDeath(DIFFICULTY* pdifficulty, float scale)
{
	DIFFICULTYCONFIG* pconfig = pdifficulty->pdifficultyconfig;

	ChangeSuck(pdifficulty, g_plsCur->uSuck + scale * pconfig->duSuckDeathImmediate);

	float duSuck;

	if (g_pgsCur->clife < 0)
		duSuck = pconfig->duSuckGameOver;
	else
	{
		duSuck = pconfig->duSuckDeath;

		if (g_pgsCur->clife <= pconfig->clifeLow)
			duSuck += pconfig->duSuckLowLife;
	}

	g_plsCur->uSuckRaw = GLimitLm(&g_lmZeroOne, g_plsCur->uSuckRaw + scale * duSuck);
}

void OnDifficultyTriggerCheckpoint(DIFFICULTY* pdifficulty, CHKPNT* pchkpnt)
{
	DIFFICULTYCONFIG* pconfig = pdifficulty->pdifficultyconfig;
	float duSuck = pconfig->duSuckChkpntNew;

	if (pchkpnt != nullptr)
	{
		int ibitChkpnt = pchkpnt->ibitChkpnt;

		if (ibitChkpnt >= 0)
		{
			int ibyte = ibitChkpnt >> 3;
			uint8_t mask = static_cast<uint8_t>(1u << (ibitChkpnt & 7));

			if ((g_plsCur->abitChkpnt.m_ab[ibyte] & mask) != 0)
				duSuck = pconfig->duSuckChkpntRepeat;
		}
	}

	ChangeSuck(pdifficulty, g_plsCur->uSuck + duSuck);

	if (pchkpnt != nullptr)
	{
		int ibitChkpnt = pchkpnt->ibitChkpnt;

		if (ibitChkpnt >= 0)
		{
			int ibyte = ibitChkpnt >> 3;
			uint8_t mask = static_cast<uint8_t>(1u << (ibitChkpnt & 7));
			uint8_t& bits = g_plsCur->abitChkpnt.m_ab[ibyte];

			if ((bits & mask) == 0)
			{
				bits |= mask;
				++g_plsCur->abitChkpnt.m_cbitSet;
			}
		}
	}
}

void OnDifficultyCollectKey(DIFFICULTY* pdifficulty)
{
	ChangeSuck(pdifficulty, 0.0);

	g_plsCur->uSuckRaw = 0.0f;
	g_plsCur->abitChkpnt.m_cbitSet = 0;
	memset(g_plsCur->abitChkpnt.m_ab, 0, sizeof(g_plsCur->abitChkpnt.m_ab));
}

void OnDifficultyBreak(DIFFICULTY* pdifficulty, glm::vec3* ppos, int ccoin, int fNoDifficultyUpdate)
{
	DIFFICULTYCONFIG* pconfig = pdifficulty->pdifficultyconfig;
	bool fAwardSpecialPrize = false;

	auto InterpolateRichValue = [](int valueNormal, int valueHard, float uSuck)
	{
		if (valueNormal < valueHard)
			return valueNormal + static_cast<int>(uSuck * (static_cast<float>(valueHard) + 0.999f - static_cast<float>(valueNormal)));

		if (valueNormal > valueHard)
			return valueHard + static_cast<int>((1.0f - uSuck) * (static_cast<float>(valueNormal) + 0.999f - static_cast<float>(valueHard)));

		return valueNormal;
	};

	auto FindRichRange = [](DIFFICULTYCONFIG* pconfig, float uSuckRaw) -> DIFFICULTYRICH*
	{
		DIFFICULTYRICH* prich = pconfig->aRichHighRange;

		for (int i = 1; i < pconfig->cRichHighRange && uSuckRaw < prich->uSuckRaw; ++i)
			prich = &pconfig->aRichHighRange[i];

		return prich;
	};

	if (g_psw->fDprizeEnabled != 0 && ccoin != 0)
	{
		if (!fNoDifficultyUpdate)
		{
			--pdifficulty->uRichHigh;
			--pdifficulty->uRichLow;
		}

		if (ccoin < 1)
		{
			if (pdifficulty->uRichHigh < 0)
			{
				fAwardSpecialPrize = g_plsCur->uSuckRaw != 0.0f;

				if (fAwardSpecialPrize)
					g_plsCur->uSuckRaw = GLimitLm(&g_lmZeroOne, g_plsCur->uSuckRaw + pconfig->duSuckRichHigh);

				DIFFICULTYRICH* prich = FindRichRange(pconfig, g_plsCur->uSuckRaw);
				pdifficulty->uRichHigh = NRandInRange(prich->uRichMin, prich->uRichMax);
			}

			if (!fNoDifficultyUpdate)
			{
				for (int i = 0; i < pconfig->cRichHighRange - 1; ++i)
				{
					DIFFICULTYRICH* prich = &pconfig->aRichHighRange[i];

					if (prich->uSuckRaw <= g_plsCur->uSuckRaw)
					{
						int uRichHighMax = NRandInRange(prich->uRichMin, prich->uRichMax);
						pdifficulty->uRichHigh = std::min(pdifficulty->uRichHigh, uRichHighMax);
						break;
					}
				}
			}

			if (fAwardSpecialPrize)
				CpdprizeAttractSwDprizes(g_psw, CID_LIFETKN, ppos, 1, nullptr);
			else
			{
				int ccoinMin;
				int ccoinMax;

				if (pdifficulty->uRichLow < 0)
				{
					int uRichLowMin = InterpolateRichValue(pconfig->uRichLowMin, pconfig->uRichLowMinHard, g_plsCur->uSuck);
					int uRichLowMax = InterpolateRichValue(pconfig->uRichLowMax, pconfig->uRichLowMaxHard, g_plsCur->uSuck);

					pdifficulty->uRichLow = NRandInRange(uRichLowMin, uRichLowMax);

					ccoinMin = pdifficulty->ccoinRichMin;
					ccoinMax = pdifficulty->ccoinRichMax;
				}
				else
				{
					ccoinMin = pdifficulty->ccoinPoorMin;
					ccoinMax = pdifficulty->ccoinPoorMax;
				}

				int ccoinDrop = NRandInRange(ccoinMin, ccoinMax);

				if (ccoinDrop != 0)
					CpdprizeAttractSwDprizes(g_psw, CID_COIN, ppos, ccoinDrop, nullptr);
			}
		}
		else
			CpdprizeAttractSwDprizes(g_psw, CID_COIN, ppos, ccoin, nullptr);
	}

	if (!fNoDifficultyUpdate)
		g_plsCur->uSuckRaw = GLimitLm(&g_lmZeroOne, g_plsCur->uSuckRaw + pconfig->duSuckBreak);
}

void ChangeSuck(DIFFICULTY* pdifficulty, float uSuck)
{
	g_plsCur->uSuck = GLimitLm(&pdifficulty->pdifficultyconfig->lmSuck, uSuck);
}

DIFFICULTYRICH g_aDifficultyRich[] =
{
	{ 0.8f, 2, 5 }
};

constexpr int g_cDifficultyRich =
	static_cast<int>(sizeof(g_aDifficultyRich) / sizeof(g_aDifficultyRich[0]));

DIFFICULTYCONFIG g_difficultyEasy =
{
	/* lmSuck */                  { 0.0f, 1.0f },
	/* lmSuckWorldLoad */         { 0.0f, 0.0f },

	/* duSuckDeathImmediate */     0.1f,
	/* duSuckChkpntNew */         -0.8f,
	/* duSuckChkpntRepeat */       0.0f,

	/* uRichLowMin */              4,
	/* uRichLowMax */              6,
	/* uRichLowMinHard */          0,
	/* uRichLowMaxHard */          2,

	/* uSuckCharmLow */            0.399f,
	/* ccharmLow */                1,
	/* uSuckCharmHigh */           0.799f,
	/* ccharmHigh */               2,

	/* duSuckDeath */              0.075f,
	/* duSuckGameOver */           0.25f,
	/* field_0x44 */              -0.2f,
	/* field_0x48 */              -0.003f,

	/* clifeLow */                 2,
	/* duSuckLowLife */            0.075f,

	/* cRichHighRange */           g_cDifficultyRich,
	/* aRichHighRange */           g_aDifficultyRich
};

DIFFICULTYCONFIG g_difficultyMedium =
{
	/* lmSuck */                  { 0.0f, 1.0f },
	/* lmSuckWorldLoad */         { 0.0f, 0.6f },

	/* duSuckDeathImmediate */     0.1f,
	/* duSuckChkpntNew */         -0.8f,
	/* duSuckChkpntRepeat */       0.0f,

	/* uRichLowMin */              4,
	/* uRichLowMax */              6,
	/* uRichLowMinHard */          0,
	/* uRichLowMaxHard */          2,

	/* uSuckCharmLow */            0.399f,
	/* ccharmLow */                1,
	/* uSuckCharmHigh */           0.799f,
	/* ccharmHigh */               2,

	/* duSuckDeath */              0.075f,
	/* duSuckGameOver */           0.25f,
	/* field_0x44 */              -0.2f,
	/* field_0x48 */              -0.003f,

	/* clifeLow */                 2,
	/* duSuckLowLife */            0.075f,

	/* cRichHighRange */           g_cDifficultyRich,
	/* aRichHighRange */           g_aDifficultyRich
};

DIFFICULTYCONFIG g_difficultyHard =
{
	/* lmSuck */                  { 0.0f, 0.0f },
	/* lmSuckWorldLoad */         { 0.0f, 0.2f },

	/* duSuckDeathImmediate */     0.0f,
	/* duSuckChkpntNew */          0.0f,
	/* duSuckChkpntRepeat */       0.0f,

	/* uRichLowMin */              4,
	/* uRichLowMax */              6,
	/* uRichLowMinHard */          0,
	/* uRichLowMaxHard */          2,

	/* uSuckCharmLow */            0.399f,
	/* ccharmLow */                0,
	/* uSuckCharmHigh */           0.799f,
	/* ccharmHigh */               0,

	/* duSuckDeath */              0.075f,
	/* duSuckGameOver */           0.25f,
	/* field_0x44 */              -0.2f,
	/* field_0x48 */              -0.003f,

	/* clifeLow */                 2,
	/* duSuckLowLife */            0.075f,

	/* cRichHighRange */           g_cDifficultyRich,
	/* aRichHighRange */           g_aDifficultyRich
};
