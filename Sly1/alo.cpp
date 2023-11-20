#include "alo.h"

std::vector<void*> allWorldObjs;
std::vector<ALO*> allSWAloObjs;

void* CreateAlo()
{
	ALO alo;
	return &alo;
}

void InitAlo(ALO* palo)
{
	//std::cout << palo << "\n";
	InitDl(&palo->dlChild, 0x78);
	InitDl(&palo->dlFreeze, 0xD0);

	InitLo(palo);
	palo->sCelBorderMRD = 2139095039;
	palo->sMRD = 2139095039;
	palo->grfzon = -1;

	InitDl(&palo->dlAct, 0x230);
}

void RemoveAloHierarchy(ALO *palo)
{
	DLI plo;

	// Loading objects child
	plo.m_pdl = &palo->dlChild;
	// Loading object's chile base offset to object child entry
	plo.m_ibDle = palo->dlChild.ibDle;

	plo.m_pdliNext = s_pdliFirst;

	// Storing parent object in global parent list
	s_pdliFirst = &plo;
	plo.m_ppv = (void**)plo.m_pdl;

	// Loading object header from RIPG object
	LO *LocalObject = (LO*)palo;

	while (true)
	{

		break;
	}
}

void OnAloAdd(ALO* palo)
{
	ALO *parentObject = palo->paloParent;

	if (parentObject == nullptr)
	{
		palo->paloRoot = palo;

		if (palo->fRealClock == 0)
		{
			AppendDlEntry(&palo->psw->dlMRD, palo);
			AppendDlEntry(&palo->psw->dlBusy, palo);

			if ((palo->pvtlo->grfcid & 2U) != 0)
				AppendDlEntry(&palo->psw->dlBusySo, palo);

			palo->paloFreezeRoot = palo;
			palo->dlFreeze.paloLast = palo;
			palo->dlFreeze.paloFirst = palo;
		}
		else
			AppendDlEntry(&palo->psw->dlMRDRealClock, palo);
	}

	//

	if (palo->pshadow != 0)
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow);
	//
	ResolveAlo(palo);
}

void OnAloRemove(ALO* palo)
{
}

void CloneAloHierarchy(ALO* palo)
{

}

void CloneAlo(ALO* palo)
{

}

void ResolveAlo(ALO* palo)
{
	if (palo->paloRoot != nullptr) 
		palo->paloRoot->cframeStatic = 0;
	
}

void AddAloHierarchy(ALO* palo)
{
	DLI plo;
	// Loads parent object child entry
	plo.m_pdl = &palo->dlChild;
	// Loads the object base offset to child entry list
	plo.m_ibDle = palo->dlChild.ibDle;
	// Loading the first parent of the object
	plo.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &plo;

	plo.m_ppv = (void**)plo.m_pdl;

	// Looping through all the child objects 
	while (true)
	{
		// Adding object to list
		palo->pvtlo->pfnOnLoAdd(palo);

		// Loading next object
		void *nextParentObject = *plo.m_ppv;
		
		plo.m_ppv = (void**) ((uintptr_t)nextParentObject + plo.m_ibDle);

		// If parent doesnt have a child break
		if (nextParentObject == nullptr) break;
	}

	s_pdliFirst = plo.m_pdliNext;
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	palo->xf.mat = pbis->ReadMatrix();
	palo->xf.pos = pbis->ReadVector();
	pbis->U8Read();
	pbis->U8Read();
	pbis->U8Read();
	palo->grfzon = pbis->U32Read();
	palo->sMRD = pbis->F32Read();
	palo->sCelBorderMRD = pbis->F32Read();
	palo->sRadiusRenderSelf = pbis->F32Read();
	palo->sRadiusRenderAll = pbis->F32Read();
	LoadOptionFromBrx(palo, pbis);
	LoadGlobsetFromBrx(&palo->globset, pbis, palo);
	LoadAloAloxFromBrx(pbis);
	
	palo->cposec = pbis->U8Read();
	palo->aposec.resize(palo->cposec);

	for (int i = 0; i < palo->cposec; i++)
	{
		palo->aposec[i].oid = (OID)pbis->S16Read();
		palo->aposec[i].agPoses.resize(palo->globset.cpose);

		for (int a = 0; a < palo->globset.cpose; a++)
			palo->aposec[i].agPoses[a] = pbis->F32Read();
	}

	LoadSwObjectsFromBrx(palo->psw, palo, pbis);
}

void LoadAloAloxFromBrx(CBinaryInputStream* pbis)
{
	uint32_t unk_0 = pbis->U32Read();

	if (unk_0 != 0)
	{
		int unk_1;

		if (unk_0 & 1)
		{
			pbis->ReadMatrix();
		}

		if (unk_0 & 2)
		{
			pbis->ReadMatrix();
		}

		if (((unk_0 & 0xc) != 0) && (unk_1 = pbis->S16Read() != -1))
		{

		}

		if ((unk_0 & 0x10) != 0)
			unk_1 = pbis->S16Read();

		if ((unk_0 & 0x20) != 0)
		{
			unk_1 = pbis->S16Read();
			pbis->ReadVector(); // Read Vector
			pbis->ReadVector(); // Read Vector
			pbis->F32Read();
		}

		if ((unk_0 & 0x40) != 0)
		{
			unk_1 = pbis->S16Read();
			unk_1 = pbis->S16Read();
		}

		if ((unk_0 & 0x80) != 0)
		{
			pbis->ReadVector();
			pbis->F32Read();

			if ((unk_0 & 0x100) != 0)
			{
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->ReadVector();
			}
		}

		if ((unk_0 & 0x200) != 0)
		{
			pbis->S16Read();
			pbis->S16Read();
		}

		if ((unk_0 & 0x400) != 0)
		{
			pbis->U8Read();
		}
	}
}

void UpdateAlo(ALO* palo, float dt)
{

}

void RenderAloAll(ALO* palo, CM* pcm, RO* proDup)
{

}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{

}

void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro)
{

}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{

}

void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro)
{

}

void DrawLo(ALO* palo)
{

	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
		{
			
		}
	}
}

void DeleteWorld(SW* psw)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

	for (int i = 0; i < allWorldObjs.size(); i++)
		DeleteObject((LO*)allWorldObjs[i]);

	delete(SW*)psw;
	
	allSWAloObjs.clear();
	allSWAloObjs.shrink_to_fit();
	allWorldObjs.clear();
	allWorldObjs.shrink_to_fit();

	g_psw = nullptr;
}

void DeleteModel(ALO *palo)
{
	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
		{
			glDeleteVertexArrays(1, &palo->globset.aglob[i].asubglob[a].VAO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].VBO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].VNO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].VCB);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].TCB);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].EBO);
		}
	}
}

void DeleteAlo(LO* palo)
{
	delete (ALO*)palo;
}
