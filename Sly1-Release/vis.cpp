#include "vis.h"

VISMAP* NewVismap()
{
	return new VISMAP{};
}

void InitVismap(VISMAP* pvismap)
{
	InitLo(pvismap);
	pvismap->psw->pvismap = pvismap;
}

int GetVismapSize()
{
	return sizeof(VISMAP);
}

void LoadVismapFromBrx(VISMAP* pvismap, CBinaryInputStream* pbis)
{
	pvismap->cvbsp = pbis->U16Read();
	pvismap->avbsp.resize(pvismap->cvbsp);

	for (int i = 0; i < pvismap->cvbsp; i++) {
		auto& node = pvismap->avbsp[i];
		node.normal = pbis->ReadVector();
		node.gDot = pbis->F32Read();

		// NEGATIVE CHILD
		uint32_t grfneg = pbis->U32Read();
		if (grfneg & 0x80000000) {
			node.bNegIsLeaf = true;
			node.grfzonNeg = grfneg & 0x7FFFFFFF;
		}
		else {
			node.pvbspNeg = &pvismap->avbsp[grfneg];
		}

		// POSITIVE CHILD
		uint32_t grfpos = pbis->U32Read();
		if (grfpos & 0x80000000) {
			node.bPosIsLeaf = true;
			node.grfzonPos = grfpos & 0x7FFFFFFF;
		}
		else {
			node.pvbspPos = &pvismap->avbsp[grfpos];
		}
	}

	pvismap->cgrfzon = pbis->U16Read();
	pvismap->agrfzonOneHop.resize(pvismap->cgrfzon);

	for (int i = 0; i < pvismap->cgrfzon; i++)
		pvismap->agrfzonOneHop[i] = pbis->U32Read();

	LoadOptionsFromBrx(pvismap, pbis);
}

void ClipVismapSphereOneHop(VISMAP* pvismap, glm::vec3* ppos, float sRadius, GRFZON* pgrfzon)
{
	constexpr GRFZON kFallback = 0x0FFFFFFF;

	if (!pgrfzon)
		return;

	// Match original behavior: if vismap/tree missing -> fallback
	if (!pvismap || pvismap->avbsp.empty() || !ppos)
	{
		*pgrfzon = kFallback;
		return;
	}

	// Original sets output to 0 before accumulating ORs
	*pgrfzon = 0;

	// Root is first node
	VBSP* root = &pvismap->avbsp[0];

	// Walk/accumulate sphere visibility
	ClipVbspSphereOneHop(pvismap, root, *ppos, sRadius, pgrfzon);
}

GRFZON GrfzonOneHop(VISMAP* pvismap, GRFZON leafMask)
{
	if (!pvismap) return 0;

	const uint32_t uVar2 = (uint32_t)leafMask; // already low-31 in your loader
	if (uVar2 == 0)
		return 0;

	uint32_t acc = 0xFFFFFFFFu;

	// Original loops up to cgrfzon
	const int count = pvismap->cgrfzon;
	for (int i = 0; i < count; ++i)
	{
		if ((uVar2 >> (i & 31)) & 1u)
			acc &= (uint32_t)pvismap->agrfzonOneHop[i];
	}

	return (GRFZON)(acc | uVar2);
}

void ClipVbspSphereOneHop(VISMAP* pvismap, VBSP* pvbsp, const glm::vec3& pos, float sRadius, GRFZON* pgrfzon)
{
	if (!pgrfzon) return;
	if (!pvismap || !pvbsp) return;

	// Matches original behavior: OR results into *pgrfzon (caller can init to 0)
	for (;;)
	{
		const float d = glm::dot(pos, pvbsp->normal) - pvbsp->gDot;

		// Equivalent to: if (-sRadius <= d)
		if (d >= -sRadius)
		{
			// Sphere intersects plane band: d <= +radius
			if (d <= sRadius)
			{
				// --- Process POS side first (recursive or leaf), OR result ---
				if (pvbsp->bPosIsLeaf)
				{
					*pgrfzon |= GrfzonOneHop(pvismap, pvbsp->grfzonPos);
				}
				else
				{
					if (pvbsp->pvbspPos)
						ClipVbspSphereOneHop(pvismap, pvbsp->pvbspPos, pos, sRadius, pgrfzon);
					else
						return; // bad data / missing link
				}

				// --- Then continue down NEG side iteratively ---
				if (pvbsp->bNegIsLeaf)
				{
					*pgrfzon |= GrfzonOneHop(pvismap, pvbsp->grfzonNeg);
					return;
				}

				if (!pvbsp->pvbspNeg) return;
				pvbsp = pvbsp->pvbspNeg;
				continue;
			}

			// Fully on POS side (d > radius)
			if (pvbsp->bPosIsLeaf)
			{
				*pgrfzon |= GrfzonOneHop(pvismap, pvbsp->grfzonPos);
				return;
			}

			if (!pvbsp->pvbspPos) return;
			pvbsp = pvbsp->pvbspPos;
			continue;
		}
		else
		{
			// Fully on NEG side (d < -radius)
			if (pvbsp->bNegIsLeaf)
			{
				*pgrfzon |= GrfzonOneHop(pvismap, pvbsp->grfzonNeg);
				return;
			}

			if (!pvbsp->pvbspNeg) return;
			pvbsp = pvbsp->pvbspNeg;
			continue;
		}
	}
}

void ClipVismapPointNoHop(VISMAP* pvismap, glm::vec3* ppos, GRFZON* pgrfzon)
{
	if (!pvismap || pvismap->avbsp.empty()) {
		*pgrfzon = 0x0FFFFFFF;
		return;
	}

	VBSP* vbsp = &pvismap->avbsp[0];

	while (true) {
		float visible = glm::dot(*ppos, vbsp->normal);

		if (visible - vbsp->gDot >= 0.0f) {
			if (vbsp->bPosIsLeaf) {
				*pgrfzon = vbsp->grfzonPos;
				return;
			}
			vbsp = vbsp->pvbspPos;
		}
		else {
			if (vbsp->bNegIsLeaf) {
				*pgrfzon = vbsp->grfzonNeg;
				return;
			}
			vbsp = vbsp->pvbspNeg;
		}
	}
}

void DeleteVismap(VISMAP* pvismap)
{
	delete pvismap;
}
