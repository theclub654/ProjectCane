#include "so.h"

SO* NewSo()
{
	return new SO{};
}

void InitSwBusySoDl(SW* psw)
{
	InitDl(&psw->dlBusySo, offsetof(SO, dleBusySo));
}

void InitSwRootDl(SW *psw)
{
	InitDl(&psw->dlRoot, offsetof(SO, dleRoot));
}

void InitSo(SO* pso)
{
	InitDl(&pso->dlPhys, offsetof(SO, dlePhys));
	InitAlo(pso);

	InitGeom(&pso->geomLocal);

	allSWSoObjs.push_back(pso);
}

int GetSoSize()
{
	return sizeof(SO);
}

void OnSoAdd(SO *pso)
{
	pso->psw->cpsoAll++;

	if (pso->paloParent == nullptr)
	{
		pso->psw->cpsoRoot++;
		AppendDlEntry(&pso->psw->dlRoot, pso);
	}

	OnAloAdd(pso);
}

void OnSoRemove(SO* pso)
{
	OnAloRemove(pso);
	pso->psw->cpsoAll--;

	if (pso->paloParent == nullptr)
	{
		RemoveDlEntry(&pso->psw->dlRoot, pso);
		pso->psw->cpsoRoot--;
	}

	pso->pstso = nullptr;
}

void CloneSo(SO* pso, SO* psoBase)
{
	LO lo = *pso;
	*pso = *psoBase;
	memcpy(pso, &lo, sizeof(LO));

	CloneLo(pso, psoBase);

	ClearDl(&pso->dlChild);

	pso->pxa = nullptr;
	pso->grfpvaXpValid = 0;
	pso->pstso = nullptr;
}

void SetSoParent(SO* pso, ALO* paloParent)
{
	if (pso->paloParent != paloParent)
	{
		SetAloParent(pso, paloParent);
	}
}

void ApplySoProxy(SO* pso, PROXY* pproxyApply)
{
	ApplyAloProxy(pso, pproxyApply);
}

void UpdateSoXfWorldHierarchy(SO* pso)
{
	UpdateAloXfWorldHierarchy(pso);
}

void UpdateSoXfWorld(SO* pso)
{
	UpdateAloXfWorld(pso);
}

void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis)
{
	*(unsigned long*)&pso->bitfield = *(unsigned long*)&pso->bitfield & 0xfffffffdffffffff | ((long)(char)pbis->U8Read() & 1U) << 0x21;
	ReadGeom(&pso->geomLocal, pbis);
	ReadBspc(&pso->geomWorld, &pso->bspc, pbis);

	pso->m = pbis->F32Read();

	pso->momintLocal =  pbis->ReadMatrix();
	pso->posComLocal = pbis->ReadVector();

	pso->cnpg = pbis->U16Read();
	pso->anpg.resize(pso->cnpg);

	for (int i = 0; i < pso->cnpg; i++)
	{
		pso->anpg[i].cmk = pbis->S16Read();
		pso->anpg[i].ipglob = pbis->U16Read();
	}

	pso->mpibspinpg.resize(pbis->U32Read());

	for (int i = 0; i < pso->mpibspinpg.size(); i++)
		pso->mpibspinpg[i] = pbis->S16Read();

	pso->chsg = pbis->U32Read();
	pso->ahsg.resize(pso->chsg);

	for (int i = 0; i < pso->chsg; i++)
	{
		pso->ahsg[i].ipglob = pbis->S16Read();
		pso->ahsg[i].ipsubglob = pbis->S16Read();
	}

	ReadGeom(&pso->geomCameraLocal, pbis);
	ReadBspc(&pso->geomCameraWorld, &pso->bspcCamera, pbis);

	LoadAloFromBrx(pso, pbis);
}

void TranslateSoToPos(SO* pso, glm::vec3& ppos)
{
	pso->xf.pos = ppos;

	if (pso->paloRoot != nullptr)
		pso->pvtalo->pfnUpdateAloXfWorld(pso);
}

void RotateSoToMat(SO* pso, glm::mat3& pmat)
{
	pso->xf.mat = pmat;

	if (pso->paloRoot != nullptr)
		pso->pvtalo->pfnUpdateAloXfWorld(pso);
}

void UpdateSo(SO* pso, float dt)
{
	UpdateAlo(pso, dt);
}

void RenderSoSelf(SO* pso, CM* pcm, RO* pro)
{
	RenderAloSelf(pso, pcm, pro);

	/*if (g_fRenderCollision != 0)
	{
		RPL rpl{};

		rpl.PFNDRAW = DrawCollision;

		glm::mat4 baseModelMatrix{};
		LoadMatrixFromPosRot(pso->xf.posWorld, pso->xf.matWorld, baseModelMatrix);
		rpl.ro.modelmatrix = baseModelMatrix;
	    
		rpl.ro.VAO = &pso->geomLocal.VAO;
		rpl.rp = RP_Opaque;
		rpl.ro.cvtx = pso->geomLocal.indices.size();

		SubmitRpl(&rpl);
	}*/
}

void DrawCollision(CM* pcm, SO* pso)
{
	if (pso->geomLocal.VAO != 0 || pso->geomCameraLocal.VAO != 0)
	{
		glm::mat4 model = pso->xf.matWorld;

		model[3][0] = pso->xf.posWorld[0];
		model[3][1] = pso->xf.posWorld[1];
		model[3][2] = pso->xf.posWorld[2];
		model[3][3] = 1.0;

		if (pso->pvtso->cid == CID_VOLZP)
			glUniform4fv(glGetUniformLocation(glGlobShader.ID, "collisionRgba"), 1, glm::value_ptr(glm::vec4(255.0, 0.0, 0.0, 1.0)));
		else
			glUniform4fv(glGetUniformLocation(glGlobShader.ID, "collisionRgba"), 1, glm::value_ptr(glm::vec4(0.76, 0.76, 0.76, 1.0)));

		glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(pso->geomLocal.VAO);
		glDrawElements(GL_LINES, pso->geomLocal.indices.size(), GL_UNSIGNED_SHORT, 0);

		if (pso->geomCameraLocal.VAO != 0)
		{
			glBindVertexArray(pso->geomCameraLocal.VAO);
			glDrawElements(GL_LINES, pso->geomCameraLocal.indices.size(), GL_UNSIGNED_SHORT, 0);

			glBindVertexArray(0);
		}
	}
}

void DeleteSo(SO *pso)
{
	delete pso;
}

void DeleteSwCollision()
{
	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.VAO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.VBO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.EBO);

		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.VAO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.VBO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.EBO);
	}
}

void DeleteSoGeom(SO* pso)
{
	glDeleteVertexArrays(1, &pso->geomLocal.VAO);
	glDeleteVertexArrays(1, &pso->geomLocal.VBO);
	glDeleteVertexArrays(1, &pso->geomLocal.EBO);

	glDeleteVertexArrays(1, &pso->geomCameraLocal.VAO);
	glDeleteVertexArrays(1, &pso->geomCameraLocal.VBO);
	glDeleteVertexArrays(1, &pso->geomCameraLocal.EBO);
}