#include "jlo.h"

JLOVOL* NewJlovol()
{
	return new JLOVOL{};
}

void InitJlovol(JLOVOL* pjlovol)
{
	InitVolbtn(pjlovol);
}

int GetJlovolSize()
{
	return sizeof(JLOVOL);
}

void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase)
{
	CloneVolbtn(pjlovol, pjlovolBase);

	pjlovol->oidLand = pjlovolBase->oidLand;
	pjlovol->ppntLand = pjlovolBase->ppntLand;
	pjlovol->oidJloc = pjlovolBase->oidJloc;
	pjlovol->pjloc = pjlovolBase->pjloc;
	pjlovol->dleJlo = pjlovolBase->dleJlo;
}

void DeleteJlovol(JLOVOL* pjlovol)
{
	delete pjlovol;
}

JLO* NewJlo()
{
	return new JLO{};
}

void InitJlo(JLO* pjlo)
{
	InitSo(pjlo);
	InitDl(&pjlo->dlJlovol, offsetof(JLOVOL, dleJlo));
}

int GetJloSize()
{
	return sizeof(JLO);
}

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjlo, pbis);
}

void CloneJlo(JLO* pjlo, JLO* pjloBase)
{
	CloneSo(pjlo, pjloBase);

	pjlo->psm = pjloBase->psm;
	pjlo->psma = pjloBase->psma;
	pjlo->pjlovolCur = pjloBase->pjlovolCur;
	pjlo->dlJlovol = pjloBase->dlJlovol;
	pjlo->ppntFeet = pjloBase->ppntFeet;
	pjlo->dzFeet = pjloBase->dzFeet;
	pjlo->prwm = pjloBase->prwm;
	pjlo->tFireNext = pjloBase->tFireNext;
	pjlo->oidJlovolStart = pjloBase->oidJlovolStart;
	pjlo->paloReticle = pjloBase->paloReticle;
	pjlo->paloTracer = pjloBase->paloTracer;
	pjlo->ppntTracerStart = pjloBase->ppntTracerStart;
	pjlo->pasegReticlePop = pjloBase->pasegReticlePop;
	pjlo->pasegReticleConverge = pjloBase->pasegReticleConverge;
	pjlo->pxfmTarget = pjloBase->pxfmTarget;
	pjlo->pxfmStatic = pjloBase->pxfmStatic;
	pjlo->jlos = pjloBase->jlos;
	pjlo->tJlos = pjloBase->tJlos;
	pjlo->radTarget = pjloBase->radTarget;
	pjlo->dtLand = pjloBase->dtLand;
	pjlo->tLand = pjloBase->tLand;
	pjlo->tStick = pjloBase->tStick;
	pjlo->smpSpin = pjloBase->smpSpin;
	pjlo->pexc = pjloBase->pexc;
}

void DeleteJlo(JLO *pjlo)
{
	delete pjlo;
}

JLOC* NewJloc()
{
	return new JLOC{};
}

void InitJloc(JLOC* pjloc)
{
	InitAlo(pjloc);
}

int GetJlocSize()
{
	return sizeof(JLOC);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);
}

void CloneJloc(JLOC* pjloc, JLOC* pjlocBase)
{
	CloneAlo(pjloc, pjlocBase);

	pjloc->cpxfm = pjlocBase->cpxfm;
	for (int i = 0; i < pjloc->cpxfm; ++i)
	{
		pjloc->apxfm[i] = pjlocBase->apxfm[i];
	}

	pjloc->jlock = pjlocBase->jlock;
	pjloc->jlomk = pjlocBase->jlomk;
	pjloc->dtFire = pjlocBase->dtFire;
	pjloc->dtMissile = pjlocBase->dtMissile;
	pjloc->rAccelTime = pjlocBase->rAccelTime;
	pjloc->rMissileSpeed = pjlocBase->rMissileSpeed;
	pjloc->radTilt = pjlocBase->radTilt;
	pjloc->svLaunch = pjlocBase->svLaunch;
	pjloc->svTarget = pjlocBase->svTarget;
}

void DeleteJloc(JLOC* pjloc)
{
	delete pjloc;
}
