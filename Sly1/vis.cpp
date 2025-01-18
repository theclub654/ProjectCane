#include "vis.h"

VISMAP* NewVismap()
{
	return new VISMAP{};
}

void InitVismap(VISMAP *pvismap)
{
	InitLo(pvismap);
	pvismap->psw->pvismap = pvismap;
}

int GetVismapSize()
{
	return sizeof(VISMAP);
}

void LoadVismapFromBrx(VISMAP *pvismap, CBinaryInputStream *pbis)
{
	pvismap->cvbsp = pbis->U16Read();
	pvismap->avbsp.resize(pvismap->cvbsp);
	//std::cout << std::hex << &pvismap->avbsp[0] <<"\n";

	for (int i = 0; i < pvismap->cvbsp; i++)
	{
		pvismap->avbsp[i].normal = pbis->ReadVector();
		pvismap->avbsp[i].gDot = pbis->F32Read();

		uint32_t grfneg = pbis->U32Read();

		if ((grfneg & 0x80000000) == 0)
		{
			pvismap->avbsp[i].pvbspNeg = &pvismap->avbsp[grfneg];
		}
		else
		{
			pvismap->avbsp[i].pvbspNeg = (VBSP*)grfneg;
			//std::cout << std::hex << grfneg<<"\n";
		}

		uint32_t grfpos = pbis->U32Read();

		if ((grfpos & 0x80000000) == 0)
		{
			pvismap->avbsp[i].pvbspPos = &pvismap->avbsp[grfpos];
		}
		else
		{
			pvismap->avbsp[i].pvbspPos = (VBSP*)grfpos;
			//std::cout << std::hex << grfpos <<"\n";
		}
	}
	//std::cout << std::hex << &pvismap->avbsp[0x1A5] << "\n";
	pvismap->cgrfzon = pbis->U16Read();
	pvismap->agrfzonOneHop.resize(pvismap->cgrfzon);

	for (int i = 0; i < pvismap->cgrfzon; i++)
		pvismap->agrfzonOneHop[i] = pbis->U32Read();

	LoadOptionsFromBrx(pvismap, pbis);
}

void ClipVismapSphereOneHop(VISMAP *pvismap, glm::vec3 *ppos, float sRadius, GRFZON *pgrfzon)
{
	if (pvismap == nullptr || pvismap->avbsp.size() == 0)
		*pgrfzon = 0xfffffff;
	else
	{
		*pgrfzon = 0;
		ClipVbspSphereOneHop(pvismap, &pvismap->avbsp[0], sRadius, ppos);
	}
}

void ClipVbspSphereOneHop(VISMAP* pvismap, VBSP* pvbsp, float sRadius, glm::vec3 *ppos)
{

}

void ClipVismapPointNoHop(VISMAP* pvismap, glm::vec3* ppos, GRFZON* pgrfzon)
{
	GRFZON grfzon{};
	uint32_t test{};
	uint32_t test1{};

	if (pvismap == nullptr || pvismap->avbsp.size() == 0)
		grfzon = 0xfffffff;
	else
	{
		VBSP* vbsp = &pvismap->avbsp[0];
		do
		{
			while (true)
			{
				float visible = glm::dot(*ppos, vbsp->normal);

				if (visible - vbsp->gDot < 0.0) // Checks to see if plane is in front of position or not
					break;

				vbsp = vbsp->pvbspPos;

				test1 = ((uint32_t)vbsp & 0x80000000);

				if (((uint32_t)vbsp & 0x80000000) != 0) // Check if pos node is empty
				{
					grfzon = (uint32_t)vbsp & 0x7fffffff;
					*pgrfzon = grfzon;
					return;
				}
			}

			vbsp = vbsp->pvbspNeg;
			grfzon = (uint32_t)vbsp & 0x7fffffff;

			test = ((uint32_t)vbsp & 0x80000000);

		} while (test == 0); // Checks if neg node is empty
	}

	// Storing the zone the position is currently in
	*pgrfzon = grfzon;
}

void DeleteVismap(VISMAP *pvismap)
{
	delete pvismap;
}
