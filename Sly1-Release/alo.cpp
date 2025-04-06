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

void AdjustAloRtckMat(ALO* palo, CM* pcm, RTCK rtck, glm::vec3* pposCenter, glm::mat4* pmat)
{
	
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
	LoadGlobsetFromBrx(&palo->globset, palo->pvtalo->cid, palo, pbis);
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
			pbis->U8Read();
		}
	}
}

void UpdateAlo(ALO* palo, float dt)
{

}

void RenderAloAll(ALO* palo, CM* pcm, RO* proDup)
{
	palo->pvtalo->pfnRenderAloSelf(palo, pcm, proDup);
}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{
	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, pro);
}

void DupAloRo(ALO *palo, RO *proOrig, RO *proDup)
{
	if (proOrig == nullptr)
	{
		glm::vec3 vecScale = glm::vec3(1.0);
		LoadMatrixFromPosRotScale(palo->xf.posWorld, palo->xf.matWorld, vecScale, proDup->modelmatrix);
		proDup->uAlpha = 1.0;
		proDup->uAlphaCelBorder = 1.0;
	}
	else
	{
		if (proOrig != proDup)
			proDup->modelmatrix = proOrig->modelmatrix;
	}
}

void RenderAloGlobset(ALO *palo, CM *pcm, RO *pro)
{
	RPL rpl{};

	rpl.PFNDRAW = DrawGlob;

	DupAloRo(palo, pro, &rpl.ro);

	glm::mat4 modelmatrix = rpl.ro.modelmatrix;

	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		if ((palo->globset.aglobi[i].grfzon & pcm->grfzon) == pcm->grfzon) // Check if submodel is in camera BSP zone
		{
			for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
			{
				rpl.ro.VAO = &palo->globset.aglob[i].asubglob[a].VAO;

				if (palo->globset.aglob[i].asubglob[a].fCelBorder == 1)
				{
					rpl.ro.celVAO = &palo->globset.aglob[i].asubglob[a].celVAO;
					rpl.ro.celcvtx = palo->globset.aglob[i].asubglob[a].celcvtx;
					rpl.ro.fCelBorder = 1;
				}
				else
				{
					rpl.ro.celVAO = nullptr;
					rpl.ro.fCelBorder = 0;
				}

				rpl.posCenter = palo->globset.aglob[i].posCenter;

				rpl.ro.fDynamic = palo->globset.aglob[i].fDynamic;

				rpl.ro.grfglob = &palo->globset.aglob[i].grfglob;
				rpl.ro.pshd = palo->globset.aglob[i].asubglob[a].pshd;

				rpl.ro.unSelfIllum = &palo->globset.aglob[i].asubglob[a].unSelfIllum;

				rpl.ro.cvtx = palo->globset.aglob[i].asubglob[a].cvtx;

				rpl.z = palo->globset.aglob[i].gZOrder;

				if (rpl.z == 3.402823e+38)
					rpl.z = glm::dot(rpl.z, rpl.z);

				rpl.rp = palo->globset.aglob[i].rp;

				if (palo->globset.aglob[i].dmat.size() != 0)
					rpl.ro.modelmatrix = modelmatrix * palo->globset.aglob[i].dmat[0];

				SubmitRpl(&rpl);

				if (palo->globset.aglob[i].dmat.size() != 0)
					rpl.ro.modelmatrix = modelmatrix;
			}
		}

	}
}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{

}

void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro)
{

}

void DrawGlob(RPL *prpl)
{
	glBindVertexArray(*prpl->ro.VAO);

	glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(prpl->ro.modelmatrix));

	glUniform1f(glGetUniformLocation(glGlobShader.ID, "usSelfIllum"), *prpl->ro.unSelfIllum);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "fDynamic"), prpl->ro.fDynamic);
	glUniform3fv(glGetUniformLocation(glGlobShader.ID, "posCenter"), 1, glm::value_ptr(prpl->posCenter));

	if (prpl->ro.pshd->shdk == SHDK_ThreeWay)
	{
		glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glShadowMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glDiffuseMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glSaturateMap);
	}
	else
	{
		glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glDiffuseMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	switch (prpl->rp)
	{
		case RP_Background:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(false);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDisable(GL_BLEND);
		glDepthMask(true);
		break;

		case RP_ProjVolume:
		glEnable(GL_BLEND);
		if ((prpl->ro.pshd->grfshd & 2) == 0)
			glBlendFunc(GL_NONE, GL_ONE);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(false);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 128, 128);
		glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
		glColorMask(0, 0, 0, 0);
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glColorMask(1, 1, 1, 1);
		glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
		glFrontFace(GL_CCW);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDepthFunc(GL_ALWAYS);
		glStencilFunc(GL_EQUAL, 128, 128);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);
		glDepthMask(true);
		break;

		case RP_Cutout:
		case RP_CutoutAfterProjVolume:
		case RP_Translucent:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
		glDepthMask(false);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDisable(GL_BLEND);
		glDepthMask(true);
		break;

		default:
			if (prpl->ro.fCelBorder == 1)
			{
				glEnable(GL_STENCIL_TEST);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);
				glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

				glBindVertexArray(*prpl->ro.celVAO);
				glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 2);
				glDepthMask(false);
				//glDepthFunc(GL_LEQUAL);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0x00);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDrawElements(GL_TRIANGLES, prpl->ro.celcvtx, GL_UNSIGNED_SHORT, 0);

				glDisable(GL_BLEND);
				glDepthMask(true);
				glStencilMask(0xFF);
				glStencilFunc(GL_ALWAYS, 0, 0xFF);
				glDepthFunc(GL_LESS);
			}
			else
				glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
		break;
	}

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

			glDeleteVertexArrays(1, &palo->globset.aglob[i].asubglob[a].celVAO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].celVBO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].celEBO);
		}
	}
}

int GetAloSize()
{
	return sizeof(ALO);
}

void DeleteAlo(ALO *palo)
{
	delete palo;
}
