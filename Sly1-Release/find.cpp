#include "find.h"
#include "proxy.h"
#include <cstdio>

DL* PdlFromSwOid(SW* psw, OID oid)
{
	return psw->adlHash + (oid * 0x95675 & 0x1ff);
}

void MatchSwObject(ALO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int* pcploMatch, LO** aplo, int* pcpaloBest)
{
	if (ploMatch == nullptr ||
		pcploMatch == nullptr ||
		pcpaloBest == nullptr)
	{
		return;
	}

	ALO* paloCurrent = nullptr;
	ALO* paloStop = nullptr;

	switch (static_cast<int>(grffsoMask))
	{
	case 1:
	{
		// A direct proxy-source object is not eligible here.
		if (ploMatch->ppxr != nullptr && fProxyMatch == 0)
			return;

		paloCurrent = ploMatch->paloParent;

		if (paloCurrent == nullptr)
			return;

		while (paloCurrent->cpaloFindSwObjects == 0)
		{
			// Do not cross an unrelated proxy boundary.
			if (paloCurrent->ppxr != nullptr)
				return;

			paloCurrent = paloCurrent->paloParent;

			if (paloCurrent == nullptr)
				return;
		}

		// The match must be directly below the context.
		if (paloCurrent->cpaloFindSwObjects != 1)
			return;

		break;
	}

	case 2:
	{
		if (ploMatch->ppxr != nullptr && fProxyMatch == 0)
			return;

		if (ploMatch->paloParent !=
			reinterpret_cast<ALO*>(ploContext))
		{
			return;
		}

		break;
	}

	case 3:
	{
		// This mode only accepts ALO-derived objects.
		if ((ploMatch->pvtlo->grfcid & 1U) == 0)
			return;

		ALO* paloMatch = static_cast<ALO*>(ploMatch);

		if (paloMatch->cpaloFindSwObjects == 0)
			return;

		break;
	}

	case 4:
	{
		if ((ploMatch->pvtlo->grfcid & 1U) != 0)
			paloCurrent = static_cast<ALO*>(ploMatch);
		else
			paloCurrent = ploMatch->paloParent;

		while (paloCurrent != nullptr)
		{
			const int depth =
				paloCurrent->cpaloFindSwObjects;

			if (depth != 0 && depth <= *pcpaloBest)
			{
				if (depth < *pcpaloBest)
				{
					*pcpaloBest = depth;
					*pcploMatch = 0;
				}

				goto ADD_MATCH;
			}

			if (paloCurrent == paloStop)
				return;

			PXR* ppxr = paloCurrent->ppxr.get();

			if (ppxr == nullptr)
			{
				paloCurrent = paloCurrent->paloParent;
				continue;
			}

			// A proxy instance supplied by CploFindSwObjects may
			// climb through its immediate parent normally.
			if (paloCurrent == ploMatch && fProxyMatch != 0)
			{
				paloCurrent = paloCurrent->paloParent;
				continue;
			}

			OID oidContextProxyRoot = OID_Nil;

			ALO* paloContext =
				reinterpret_cast<ALO*>(ploContext);

			while (paloContext != nullptr)
			{
				if (paloContext->ppxr != nullptr)
				{
					oidContextProxyRoot =
						paloContext->ppxr->oidProxyRoot;
					break;
				}

				paloContext = paloContext->paloParent;
			}

			if (ppxr->oidProxyRoot != oidContextProxyRoot)
				return;

			paloStop = paloCurrent->paloParent;
			paloCurrent = paloCurrent->paloParent;
		}

		// If another candidate already established a hierarchy
		// distance, this object is not an equally good match.
		if (*pcpaloBest != INT_MAX)
			return;

		break;
	}

	case 5:
		break;

	default:
		return;
	}

	ADD_MATCH:
	if (fIncludeRemoved == 0 &&
		FIsLoInWorld(ploMatch) == 0)
	{
		return;
	}

	const int matchIndex = *pcploMatch;

	if (matchIndex < cploMax && aplo != nullptr)
		aplo[matchIndex] = ploMatch;

	*pcploMatch = matchIndex + 1;
}

int CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo)
{
	if (psw == nullptr || oid == OID_Nil)
		return 0;

	const uint32_t flags = static_cast<uint32_t>(grffso);
	uint32_t grffsoMask = flags & 0xFFU;

	const int fIncludeRemoved = static_cast<int>(flags & 0x100U);
	const bool fLimitResult = (flags & 0x200U) == 0;

	ALO* paloContext = nullptr;
	int cpaloBest = INT_MAX;

	if (grffsoMask == 1 || grffsoMask == 3 || grffsoMask == 4)
	{
		if (ploContext != nullptr)
		{
			if ((ploContext->pvtlo->grfcid & 1U) != 0)
				paloContext = static_cast<ALO*>(ploContext);
			else
				paloContext = ploContext->paloParent;
		}

		int depth = 0;

		for (ALO* palo = paloContext; palo != nullptr; palo = palo->paloParent)
		{
			++depth;
			palo->cpaloFindSwObjects = depth & 0xF;
		}

		/*
		 * This reproduces the prototype's short-circuit expression:
		 *
		 * if (ploContext == nullptr)
		 * {
		 *     const bool fReturnEmpty = grffsoMask == 3;
		 *     grffsoMask = 5;
		 *
		 *     if (fReturnEmpty)
		 *         return 0;
		 * }
		 */
		if (ploContext == nullptr)
		{
			const bool fReturnEmpty = grffsoMask == 3;
			grffsoMask = 5;

			if (fReturnEmpty)
				return 0;
		}
	}

	int cploMatch = 0;
	DL* pdl = PdlFromSwOid(psw, oid);

	if (pdl != nullptr)
	{
		for (LO* ploMatch = pdl->ploFirst; ploMatch != nullptr; ploMatch = ploMatch->dleOid.ploNext)
		{
			if (ploMatch->oid != oid)
				continue;

			if ((ploMatch->pvtlo->grfcid & 0x100U) == 0)
			{
				/*
				 * Do not process a proxy-source object directly when its
				 * proxy-root OID equals the requested OID.
				 */
				if (ploMatch->ppxr != nullptr && ploMatch->ppxr->oidProxyRoot == oid)
					continue;

				MatchSwObject((ALO*)ploMatch, static_cast<GRFFSO>(grffsoMask), fIncludeRemoved, 0, ploContext, cploMax, &cploMatch, aplo, &cpaloBest);
			}
			else
			{
				PROXY* pproxy = static_cast<PROXY*>(ploMatch);

				for (PXR* ppxr = pproxy->dlProxyRoot.ppxrFirst; ppxr != nullptr; ppxr = ppxr->dleProxyRoot.ppxrNext)
				{
					if (ppxr->plo == nullptr)
						continue;

					MatchSwObject((ALO*)ppxr->plo, static_cast<GRFFSO>(grffsoMask), fIncludeRemoved, 1, ploContext, cploMax, &cploMatch, aplo, &cpaloBest);
				}
			}
		}
	}

	for (ALO* palo = paloContext; palo != nullptr; palo = palo->paloParent)
		palo->cpaloFindSwObjects = 0;

	if (fLimitResult && cploMatch > cploMax)
		cploMatch = cploMax;

	return cploMatch;
}

LO* PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, grffso | 0x200, oid, ploContext, 1, &plo);
	return plo;
}

LO* PloFindSwNearest(SW* psw, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, 0x204, oid, ploContext, 1, &plo);
	return plo;
}

LO* PloFindSwChild(SW* psw, OID oid, ALO* paloAncestor)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, 0x201, oid, paloAncestor, 1, &plo);
	return plo;
}

int FIsCidDerivedFrom(CID cid, CID cidAncestor)
{
	VT* current = (VT*)g_mpcidpvt[cid];
	while (current) {
		if (current->cid == cidAncestor) {
			return 1;
		}
		current = current->pvtSuper;
	}
	return 0;
}

int CploFindSwObjectsByClass(SW* psw, GRFFSO grffso, CID cid, ALO* ploContext, int cploMax, LO** aplo)
{
	if (cid == CID_Nil)
		return 0;

	const uint32_t grffsoMask = static_cast<uint32_t>(grffso) & 0xFFU;
	const int fIncludeRemoved = static_cast<uint32_t>(grffso) & 0x100U;
	const bool fUnlimitedCount = (static_cast<uint32_t>(grffso) & 0x200U) != 0;

	if (ploContext == nullptr && (static_cast<uint32_t>(grffso) & 5U) == 0)
		return 0;

	int cploMatch = 0;

	if (grffsoMask == 3)
	{
		LO* plo = ploContext;

		if (plo != nullptr && (plo->pvtlo->grfcid & 1U) == 0)
			plo = plo->paloParent;

		while (plo != nullptr)
		{
			if (FIsBasicDerivedFrom(plo, cid))
			{
				if (fIncludeRemoved != 0 || FIsLoInWorld(plo))
				{
					if (cploMatch < cploMax)
						aplo[cploMatch] = plo;

					++cploMatch;
				}
			}

			plo = plo->paloParent;
		}
	}
	else if (grffsoMask == 2 && fIncludeRemoved == 0)
	{
		if (ploContext == nullptr)
			return 0;

		if ((ploContext->pvtlo->grfcid & 1U) == 0)
			return 0;

		for (LO* plo = ploContext->dlChild.ploFirst; plo != nullptr; plo = plo->dleChild.ploNext)
		{
			if (!FIsBasicDerivedFrom(plo, cid))
				continue;

			if (cploMatch < cploMax)
				aplo[cploMatch] = plo;

			++cploMatch;
		}
	}
	else
	{
		ALO* paloContext = nullptr;
		int cpaloBest = INT_MAX;

		if (grffsoMask == 1 || grffsoMask == 3 || grffsoMask == 4)
		{
			paloContext = reinterpret_cast<ALO*>(ploContext);

			if (ploContext != nullptr && (ploContext->pvtlo->grfcid & 1U) == 0)
				paloContext = ploContext->paloParent;

			int depth = 0;

			for (ALO* palo = paloContext; palo != nullptr; palo = palo->paloParent)
			{
				++depth;
				palo->cpaloFindSwObjects = depth & 0xF;
			}
		}

		CID cidCurrent = cid;

		while (static_cast<uint32_t>(cidCurrent) < 162U && FIsCidDerivedFrom(cidCurrent, cid))
		{
			LO* plo = psw->aploCidHead[cidCurrent];
			cidCurrent = static_cast<CID>(cidCurrent + CID_LO);

			while (plo != nullptr)
			{
				MatchSwObject((ALO*)plo, grffsoMask, fIncludeRemoved, 0, ploContext, cploMax, &cploMatch, aplo, &cpaloBest);
				plo = plo->ploCidNext;
			}
		}

		for (ALO* palo = paloContext; palo != nullptr; palo = palo->paloParent)
			palo->cpaloFindSwObjects = 0;
	}

	if (!fUnlimitedCount && cploMatch > cploMax)
		cploMatch = cploMax;

	return cploMatch;
}

LO* PloFindSwObjectByClass(SW* psw, GRFFSO grffso, CID cid, LO* ploContext)
{
	LO* result = nullptr;

	// Search for the first matching object in the scene world (SW) with specified class ID and context
	CploFindSwObjectsByClass(psw, grffso | 0x200, cid, (ALO*)ploContext, 1, &result);

	return result;
}

ALO* PaloFindLoCommonParent(LO* plo, LO* ploOther)
{
	for (ALO* pAlo = reinterpret_cast<ALO*>(plo); pAlo; pAlo = pAlo->paloParent)
	{
		for (ALO* pOther = reinterpret_cast<ALO*>(ploOther); pOther; pOther = pOther->paloParent)
		{
			if (pAlo == pOther)
				return pAlo;
		}
	}

	return nullptr;
}
