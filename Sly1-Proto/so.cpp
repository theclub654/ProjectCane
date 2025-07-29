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
	//pso->dleRoot = psoBase->dleRoot;
	//pso->dlPhys = psoBase->dlPhys;
	//pso->dlePhys = psoBase->dlePhys;
	pso->momintLocal = psoBase->momintLocal;
	pso->momintInvLocal = psoBase->momintInvLocal;
	pso->dvGravity = psoBase->dvGravity;
	pso->gBuoyancy = psoBase->gBuoyancy;
	pso->gViscosity = psoBase->gViscosity;
	pso->m = psoBase->m;
	pso->posWorldPrev = psoBase->posWorldPrev;
	pso->geomLocal = psoBase->geomLocal;
	pso->geomWorld = psoBase->geomWorld;
	pso->plvo = psoBase->plvo;
	pso->sRadiusSelf = psoBase->sRadiusSelf;
	pso->sRadiusAll = psoBase->sRadiusAll;
	pso->sRadiusPrune = psoBase->sRadiusPrune;
	pso->posPrune = psoBase->posPrune;
	pso->bspc = psoBase->bspc;
	pso->cnpg = psoBase->cnpg;
	pso->anpg = psoBase->anpg;
	pso->mpibspinpg = psoBase->mpibspinpg; 
	pso->chsg = psoBase->chsg;
	pso->ahsg = psoBase->ahsg; 
	pso->mpisurfihsgMic = psoBase->mpisurfihsgMic;
	//pso->dleBusySo = psoBase->dleBusySo;
	pso->posMin = psoBase->posMin;
	pso->posMax = psoBase->posMax;
	pso->constrForce = psoBase->constrForce;
	pso->constrTorque = psoBase->constrTorque;
	//pso->poxa = psoBase->poxa;
	pso->dpos = psoBase->dpos;
	pso->drot = psoBase->drot;
	pso->pxa = nullptr;
	pso->pxpInternal = psoBase->pxpInternal;
	pso->grfpvaXpValid = 0;
	pso->ipsoRoot = psoBase->ipsoRoot;
	pso->ipso = psoBase->ipso;
	pso->posComLocal = psoBase->posComLocal;
	//pso->psoPhysHook = psoBase->psoPhysHook;
	pso->geomCameraLocal = psoBase->geomCameraLocal;
	pso->geomCameraWorld = psoBase->geomCameraWorld;
	//pso->bspcCamera = psoBase->bspcCamera;
	pso->cmk = psoBase->cmk;
	pso->egk = psoBase->egk;
	pso->fSphere = psoBase->fSphere;
	pso->fClone = psoBase->fClone;
	pso->fNoXpsAll = psoBase->fNoXpsAll;
	pso->fNoXpsSelf = psoBase->fNoXpsSelf;
	pso->fNoXpsCenter = psoBase->fNoXpsCenter;
	pso->fActive = psoBase->fActive;
	pso->fVelcro = psoBase->fVelcro;
	pso->fIgnoreLocked = psoBase->fIgnoreLocked;
	pso->fIceable = psoBase->fIceable;
	pso->fRoot = psoBase->fRoot;
	pso->fPhys = psoBase->fPhys;
	pso->fNoGravity = psoBase->fNoGravity;
	pso->fCenterXp = psoBase->fCenterXp;
	pso->fLockedSelf = psoBase->fLockedSelf;
	pso->fLockedAll = psoBase->fLockedAll;
	pso->fLockedAbove = psoBase->fLockedAbove;
	pso->fCpsoBuildContactGroup = psoBase->fCpsoBuildContactGroup;
	pso->fCpxpBuildArray = psoBase->fCpxpBuildArray;
	pso->fUpdateXaList1 = psoBase->fUpdateXaList1;
	pso->fUpdateXaList2 = psoBase->fUpdateXaList2;
	pso->fRecalcSwXpAll = psoBase->fRecalcSwXpAll;
	pso->fHandleDiveEffect = psoBase->fHandleDiveEffect;
	pso->fGenSpliceTouchEvents = psoBase->fGenSpliceTouchEvents;
	pso->pstso = nullptr;

	CloneAlo(pso, psoBase);
}

void SetSoParent(SO* pso, ALO* paloParent)
{
	if (pso->paloParent == paloParent)
		return;

	// Backup original root for later comparison
	SO* previousRoot = static_cast<SO*>(pso->paloRoot);

	// Convert constraint force and torque vectors to world space
	glm::vec3 normalForceWorld, normalTorqueWorld;
	ConvertAloVec(pso->paloParent, nullptr, &pso->constrForce.normal, &normalForceWorld);
	ConvertAloVec(pso->paloParent, nullptr, &pso->constrTorque.normal, &normalTorqueWorld);

	// Perform the reparenting
	SetAloParent(static_cast<ALO*>(pso), paloParent);

	// Convert world-space force/torque back into new parent's local space
	ConvertAloVec(nullptr, paloParent, &normalForceWorld, &pso->constrForce.normal);
	ConvertAloVec(nullptr, paloParent, &normalTorqueWorld, &pso->constrTorque.normal);

	// Handle post-reparent updates
	if (pso->paloRoot != nullptr) {
		// If the old root had flag 0x20000000000000 set, update it
		if ((reinterpret_cast<uint64_t&>(pso->bspcCamera.absp) & 0x20000000000000ULL) != 0 &&
			previousRoot != nullptr) {
			//RecalcSoLocked(previousRoot);
		}

		// Check whether the new root wants self or root update
		if ((reinterpret_cast<uint64_t&>(
			static_cast<SO*>(pso->paloRoot)->bspcCamera.absp) & 0x40000000000000ULL) == 0) {
			//RecalcSoLocked(pso);
		}
		else {
			//RecalcSoLocked(static_cast<SO*>(pso->paloRoot));
		}
	}

	// Always rebuild physics hook
	//RebuildSoPhysHook(pso);
}

void SetSoVelocityVec(SO* pso, glm::vec3* pv)
{
	if (!pso || !pv) return;

	// Apply velocity using base ALO logic
	SetAloVelocityVec(pso, pv);

	// Compute velocity delta
	glm::vec3 newV = *pv;
	glm::vec3 oldV = pso->xf.v;
	glm::vec3 delta = newV - oldV;

	// Threshold check
	constexpr float velocityEpsilon = 2.0f;
	bool withinThreshold =
		glm::abs(delta.x) < velocityEpsilon &&
		glm::abs(delta.y) < velocityEpsilon &&
		glm::abs(delta.z) < velocityEpsilon;

	/*if (!withinThreshold) {
		InvalidateSwXpForObject(pso->psw, pso, 2);
	}*/
}

void SetSoAngularVelocityVec(SO* pso, glm::vec3* pw)
{
	if (!pso || !pw) return;

	// Set angular velocity using ALO-level logic
	SetAloAngularVelocityVec(pso, pw);

	// Compare new vs. existing angular velocity
	glm::vec3 newW = *pw;
	glm::vec3 oldW = pso->xf.w;
	glm::vec3 delta = newW - oldW;

	// Threshold comparison
	constexpr float epsilon = 0.02f;
	bool withinThreshold =
		glm::abs(delta.x) < epsilon &&
		glm::abs(delta.y) < epsilon &&
		glm::abs(delta.z) < epsilon;

	/*if (!withinThreshold) {
		InvalidateSwXpForObject(pso->psw, pso, 2);
	}*/
}

void SetSoConstraints(SO* pso, CT ctForce, const glm::vec3* pnormalForce, CT ctTorque, const glm::vec3* pnormalTorque)
{
	
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
	pso->pvtso->pfnUpdateSoPosWorldPrev(pso);
}

void UpdateSoPosWorldPrev(SO* pso)
{
	pso->posWorldPrev = pso->xf.posWorld;
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
			glUniform4fv(glslCollisionRgba, 1, glm::value_ptr(glm::vec4(255.0, 0.0, 0.0, 1.0)));
		else
			glUniform4fv(glslCollisionRgba, 1, glm::value_ptr(glm::vec4(0.76, 0.76, 0.76, 1.0)));

		glUniformMatrix4fv(glslModel, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(pso->geomLocal.VAO);
		glDrawElements(GL_LINES, pso->geomLocal.indices.size(), GL_UNSIGNED_SHORT, 0);

		if (pso->geomCameraLocal.VAO != 0)
		{
			glBindVertexArray(pso->geomCameraLocal.VAO);
			glDrawElements(GL_LINES, pso->geomCameraLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}

		glBindVertexArray(0);
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

void DeallocateSoVector()
{
	allSWSoObjs.clear();
	allSWSoObjs.shrink_to_fit();
}

std::vector <SO*> allSWSoObjs;