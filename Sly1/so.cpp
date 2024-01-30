#include "so.h"

void* NewSo()
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
	pbis->U8Read();

	pbis->ReadGeom(&pso->geomLocal);
	pbis->ReadBspc();

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

	pbis->ReadGeom(&pso->geomCameraLocal);
	pbis->ReadBspc();

	LoadAloFromBrx(pso, pbis);
}

void TranslateSoToPos(SO* pso, glm::vec3& ppos)
{
	pso->xf.pos = ppos;

	if (pso->paloRoot != nullptr)
		pso->paloRoot->pvtalo->pfnUpdateAloXfWorld(pso);
}

void RotateSoToMat(SO* pso, glm::mat3& pmat)
{
	pso->xf.mat = pmat;

	pso->pvtalo->pfnUpdateAloXfWorld(pso);
}

void MakeCollisionGLBuffers(GEOM *pgeom)
{
	glGenVertexArrays(1, &pgeom->VAO);
	glBindVertexArray(pgeom->VAO);

	glGenBuffers(1, &pgeom->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, pgeom->VBO);
	glBufferData(GL_ARRAY_BUFFER, pgeom->apos.size() * sizeof(glm::vec3), pgeom->apos.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &pgeom->EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pgeom->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, pgeom->indices.size() * sizeof(uint16_t), pgeom->indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void UpdateSo(SO* pso, float dt)
{

}

void RenderSoSelf(SO* pso, CM* pcm, RO* pro)
{
	RenderAloSelf(pso, pcm, pro);
}

void DrawSoCollision(SO* pso)
{
	glm::mat4 model = pso->xf.matWorld;
	
	model[3][0] = pso->xf.posWorld[0];
	model[3][1] = pso->xf.posWorld[1];
	model[3][2] = pso->xf.posWorld[2];
	model[3][3] = 1.0;

	int modelUniformLocation = glGetUniformLocation(glShaderCollision.ID, "model");
	glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(pso->geomLocal.VAO);
	glDrawElements(GL_LINES, pso->geomLocal.indices.size(), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(pso->geomCameraLocal.VAO);
	glDrawElements(GL_LINES, pso->geomCameraLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
}

void DeleteSo(LO* plo)
{
	delete(SO*)plo;
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

	allSWSoObjs.clear();
	allSWSoObjs.shrink_to_fit();
}
