#include "tail.h"

TAIL* NewTail()
{
	return new TAIL{};
}

void InitTail(TAIL* ptail)
{
	InitAlo(ptail);
    ptail->posTip = s_posTipDefault;
}

void SetTailRSpring(TAIL* ptail, float rSpring)
{
	ptail->rSpring = rSpring;
}

void* GetTailRSpring(TAIL* ptail)
{
	return &ptail->rSpring;
}

void SetTailRDampingWorld(TAIL* ptail, float rDampingWorld)
{
	ptail->rDampingWorld = rDampingWorld;
}

void* GetTailRDampingWorld(TAIL* ptail)
{
	return &ptail->rDampingWorld;
}

void SetTailRDampingLocal(TAIL* ptail, float rDampingLocal)
{
	ptail->rDampingLocal = rDampingLocal;
}

void* GetTailRDampingLocal(TAIL* ptail)
{
	return &ptail->rDampingLocal;
}

void SetTailRadMax(TAIL* ptail, float radMax)
{
	ptail->radMax = radMax;
}

void* GetTailRadMax(TAIL* ptail)
{
	return &ptail->radMax;
}

void SetTailCIterate(TAIL* ptail, int cIterate)
{
	ptail->cIterate = cIterate;
}

void* GetTailCIterate(TAIL* ptail)
{
	return &ptail->cIterate;
}

void SetTailDvGravity(TAIL* ptail, glm::vec3 dvGravity)
{
	ptail->dvGravity = dvGravity;
}

void* GetTailDvGravity(TAIL* ptail)
{
	return &ptail->dvGravity;
}

void SetTailFUnlockRot(TAIL* ptail, int fUnlockRot)
{
	ptail->fUnlockRot = fUnlockRot;
}

void* GetTailFUnlockRot(TAIL* ptail)
{
	return &ptail->fUnlockRot;
}

void SetTailPosTip(TAIL* ptail, glm::vec3 posTip)
{
	ptail->posTip = posTip;
}

void* GetTailPosTip(TAIL* ptail)
{
	return &ptail->posTip;
}

int GetTailSize()
{
	return sizeof(TAIL);
}

void CloneTail(TAIL* ptail, TAIL* ptailBase)
{
    CloneAlo(ptail, ptailBase);

    ptail->ptailFirst = ptailBase->ptailFirst;
    ptail->ctsd = ptailBase->ctsd;
    ptail->atsd = ptailBase->atsd;
    ptail->fReset = ptailBase->fReset;
    ptail->rSpring = ptailBase->rSpring;
    ptail->rDampingWorld = ptailBase->rDampingWorld;
    ptail->rDampingLocal = ptailBase->rDampingLocal;
    ptail->radMax = ptailBase->radMax;
    ptail->cIterate = ptailBase->cIterate;
    ptail->dvGravity = ptailBase->dvGravity;
    ptail->fUnlockRot = ptailBase->fUnlockRot;
    ptail->posTip = ptailBase->posTip;
}

void PostTailLoad(TAIL* ptail)
{
	PostAloLoad(ptail);

	if (ptail->paloParent != nullptr && FIsBasicDerivedFrom(ptail->paloParent, CID_TAIL) != 0)
		return;

	auto PtailChild = [](TAIL* ptailParent) -> TAIL*
	{
		for (LO* plo = ptailParent->dlChild.ploFirst; plo != nullptr; plo = plo->dleChild.ploNext)
		{
			if (FIsBasicDerivedFrom(plo, CID_TAIL) != 0)
				return reinterpret_cast<TAIL*>(plo);
		}

		return nullptr;
	};

	int ctail = 0;

	for (TAIL* ptailCur = ptail; ptailCur != nullptr; ptailCur = PtailChild(ptailCur))
		++ctail;

	if (ctail < 2 && ptail->fUnlockRot == 0)
	{
		ptail->ctsd = 0;
		ptail->atsd.clear();
		return;
	}

	// One TSD for each TAIL plus one final endpoint.
	ptail->ctsd = ctail + 1;
	ptail->atsd.clear();
	ptail->atsd.resize(ptail->ctsd);

	TAIL* ptailCur = ptail;

	for (int itsd = 0; itsd < ctail; ++itsd)
	{
		TSD& tsd = ptail->atsd[itsd];

		tsd.ptail = ptailCur;
		tsd.pos = ptailCur->xf.posWorld;
		tsd.s = glm::length(ptailCur->xf.pos);

		ptailCur->ptailFirst = ptail;

		TAIL* ptailNext = PtailChild(ptailCur);

		if (ptailNext != nullptr)
			tsd.posNext = ptailNext->xf.pos;
		else
			tsd.posNext = ptail->posTip;

		ptailCur = ptailNext;
	}

	TSD& tsdLast = ptail->atsd[ctail - 1];
	TSD& tsdEnd  = ptail->atsd[ctail];

	ConvertAloPos(tsdLast.ptail, nullptr, &tsdLast.posNext, &tsdEnd.pos);
	tsdEnd.s = tsdLast.s;
}

void UpdateTailConstraints(TAIL* ptail)
{
	UpdateAloConstraints(ptail);

	if (ptail->atsd.empty() || ptail->ctsd <= 1)
		return;

	const int itsdMic = ptail->fUnlockRot != 0 ? 1 : 2;
	const int itsdFirstRot = itsdMic - 1;

	if (ptail->ctsd <= itsdFirstRot || ptail->atsd[itsdFirstRot].ptail == nullptr)
		return;

	if (ptail->atsd[itsdFirstRot].ptail->pactRot != nullptr)
	{
		ptail->fReset = 1;
		return;
	}

	if (ptail->fReset != 0)
	{
		for (int i = 0; i < ptail->ctsd - 1; ++i)
		{
			TSD& tsd = ptail->atsd[i];

			tsd.pos = tsd.ptail->xf.posWorld;
			CalculateAloMovement(tsd.ptail, nullptr, tsd.pos, &tsd.v, nullptr, nullptr, nullptr);
		}

		TSD& tsdLast = ptail->atsd[ptail->ctsd - 2];
		TSD& tsdEnd = ptail->atsd[ptail->ctsd - 1];

		ConvertAloPos(tsdLast.ptail, nullptr, &tsdLast.posNext, &tsdEnd.pos);
		CalculateAloMovement(tsdLast.ptail, nullptr, tsdEnd.pos, &tsdEnd.v, nullptr, nullptr, nullptr);

		ptail->fReset = 0;
	}

	ptail->atsd[0].pos = ptail->xf.posWorld;
	CalculateAloMovement(ptail, nullptr, ptail->atsd[0].pos, &ptail->atsd[0].v, nullptr, nullptr, nullptr);

	if (ptail->fUnlockRot == 0)
	{
		ConvertAloPos(ptail, nullptr, &ptail->atsd[0].posNext, &ptail->atsd[1].pos);
		CalculateAloMovement(ptail, nullptr, ptail->atsd[1].pos, &ptail->atsd[1].v, nullptr, nullptr, nullptr);
	}

	for (int i = itsdMic; i < ptail->ctsd; ++i)
		ptail->atsd[i].v += ptail->dvGravity * g_clock.dt;

	if (ptail->cIterate > 0)
	{
		const float dtStep = g_clock.dt / static_cast<float>(ptail->cIterate);

		for (int iIterate = 0; iIterate < ptail->cIterate; ++iIterate)
		{
			glm::vec3 dposPrev;

			if (ptail->fUnlockRot == 0)
				dposPrev = ptail->atsd[1].pos - ptail->atsd[0].pos;
			else
			{
				dposPrev = ptail->matOrig * ptail->atsd[0].posNext;

				if (ptail->paloParent != nullptr)
					dposPrev = ptail->paloParent->xf.matWorld * dposPrev;
			}

			for (int i = itsdMic; i < ptail->ctsd; ++i)
			{
				TSD& tsdPrev = ptail->atsd[i - 1];
				TSD& tsd = ptail->atsd[i];

				glm::vec3 dpos = tsd.pos - tsdPrev.pos;
				const glm::vec3 dvLocal = tsd.v - tsdPrev.v;

				glm::vec3 normal;
				const float rad = RadBetweenVectors(&dpos, &dposPrev, &normal);

				if (rad > ptail->radMax)
				{
					glm::mat3 matLimit;
					LoadRotateMatrixRad(rad - ptail->radMax, &normal, &matLimit);

					dpos = matLimit * dpos;
					tsd.pos = tsdPrev.pos + dpos;
				}

				const glm::vec3 dvSpring = glm::cross(normal, dpos);
				const float rDampingLocal = std::min(1.0f, -ptail->rDampingLocal * dtStep);
				const float rDampingWorld = std::min(1.0f, -ptail->rDampingWorld * dtStep);

				tsd.v += dvLocal * rDampingLocal;
				tsd.v += tsd.v * rDampingWorld;
				tsd.v += dvSpring * (ptail->rSpring * rad * dtStep);
				tsd.pos += tsd.v * dtStep;

				dposPrev = dpos;
			}

			for (int i = itsdMic; i < ptail->ctsd; ++i)
			{
				TSD& tsdPrev = ptail->atsd[i - 1];
				TSD& tsd = ptail->atsd[i];

				const glm::vec3 dpos = tsd.pos - tsdPrev.pos;
				const float length = glm::length(dpos);
				const glm::vec3 normal = length >= 0.0001f ? dpos / length : g_normalX;

				tsd.pos = tsdPrev.pos + normal * tsd.s;

				const glm::vec3 vPrevNormal = normal * glm::dot(tsdPrev.v, normal);
				const glm::vec3 vCurrentTangent = tsd.v - normal * glm::dot(tsd.v, normal);

				tsd.v = vPrevNormal + vCurrentTangent;
			}
		}
	}

	glm::mat3 matPrev;

	if (ptail->fUnlockRot == 0)
		matPrev = ptail->xf.matWorld;
	else if (ptail->paloParent != nullptr)
		matPrev = ptail->paloParent->xf.matWorld;
	else
		matPrev = glm::mat3(1.0f);

	for (int i = itsdFirstRot; i < ptail->ctsd - 1; ++i)
	{
		TSD& tsd = ptail->atsd[i];
		TAIL* ptailCur = tsd.ptail;

		glm::vec3 dpos = ptail->atsd[i + 1].pos - tsd.pos;

		glm::mat3 matBase;

		if (ptailCur->palox == nullptr)
			matBase = matPrev * ptailCur->matOrig;
		else
		{
			matBase = matPrev;
			matBase *= ptailCur->palox->matPostRotation;
			matBase *= ptailCur->matOrig;
			matBase *= ptailCur->palox->matPreRotation;
		}

		glm::vec3 dposOrig = matBase * tsd.posNext;

		glm::mat3 matRotate;
		LoadRotateVectorMatrix(&dposOrig, &dpos, &matRotate);

		glm::mat3 matWorld = matRotate * matBase;
		NormalizeRotateMatrix3(&matWorld);

		ptailCur->xf.matWorld = matWorld;
		ptailCur->xf.mat = glm::transpose(matPrev) * matWorld;

		matPrev = matWorld;
	}

	TAIL* ptailMic = ptail->atsd[itsdFirstRot].ptail;

	if (ptailMic->pvtlo->pfnUpdateLoXfWorld != nullptr)
		ptailMic->pvtlo->pfnUpdateLoXfWorld(ptailMic);
}

void MatchTailOtherObject(TAIL* ptail, ALO* paloOther)
{
    if (ptail->ptailFirst == ptail)
        ptail->fReset = 1;

    MatchAloOtherObject(ptail, paloOther);
}

void DeleteTail(TAIL* ptail)
{
	delete ptail;
}

glm::vec3 s_posTipDefault = {100.0, 0.0, 0.0};