#include "fader.h"

void InitSwFaderDl(SW* psw)
{
	InitDl(&psw->dlFader, offsetof(FADER, dleFader));
}

void InitSwRealClockFader(SW* psw)
{
	InitDl(&psw->dlRealClockFader, offsetof(FADER, dleFader));
}

std::shared_ptr <FADER> PfaderNew(ALO* palo)
{
    auto pfader = std::make_shared<FADER>();

    pfader->palo = palo;

    if (palo->fRealClock == 0) {
        AppendDlEntry(&g_psw->dlFader, pfader.get());
    }
    else {
        AppendDlEntry(&g_psw->dlRealClockFader, pfader.get());
    }

    return pfader;
}

void UpdateFader(FADER* pfader, float dt)
{
	float uAlphaOld = pfader->uAlpha;

	if (pfader->duAlpha != 0.0f && !FIsLoInWorld(pfader->palo)) {
		RemoveFader(pfader);
		return;
	}

	pfader->uAlpha += pfader->duAlpha * dt;

	if (pfader->uAlpha <= 0.0f) {
		if (uAlphaOld > 0.0f) {
			pfader->palo->pvtlo->pfnRemoveLo(pfader->palo);
			RemoveFader(pfader);
		}

		return;
	}

	if (pfader->uAlpha >= 1.0f && uAlphaOld < 1.0f)
		RemoveFader(pfader);
}

void RemoveFader(FADER* pfader)
{
	if (pfader->palo->fRealClock)
		RemoveDlEntry(&g_psw->dlRealClockFader, pfader);
	else
		RemoveDlEntry(&g_psw->dlFader, pfader);

	pfader->palo->pfader.reset();
}
