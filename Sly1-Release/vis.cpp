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
	if (pvismap == nullptr || pvismap->avbsp.size() == 0)
		*pgrfzon = 0xfffffff;
	else
	{
		*pgrfzon = 0;
		ClipVbspSphereOneHop(pvismap, &pvismap->avbsp[0], sRadius, ppos);
	}
}

void ClipVbspSphereOneHop(VISMAP* pvismap, VBSP* pvbsp, float sRadius, glm::vec3* ppos)
{

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
