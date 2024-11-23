#include "alo.h"
extern std::vector<ALO*> allSWAloObjs;

DLI* s_pdliFirst = nullptr;

ALO* NewAlo()
{
	return new ALO{};
}

void InitAlo(ALO* palo)
{
	InitDl(&palo->dlChild,  offsetof(LO, dleChild));
	InitDl(&palo->dlFreeze, offsetof(ALO, dleFreeze));

	if (palo->paloParent == nullptr)
		*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff0ffffff | 0xa000000;
	else
		*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff0ffffff | 0x1000000;

	InitLo(palo);

	byte value = 0xFF;
	byte value1 = 0x0;
	memcpy((char*)&palo->bitfield + 1, &value,  sizeof(byte));
	memcpy((char*)&palo->bitfield + 2, &value,  sizeof(byte));
	memcpy((char*)&palo->bitfield + 0, &value1, sizeof(byte));

	palo->sCelBorderMRD = 2139095039;
	palo->sMRD = 2139095039;
	palo->grfzon = -1;
	palo->xf.mat = glm::identity<glm::mat3>();
	palo->xf.matWorld = glm::identity<glm::mat3>();
	palo->matOrig = glm::identity<glm::mat3>();
	
	InitDl(&palo->dlAct, offsetof(ACT, dleAlo));

	if (palo->pvtlo->cid != CID_LIGHT)
		allSWAloObjs.push_back(palo);
}

void RemoveAloHierarchy(ALO *palo)
{
	DLI dliChild{};

	dliChild.m_pdl = &palo->dlChild;

	dliChild.m_ibDle = palo->dlChild.ibDle;

	dliChild.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &dliChild;
	dliChild.m_ppv = (void**)(uintptr_t)dliChild.m_pdl;

	while (true)
	{
		palo->pvtlo->pfnOnLoRemove(palo);

		// Loading next object
		void* nextParentObject = *dliChild.m_ppv;

		dliChild.m_ppv = (void**)((uintptr_t)nextParentObject + dliChild.m_ibDle);

		// If parent doesnt have a child break
		if (nextParentObject == nullptr) break;
	}

	s_pdliFirst = dliChild.m_pdliNext;
}

void OnAloAdd(ALO* palo)
{
	if (palo->paloParent == nullptr)
	{
		palo->paloRoot = palo;

		if (palo->fRealClock == 0)
		{
			AppendDlEntry(&palo->psw->dlMRD, palo);
			*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield | 0x2000000000;
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

	if (palo->pshadow != 0)
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow);

	palo->pvtalo->pfnUpdateAloXfWorld(palo);

	ResolveAlo(palo);
}

void OnAloRemove(ALO* palo)
{
	if (palo->paloParent == nullptr)
	{
		if (palo->fRealClock != 0)
			RemoveDlEntry(&palo->psw->dlMRDRealClock, palo);

		else
		{
			if (true)
			{
				RemoveDlEntry(&palo->psw->dlMRD, palo);
				RemoveDlEntry(&palo->psw->dlBusy, palo);

				if ((palo->pvtalo->grfcid & 0x2U) != 0)
				{
					RemoveDlEntry(&palo->psw->dlBusySo, palo);
				}
			}

			palo->paloFreezeRoot = nullptr;
			ClearDl(&palo->dlFreeze);
		}
	}

	if (palo->pshadow != nullptr)
		RemoveDlEntry(&palo->psw->dlShadow, palo->pshadow);

	ResolveAlo(palo);
	palo->paloRoot = nullptr;
}

void CloneAloHierarchy(ALO* palo, ALO* paloBase)
{
	DLI parentPalo{};

	// Storing the parent ptr to child ALO
	parentPalo.m_pdl = &paloBase->dlChild;

	// Storing the base offset to child ALO
	parentPalo.m_ibDle = paloBase->dlChild.ibDle;

	// Storing the parent ALO
	parentPalo.m_pdliNext = s_pdliFirst;

	// Keeping track of the ALO parent
	s_pdliFirst = &parentPalo;

	// Storing ptr to next ALO to clone
	parentPalo.m_ppv = (void**)(uintptr_t)parentPalo.m_pdl;

	// Clones paloBase to palo
	palo->pvtlo->pfnCloneLo(palo, paloBase);

	// Loading next ALO to clone
	LO* plo = (LO*)*parentPalo.m_ppv;
	// Loading up next ALO to clone after than one above
	parentPalo.m_ppv = (void**) ((uintptr_t)plo + parentPalo.m_ibDle);

	while (plo != nullptr)
	{
		// Clones ALO object
		PloCloneLo(plo, palo->psw, palo);
		// Loads next ALO object to clone
		plo = (LO*)*parentPalo.m_ppv;
		// Loads next ALO object to clone after this one
		parentPalo.m_ppv = (void**) ((uintptr_t)plo + parentPalo.m_ibDle);
	}

	s_pdliFirst = parentPalo.m_pdliNext;
}

void CloneAlo(ALO* palo, ALO* paloBase)
{
	LO lo = *palo;
	*palo = *paloBase;
	memcpy(palo, &lo, sizeof(LO));

	CloneLo(palo, paloBase);

	ClearDl(&palo->dlChild);
}

void ResolveAlo(ALO* palo)
{
	if (palo->paloRoot != nullptr) 
		palo->paloRoot->cframeStatic = 0;
}

void SetAloParent(ALO* palo, ALO* paloParent)
{
	if (palo->paloParent == paloParent)
		return;

	glm::vec3 posWorld = palo->xf.posWorld;
	glm::mat3 matWorld = palo->xf.matWorld;

	palo->pvtalo->pfnRemoveLo(palo);

	ConvertAloPos(nullptr, paloParent, posWorld, palo->xf.pos);
	ConvertAloMat(nullptr, paloParent, matWorld, palo->xf.mat);

	palo->paloParent = paloParent;

	palo->pvtlo->pfnAddLo(palo);
}

void ApplyAloProxy(ALO* palo, PROXY* pproxyApply)
{
	glm::vec3 posWorld{};
	ConvertAloPos((ALO*)pproxyApply, nullptr, palo->xf.pos, posWorld);
	palo->pvtalo->pfnTranslateAloToPos(palo, posWorld);

	glm::mat3 matWorld{};
	ConvertAloMat((ALO*)pproxyApply, nullptr, palo->xf.mat, matWorld);
	palo->pvtalo->pfnRotateAloToMat(palo, matWorld);

	palo->posOrig = palo->xf.pos;
	palo->matOrig = palo->xf.mat;
}

void UpdateAloXfWorld(ALO* palo)
{
	palo->pvtalo->pfnUpdateAloXfWorldHierarchy(palo);
}

void UpdateAloXfWorldHierarchy(ALO* palo)
{
	if (palo->palox == nullptr)
	{
		UpdateTrans:
		if (palo->paloParent == nullptr)
		{
			palo->xf.posWorld = palo->xf.pos;
			palo->xf.matWorld = palo->xf.mat;
		}

		else
		{
			palo->xf.pos = palo->paloParent->xf.matWorld * palo->xf.pos;
			palo->xf.posWorld = palo->xf.pos + palo->paloParent->xf.posWorld;

			palo->xf.mat = palo->paloParent->xf.matWorld * palo->xf.mat;
			palo->xf.matWorld = palo->xf.mat;
		}
	}

	else
	{
		if ((palo->palox->grfalox & 0xCU) == 0)
		{
			palo->paloParent = palo->paloParent;
			goto UpdateTrans;
		}

		if (palo->paloParent == nullptr)
		{
			palo->xf.posWorld = palo->xf.pos;
		}

		else
		{
			palo->xf.pos = palo->xf.pos * palo->paloParent->xf.matWorld;
			palo->xf.posWorld = palo->paloParent->xf.posWorld + palo->xf.pos;
		}

		if (palo->paloParent != nullptr)
		{
			palo->xf.matWorld = palo->xf.mat * palo->paloParent->xf.matWorld;
			goto UpdateTrans;
		}

		palo->xf.matWorld = palo->xf.mat;
	}

	ALO* object = palo->dlChild.paloFirst;

	if (object == nullptr)
	{
		palo->palox = palo->palox;
	}

	else
	{
		while (true)
		{
			if (object->pvtalo->pfnUpdateAloXfWorldHierarchy == nullptr)
				object = object->dleChild.paloNext;

			else 
			{
				object->pvtalo->pfnUpdateAloXfWorldHierarchy(object);
				object = object->dleChild.paloNext;
			}

			if (object == nullptr) 
				break;
		}
	}
}

void TranslateAloToPos(ALO* palo, glm::vec3 &ppos)
{
	palo->xf.pos = ppos;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3 &pposFrom, glm::vec3 &pposTo)
{
	if (paloFrom != paloTo)
	{
		if (paloFrom != nullptr)
		{
			pposFrom = paloFrom->xf.matWorld * pposFrom;
			pposFrom = paloFrom->xf.posWorld + pposFrom;
		}

		if (paloTo != nullptr)
		{
			pposFrom = pposFrom - paloTo->xf.posWorld;
			pposFrom = pposFrom * paloTo->xf.matWorld;
			pposTo = pposFrom;
			return;
		}
	}

	pposTo = pposFrom;
}

void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3 *pvecFrom, glm::vec3 *pvecTo)
{
	if (paloFrom != paloTo)
	{
		if (paloFrom != nullptr)
		{
			glm::vec3 vecWorld = paloFrom->xf.matWorld * *pvecFrom;
			pvecFrom = &vecWorld;
		}

		if (paloTo != nullptr)
		{
			*pvecTo = paloTo->xf.matWorld * *pvecFrom;
			return;
		}
	}

	*pvecTo = *pvecFrom;
}

void RotateAloToMat(ALO* palo, glm::mat3 &pmat)
{
	palo->xf.mat = pmat;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3 &pmatFrom, glm::mat3 &pmatTo)
{
	if (paloFrom != paloTo)
	{
		if (paloFrom != nullptr)
			pmatFrom = paloFrom->xf.matWorld * pmatFrom;

		if (paloTo != nullptr)
		{
			pmatTo = paloTo->xf.matWorld * pmatFrom;
			return;
		}
	}

	pmatTo = pmatFrom;
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

	plo.m_ppv = (void**)(uintptr_t)plo.m_pdl;

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
	// Model matrix
	palo->xf.mat = pbis->ReadMatrix();
	palo->xf.pos = pbis->ReadVector();
	//

	/*int bitfield;
	memcpy(&bitfield, &palo->bitfield, 4);
	std::cout << std::bitset<32>(bitfield) << "\n";*/

	byte temp0 = pbis->U8Read();
	temp0 = ((long)(char)temp0 & 3U) << 0x18;
	*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffffcffffff | temp0;

	//memcpy(&bitfield, &palo->bitfield, 4);
	//std::cout << std::bitset<32>(bitfield) << "\n";

	*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff3ffffff | ((long)(char)pbis->U8Read() & 3U) << 0x1a;
	*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xffffffffcfffffff | ((long)(char)pbis->U8Read() & 3U) << 0x1c;
	
	palo->grfzon = pbis->U32Read();
	palo->sMRD = pbis->F32Read();
	palo->sCelBorderMRD = pbis->F32Read();
	palo->sRadiusRenderSelf = pbis->F32Read();
	palo->sRadiusRenderAll = pbis->F32Read();

	LoadOptionsFromBrx(palo, pbis);
	LoadGlobsetFromBrx(&palo->globset ,pbis, palo);
	LoadAloAloxFromBrx(palo, pbis);

	palo->pvtalo->pfnUpdateAloXfWorld(palo);

	palo->cposec = pbis->U8Read();
	palo->aposec.resize(palo->cposec);

	for (int i = 0; i < palo->cposec; i++)
	{
		palo->aposec[i].oid = (OID)pbis->S16Read();
		palo->aposec[i].agPoses.resize(palo->globset.cpose);

		for (int a = 0; a < palo->globset.cpose; a++)
			palo->aposec[i].agPoses[a] = pbis->F32Read();
	}

	// Loads ALO children objects
	LoadSwObjectsFromBrx(palo->psw, palo, pbis);
}

void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	GRFALOX grfalox = pbis->U32Read();

	if (grfalox != 0)
	{
		ALOX alox;
		palo->palox = &alox;

		palo->palox->grfalox = grfalox;

		int unk_1;

		if (grfalox & 1)
		{
			pbis->ReadMatrix();
		}

		if (grfalox & 2)
		{
			pbis->ReadMatrix();
		}

		if (((grfalox & 0xc) != 0) && (unk_1 = pbis->S16Read() != -1))
		{

		}

		if ((grfalox & 0x10) != 0)
			unk_1 = pbis->S16Read();

		if ((grfalox & 0x20) != 0)
		{
			unk_1 = pbis->S16Read();
			pbis->ReadVector(); // Read Vector
			pbis->ReadVector(); // Read Vector
			pbis->F32Read();
		}

		if ((grfalox & 0x40) != 0)
		{
			unk_1 = pbis->S16Read();
			unk_1 = pbis->S16Read();
		}

		if ((grfalox & 0x80) != 0)
		{
			pbis->ReadVector();
			pbis->F32Read();

			if ((grfalox & 0x100) != 0)
			{
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->ReadVector();
			}
		}

		if ((grfalox & 0x200) != 0)
		{
			pbis->S16Read();
			pbis->S16Read();
		}

		if ((grfalox & 0x400) != 0)
		{
			pbis->U8Read();
		}
	}
}

void DebugClearLightingToF32(ALO* palo)
{

}

void DebugConvertLightingF32ToU8(ALO* palo)
{
	glm::vec3 lighting{};

	float ambientSaturation = palo->psw->lsmDefault.uShadow;
	float diffuseSaturation = (palo->psw->lsmDefault.uMidtone + palo->globset.aglob[0].asubglob[0].unSelfIllum) * 0.31;
	
	if (lighting.g < lighting.r) {
		lighting.g = glm::max(lighting.b, lighting.r);
	}
	
	else
	{
		lighting.g = glm::max(lighting.g, lighting.b);
	}

	lighting.g = 1.0 - lighting.g;
	lighting.b = lighting.g - diffuseSaturation;

	diffuseSaturation = glm::min(diffuseSaturation, lighting.g);
	diffuseSaturation = glm::max(diffuseSaturation, diffuseSaturation * 0);
	lighting.g = glm::min(lighting.b, ambientSaturation);

	lighting.g = glm::max(lighting.g, lighting.g * 0);

	//float color = (aVertexColor.r + aVertexColor.g + aVertexColor.b) * 0.3333333;
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

void DrawGlob(ALO* palo, int index)
{
	glm::mat4 model = palo->xf.matWorld;

	model[3][0] = palo->xf.posWorld[0];
	model[3][1] = palo->xf.posWorld[1];
	model[3][2] = palo->xf.posWorld[2];
	model[3][3] = 1.0;

	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		for (int a = 0; a < palo->globset.aglob[i].csubglob; a++)
		{
			glBindVertexArray(palo->globset.aglob[i].asubglob[a].VAO);

			glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

			glUniform1i(glGetUniformLocation(glGlobShader.ID, "shdk"), palo->globset.aglob[i].asubglob[a].pshd->shdk);
			glUniform1f(glGetUniformLocation(glGlobShader.ID, "usSelfIllum"), palo->globset.aglob[i].asubglob[a].unSelfIllum);

			if (palo->globset.aglob[i].asubglob[a].pshd->shdk == SHDK_ThreeWay)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, palo->globset.aglob[i].asubglob[a].pshd->glShadowTexture);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, palo->globset.aglob[i].asubglob[a].pshd->glDiffuseTexture);

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, palo->globset.aglob[i].asubglob[a].pshd->glSaturateTexture);
			}

			else
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, palo->globset.aglob[i].asubglob[a].pshd->glDiffuseTexture);
			}
			
			switch (palo->globset.aglob[i].rp)
			{
				case RP_Translucent:
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_LINES, GL_NONE);
				break;

				case RP_ProjVolume:
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_LINES, GL_LINES, GL_NONE);
				break;
			}

			glDrawElements(GL_TRIANGLES, palo->globset.aglob[i].asubglob[a].indices.size(), GL_UNSIGNED_SHORT, 0);
			
			//Draws instanced models, I WILL OPTIMIZE THIS LATER
			for (int b = 0; b < palo->globset.aglob[i].pdmat.size(); b++)
			{
				// Sends instance model matrix to GPU
				glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(palo->globset.aglob[i].pdmat[b]));
				glDrawElements(GL_TRIANGLES, palo->globset.aglob[i].asubglob[a].indices.size(), GL_UNSIGNED_SHORT, 0);
			}
		}

		if (palo->globset.aglob[i].rp == RP_Translucent || palo->globset.aglob[i].rp == RP_Cutout || palo->globset.aglob[i].rp == RP_ProjVolume)
			glDisable(GL_BLEND);
	}

	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void DeleteModel(ALO *palo)
{
	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
		{
			glDeleteVertexArrays(1, &palo->globset.aglob[i].asubglob[a].VAO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].VBO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].EBO);
		}
	}
}

int GetAloSize()
{
	return sizeof(ALO);
}

void DeleteAlo(ALO* palo)
{
	delete palo;
}
