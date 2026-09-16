#include "explo.h"

EXPLO* NewExplo()
{
	return new EXPLO{};
}

void InitExplo(EXPLO* pexplo)
{
	InitXfm(pexplo);
	pexplo->oidShape = OID_Nil;
	pexplo->oidReference = OID_Nil;
}

EMITOK* PemitbEnsureExploEmitok(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.emitok;
}

glm::vec3* PemitbEnsureExploPosOrigin(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.posOrigin;
}

glm::vec3* PemitbEnsureExploPosMin(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.boxOrigin.posMin;
}

glm::vec3* PemitbEnsureExploPosMax(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.boxOrigin.posMax;
}

EMITNK* PemitbEnsureExploEmitnk(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.emitnk;
}

glm::vec3* PemitbEnsureExploVec(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.vec;
}

LM* PemitbEnsureExploLmSOffset(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.lmSOffset;
}

void* PemitbEnsureExploEmitvk(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emito.emitvk;
}

LM* PemitbEnsureExploLmSv(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emitv.lmSv;
}

float* PemitbEnsureExploRSvz(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emitv.rSvz;
}

LM* PemitbEnsureExploLmTilt(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emitv.lmTilt;
}

int* PemitbEnsureExploCParticlePerRing(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emitv.cParticlePerRing;
}

float* PemitbEnsureExploURandomRad(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emitv.uRandomRad;
}

LM* PemitbEnsureExploLmDtSkip(EXPLO* pexplo, ENSK ensk)
{
	return &PemitbEnsureExplo(pexplo, ensk)->emitv.lmDtSkip;
}

void PemitbSetExploEmitok(EXPLO* pexplo, int value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.emitok = static_cast<EMITOK>(value);
}

void PemitbSetExploPosOrigin(EXPLO* pexplo, glm::vec3 value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.posOrigin = value;
}

void PemitbSetExploPosMin(EXPLO* pexplo, glm::vec3 value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.boxOrigin.posMin = value;
}

void PemitbSetExploPosMax(EXPLO* pexplo, glm::vec3 value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.boxOrigin.posMax = value;
}

void PemitbSetExploEmitnk(EXPLO* pexplo, int value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.emitnk = static_cast<EMITNK>(value);
}

void PemitbSetExploVec(EXPLO* pexplo, glm::vec3 value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.vec = value;
}

void PemitbSetExploLmSOffset(EXPLO* pexplo, glm::vec2 value)
{
	LM& lm = PemitbEnsureExplo(pexplo, ENSK_Set)->emito.lmSOffset;
	lm.gMin = value.x;
	lm.gMax = value.y;
}

void PemitbSetExploEmitvk(EXPLO* pexplo, int value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emito.emitvk = static_cast<EMITVK>(value);
}

void PemitbSetExploLmSv(EXPLO* pexplo, glm::vec2 value)
{
	LM& lm = PemitbEnsureExplo(pexplo, ENSK_Set)->emitv.lmSv;
	lm.gMin = value.x;
	lm.gMax = value.y;
}

void PemitbSetExploRSvz(EXPLO* pexplo, float value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emitv.rSvz = value;
}

void PemitbSetExploLmTilt(EXPLO* pexplo, glm::vec2 value)
{
	LM& lm = PemitbEnsureExplo(pexplo, ENSK_Set)->emitv.lmTilt;
	lm.gMin = value.x;
	lm.gMax = value.y;
}

void PemitbSetExploCParticlePerRing(EXPLO* pexplo, int value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emitv.cParticlePerRing = value;
}

void PemitbSetExploURandomRad(EXPLO* pexplo, float value)
{
	PemitbEnsureExplo(pexplo, ENSK_Set)->emitv.uRandomRad = value;
}

void PemitbSetExploLmDtSkip(EXPLO* pexplo, glm::vec2 value)
{
	LM& lm = PemitbEnsureExplo(pexplo, ENSK_Set)->emitv.lmDtSkip;
	lm.gMin = value.x;
	lm.gMax = value.y;
}

void* GetExploOidReference(EXPLO* pexplo)
{
	return &pexplo->oidReference;
}

void SetExploOidReference(EXPLO* pexplo, int oidReference)
{
	pexplo->oidReference = (OID)oidReference;
}

void* GetExploOidShape(EXPLO* pexplo)
{
	return &pexplo->oidShape;
}

void SetExploOidShape(EXPLO* pexplo, int oidShape)
{
	pexplo->oidShape = (OID)oidShape;
}

int GetExploSize()
{
	return sizeof(EXPLO);
}

void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis)
{
	EMITB emitb{};
	pexplo->pemitb = std::make_shared <EMITB>(emitb);
	InitEmitb(pexplo->pemitb.get());

	LoadXfmFromBrx(pexplo, pbis);
	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pexplo->pemitb->emito.emitcrvOrigin.pcrv = pcrv;

		if (pcrv != nullptr && pcrv->pvtcrv->pfnLoadCrvFromBrx != nullptr)
			pcrv->pvtcrv->pfnLoadCrvFromBrx(pcrv.get(), pbis);
	}

	if (pexplo->pemitb->emito.emitok == EMITOK_Mesh)
		LoadEmitMeshFromBrx(&pexplo->pemitb->emito.emitmeshOrigin, pbis);

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
	{
		EMITB* pemitb = PemitbEnsureExplo(pexplo, ENSK_Set);
		LoadEmitblipColorsFromBrx(&pemitb->emitp.emitblip, crgba, pbis);
	}
}

void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase)
{
	CloneExpl(pexplo, pexploBase);

	pexplo->pemitb = pexploBase->pemitb;
	pexplo->oidReference = pexploBase->oidReference;
	pexplo->oidShape = pexploBase->oidShape;
}

void BindExplo(EXPLO* pexplo)
{
	EMITB* pemitb = pexplo->pemitb.get();

	const bool fNeedsBinding =
		pexplo->oidReference != OID_Nil ||
		pexplo->oidShape != OID_Nil ||
		pemitb->emito.emitok == EMITOK_Skeleton ||
		(pemitb->emitp.emitpk == EMITPK_Blip && pemitb->emitp.emitblip.oidSplineTarget != OID_Nil);

	if (fNeedsBinding)
	{
		pemitb = PemitbEnsureExplo(pexplo, ENSK_Set);

		if (pexplo->oidReference != OID_Nil)
			pemitb->emito.paloReference = static_cast<ALO*>(PloFindSwObject(pexplo->psw, 260, (OID)pexplo->oidReference, pexplo));
		if (pexplo->oidShape != OID_Nil)
		{
			XFM* pxfmShape = static_cast<XFM*>(PloFindSwObject(pexplo->psw, 260, (OID)pexplo->oidShape, pexplo));

			if (pxfmShape != nullptr)
			{
				pemitb->emito.posOrigin = pxfmShape->posLocal;
				pemitb->emito.paloReference = pxfmShape->paloParent;
			}
		}

		pemitb = pexplo->pemitb.get();
	}

	BindEmitb(pemitb, pexplo);
}

std::shared_ptr <EMITB> PemitbCopyOnWrite(const std::shared_ptr<EMITB>& pemitb)
{
	if (!pemitb || pemitb.use_count() == 1)
		return pemitb;

	return std::make_shared<EMITB>(*pemitb);
}

EMITB* PemitbEnsureExplo(EXPLO* pexplo, ENSK ensk)
{
	if (ensk == ENSK_Set)
		pexplo->pemitb = PemitbCopyOnWrite(pexplo->pemitb);

	return pexplo->pemitb.get();
}

void AddExploSkeleton(EXPLO* pexplo, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther)
{
	EMITB *pemitb = PemitbEnsureExplo(pexplo, ENSK_Set);
	AddEmitoSkeleton(&pemitb->emito, oid, oidOther, sRadius, gDensity, sRadiusOther, gDensityOther, pexplo);
}

void SetExploRipt(EXPLO* pexplo, RIPT ript)
{
	EMITB* pemitb = PemitbEnsureExplo(pexplo, ENSK_Set);
	SetEmitbRipt(pemitb, ript);
}

void StandardSmokeCloud(glm::vec3* ppos, float sRadius)
{
	EXPLO* stockSmoke = (EXPLO*)g_psw->aploStock[9];
	if (stockSmoke == nullptr)
		return;

	EXPLSO explso{};
	explso.posOrigin = *ppos;
	explso.grfexplso = 12;
	explso.rScale = 0.0f;
	explso.sRadius = sRadius;

	stockSmoke->pvtexplo->pfnExplodeExploExplso(stockSmoke, &explso);
}

void DeleteExplo(EXPLO* pexplo)
{
	delete pexplo;
}
