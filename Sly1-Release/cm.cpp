#include "cm.h"
#include "cplook.h"
#include "cpalign.h"
#include "cpman.h"
#include "cpaseg.h"
#include "cptn.h"
#include "jt.h"
#include <algorithm>
#include <cmath>

void CalcCmAdjust(CM* pcm, SO* psoFocus, CPDEFI* pcpdefi, glm::vec3* pdpos)
{
	SetVectorCylind(pdpos, pcpdefi->radHome + 3.1415927, pcpdefi->sAdjust, 0.0);
}

void SpringCm(CM* pcm, glm::vec3* ppos, glm::vec3* pv, const glm::vec3* pposTarget, float rSpring, float rDamping, float dt)
{
	if (pcm->fCut)
	{
		*ppos = *pposTarget;
		*pv = glm::vec3(0.0f);
		return;
	}

	const glm::vec3 dpos = *pposTarget - *ppos;
	const glm::vec3 dv = dpos * rSpring + *pv * rDamping;

	*pv += dv * dt;
	*ppos += *pv * dt;
}

void AdjustCmCpdefi(CM* pcm, JT* pjtFocus, float dt, CPDEFI* pcpdefi)
{
	glm::vec3 dposAdjustTarget;
	CalcCmAdjust(pcm, static_cast<SO*>(pjtFocus), pcpdefi, &dposAdjustTarget);

	if (pjtFocus == g_pjt)
	{
		switch (pjtFocus->jtfock)
		{
			case JTFOCK_Relax:
			SpringCm(pcm, &pcm->dposAdjust, &pcm->vAdjust, &dposAdjustTarget, 5.0f, -3.5f, dt);
			SpringCm(pcm, &pcm->dposFocus, &pcm->vFocus, &g_vecZero, 5.0f, -3.5f, dt);
			break;

			case JTFOCK_Clamp:
			SpringCm(pcm, &pcm->dposAdjust, &pcm->vAdjust, &dposAdjustTarget, 1.0f, -10.0f, dt);
			SpringCm(pcm, &pcm->dposFocus, &pcm->vFocus, &pcm->dposFocus, 0.0f, -10.0f, dt);
			break;

			case JTFOCK_Focus:
			SpringCm(pcm, &pcm->dposAdjust, &pcm->vAdjust, &dposAdjustTarget, 5.0f, -3.5f, dt);
			break;

			default:
			break;
		}
	}
	else
	{
		SpringCm(pcm, &pcm->dposAdjust, &pcm->vAdjust, &dposAdjustTarget, 5.0f, -3.5f, dt);
		SpringCm(pcm, &pcm->dposFocus, &pcm->vFocus, &g_vecZero, 5.0f, -3.5f, dt);
	}

	pcpdefi->posAdjust = pcpdefi->posBase + pcm->dposAdjust;
}

void ResetCmAdjust(CM* pcm, SO* psoFocus, CPDEFI* pcpdefi)
{
	CalcCmAdjust(pcm, psoFocus, pcpdefi, &pcm->dposAdjust);

	pcm->vFocus = glm::vec3(0.0f);
	pcm->vAdjust = glm::vec3(0.0f);
	pcm->dposFocus = glm::vec3(0.0f);
}

void GetCmCpdefi(CM* pcm, SO* psoFocus, float dt, CPDEFI* pcpdefi)
{
	(void)dt;

	if (psoFocus == pcm->psoFocusPrev)
	{
		*pcpdefi = pcm->cpdefiPrev;
		return;
	}

	psoFocus->pvtso->pfnGetSoCpdefi(psoFocus, dt, pcpdefi);

	glm::vec3 dposAdjust;
	CalcCmAdjust(pcm, psoFocus, pcpdefi, &dposAdjust);

	pcpdefi->posAdjust = pcpdefi->posBase + dposAdjust;
}

void RecalcCm(CM* pcm)
{
	const float framebufferWidth = static_cast<float>(g_gl.width);
	const float framebufferHeight = static_cast<float>(g_gl.height);
	const float camAspect = (g_gl.aspectMode == FitToScreen)
		? (framebufferWidth / framebufferHeight)
		: g_gl.aspectRatio;
	const float yScreenRange = tanf(pcm->radFOV * 0.5f);

	// These values are also the camera's world/screen conversion scale. The
	// original RecalcCmFrustrum refreshed them whenever the projection changed.
	pcm->yScreenRange = yScreenRange;
	pcm->xScreenRange = yScreenRange * camAspect;
	pcm->rMRDAdjust = std::max(1.0f, pcm->rMRD * (1.0f / pcm->radFOV));
	pcm->sRadiusNearClip = yScreenRange * sqrtf(camAspect * camAspect + 1.0f) * pcm->sNearClip + 1.0f;

	BuildProjectionMatrix(pcm->radFOV, camAspect, pcm->sNearClip, pcm->sFarClip, pcm->matProj);
	UpdateCmMat4(pcm);
}

CM* NewCm()
{
	return new CM{};
}

void InitCm(CM* pcm)
{
	InitLo(pcm);

	pcm->radFOVTarget = 1.0f;
	pcm->rMRD = g_drawDistanceMultiplier;
	pcm->radFOV = 1.0f;
	pcm->smpaRadFOV = s_smpaRadFOV;

	pcm->rAspect = 1.2987013f;
	pcm->sNearClip = 100.0f;
	pcm->sFarClip = 20000.0f;

	pcm->sNearFog = g_sNearFog;
	pcm->sFarFog = g_sFarFog;
	pcm->uFogMax = g_uFogMax;
	pcm->rgbaFog = g_rgbaFog;

	pcm->mat = glm::mat3(1.0f);

	RecalcCm(pcm);

	pcm->cpman.pvtcpman = &g_vtcpman;
	InitCplcy(&pcm->cpman, pcm);

	pcm->cplook.pvtcplook = &g_vtcplook;
	InitCplook(&pcm->cplook, pcm);

	pcm->cpalign.pvtcpalign = &g_vtcpalign;
	InitCpalign(&pcm->cpalign, pcm);

	pcm->cpaseg.pvtcpaseg = &g_vtcpaseg;
	InitCplcy(&pcm->cpaseg, pcm);

	pcm->cptn.pvtcptn = &g_vtcptn;
	InitCplcy(&pcm->cptn, pcm);

	ResetCm(pcm, 1);
}

int GetCmSize()
{
	return sizeof(CM);
}

void CloneCm(CM* pcm, CM* pcmBase)
{
	CloneLo(pcm, pcmBase);

	pcm->pos = pcmBase->pos;
	pcm->yaw = pcmBase->yaw;
	pcm->pitch = pcmBase->pitch;

	for (int i = 0; i < 3; ++i)
		pcm->anormalFrustrumTranspose[i] = pcmBase->anormalFrustrumTranspose[i];

	pcm->frustum = pcmBase->frustum;
	pcm->mat = pcmBase->mat;
	pcm->rMRDAdjust = pcmBase->rMRDAdjust;
	pcm->matProj = pcmBase->matProj;
	pcm->matWorldToClip = pcmBase->matWorldToClip;
	pcm->matClipToWorld = pcmBase->matClipToWorld;

	for (int i = 0; i < 4; ++i)
		pcm->anormalFrustrum[i] = pcmBase->anormalFrustrum[i];

	pcm->rMRD = pcmBase->rMRD;
	pcm->radFOV = pcmBase->radFOV;
	pcm->radFOVTarget = pcmBase->radFOVTarget;
	pcm->smpaRadFOV = pcmBase->smpaRadFOV;
	pcm->rAspect = pcmBase->rAspect;
	pcm->sNearClip = pcmBase->sNearClip;
	pcm->sFarClip = pcmBase->sFarClip;
	pcm->sRadiusNearClip = pcmBase->sRadiusNearClip;
	pcm->xScreenRange = pcmBase->xScreenRange;
	pcm->yScreenRange = pcmBase->yScreenRange;
	pcm->sNearFog = pcmBase->sNearFog;
	pcm->sFarFog = pcmBase->sFarFog;
	pcm->uFogMax = pcmBase->uFogMax;
	pcm->rgbaFog = pcmBase->rgbaFog;
	pcm->fgfn = pcmBase->fgfn;
	pcm->tJolt = pcmBase->tJolt;
	pcm->grfzon = pcmBase->grfzon;

	pcm->fCutNext = pcmBase->fCutNext;
	pcm->fCut = pcmBase->fCut;
	pcm->fRadCut = pcmBase->fRadCut;
	pcm->radCut = pcmBase->radCut;
	pcm->fDisplaced = pcmBase->fDisplaced;
	pcm->uPanProgress = pcmBase->uPanProgress;
	pcm->uTiltProgress = pcmBase->uTiltProgress;
	pcm->uSProgress = pcmBase->uSProgress;

	pcm->dposCenter = pcmBase->dposCenter;
	pcm->vCenter = pcmBase->vCenter;
	pcm->dposAdjust = pcmBase->dposAdjust;
	pcm->vAdjust = pcmBase->vAdjust;
	pcm->dposFocus = pcmBase->dposFocus;
	pcm->vFocus = pcmBase->vFocus;
	pcm->posScreen = pcmBase->posScreen;
	pcm->vScreen = pcmBase->vScreen;

	pcm->swPanPos = pcmBase->swPanPos;
	pcm->swTiltPos = pcmBase->swTiltPos;
	pcm->sv = pcmBase->sv;
	pcm->swPanMat = pcmBase->swPanMat;
	pcm->swTiltMat = pcmBase->swTiltMat;

	pcm->posCenterPrev = pcmBase->posCenterPrev;
	pcm->posClear = pcmBase->posClear;
	pcm->matClear = pcmBase->matClear;

	pcm->psoFocusPrev = pcmBase->psoFocusPrev;
	pcm->cpdefiPrev = pcmBase->cpdefiPrev;

	pcm->cpaloFade = pcmBase->cpaloFade;
	for (int i = 0; i < 8; ++i)
		pcm->apaloFade[i] = pcmBase->apaloFade[i];

	pcm->tActivateCplcy = pcmBase->tActivateCplcy;
	pcm->matRotateToCam = pcmBase->matRotateToCam;
	pcm->matRotateTiltToCam = pcmBase->matRotateTiltToCam;

	pcm->ccpr = pcmBase->ccpr;
	for (int i = 0; i < 8; ++i)
		pcm->acpr[i] = pcmBase->acpr[i];

	pcm->cpman = pcmBase->cpman;
	pcm->cplook = pcmBase->cplook;
	pcm->cpalign = pcmBase->cpalign;
	pcm->cpaseg = pcmBase->cpaseg;
	pcm->cptn = pcmBase->cptn;
}

void BuildProjectionMatrix(float fov, float aspectRatio, float near, float far, glm::mat4 &pmat)
{
	pmat = glm::perspective(fov, aspectRatio, near, far);
}

void SetSwCameraFov(SW* psw, float radFOV)
{
	SetCmFov(g_pcm, radFOV);
}

void SetSwCameraNearClip(SW* psw, float sNearClip)
{
	SetCmNearClip(g_pcm, sNearClip);
}

void SetSwCameraFarClip(SW* psw, float sFarClip)
{
	SetCmFarClip(g_pcm, sFarClip);
}

void SetSwCameraNearFog(SW* psw, float sNearFog)
{
	SetCmNearFog(g_pcm, sNearFog);
}

void SetSwCameraFarFog(SW* psw, float sFarFog)
{
	SetCmFarFog(g_pcm, sFarFog);
}

void SetSwCameraUFogMax(SW* psw, float uFogMax)
{
	SetCmUFogMax(g_pcm, uFogMax);
}

void SetSwCameraRgbaFog(SW* psw, RGBA prgbaFog)
{
	SetCmRgbaFog(g_pcm, prgbaFog);
}

void SetSwCameraMrdRatio(SW* psw, float rMRD)
{
	SetCmMrdRatio(g_pcm, rMRD);
}

void* GetCmPos(CM* pcm)
{
	return &pcm->pos;
}

void SetCmPos(CM* pcm, glm::vec3* ppos)
{
	SetCmPosMat(pcm, ppos, nullptr);
}

void* GetCmMat(CM* pcm)
{
	return &pcm->mat;
}

void SetCmMat(CM* pcm, glm::mat3* pmat)
{
	SetCmPosMat(pcm, nullptr, pmat);
}

void* GetCmFov(CM* pcm)
{
	return &pcm->radFOV;
}

void SetCmFov(CM* pcm, float radFOV)
{
	pcm->radFOVTarget = radFOV;
	pcm->radFOV = radFOV;
	pcm->svRadFOV = 0.0;
	RecalcCm(pcm);
}

void* GetCmRadFOVTarget(CM* pcm)
{
	return &pcm->radFOVTarget;
}

void SetCmRadFOVTarget(CM* pcm, float radFOVTarget)
{
	pcm->radFOVTarget = radFOVTarget;
}

void* GetCmSmpaRadFOV(CM* pcm)
{
	return &pcm->smpaRadFOV;
}

void SetCmSmpaRadFOV(CM* pcm, SMPA smpaRadFOV)
{
	pcm->smpaRadFOV = smpaRadFOV;
}

void* GetCmNearClip(CM* pcm)
{
	return &pcm->sNearClip;
}

void SetCmNearClip(CM* pcm, float sNearClip)
{
	pcm->sNearClip = sNearClip;
	RecalcCm(pcm);
}

void* GetCmFarClip(CM* pcm)
{
	return &pcm->sFarClip;
}

void SetCmFarClip(CM* pcm, float sFarClip)
{
	pcm->sFarClip = sFarClip;
	RecalcCm(pcm);
}

void* GetCmNearFog(CM* pcm)
{
	return &pcm->sNearFog;
}

void SetCmNearFog(CM* pcm, float sNearFog)
{
	pcm->sNearFog = sNearFog;
	RecalcCm(pcm);
}

void* GetCmFarFog(CM* pcm)
{
	return &pcm->sFarFog;
}

void SetCmFarFog(CM* pcm, float sFarFog)
{
	pcm->sFarFog = sFarFog;
	RecalcCm(pcm);
}

void* GetCmUFogMax(CM* pcm)
{
	return &pcm->uFogMax;
}

void SetCmUFogMax(CM* pcm, float uFogMax)
{
	pcm->uFogMax = uFogMax;
	RecalcCm(pcm);
}

void* GetCmRgbaFog(CM* pcm)
{
	return &pcm->rgbaFog;
}

void SetCmRgbaFog(CM* pcm, RGBA prgbaFog)
{
	float R = prgbaFog.bRed    / 255.0;
	float G = prgbaFog.bGreen  / 255.0;
	float B = prgbaFog.bBlue   / 255.0;
	float A = (prgbaFog.bAlpha / 255.0) * 2;

	pcm->rgbaFog = glm::vec4(R, G, B, A);
	RecalcCm(pcm);
}

void* GetCmMrdRatio(CM* pcm)
{
	return &pcm->rMRD;
}

void SetCmMrdRatio(CM* pcm, float rMRD)
{
	pcm->rMRD = rMRD;
	RecalcCm(g_pcm);
}

void ResetCm(CM* pcm, int fResetLookat)
{
	if (fResetLookat)
		SetCmLookAt(pcm, &g_posEyeDefault, &g_posCenterDefault);

	pcm->ccpr = 0;

	SetCpmanCpmt(&pcm->cpman, CPMT_Truck);
	SetCmPolicy(pcm, CPP_Base, &pcm->cpman, nullptr, nullptr);
	SetCmCut(pcm, nullptr);

	pcm->tJolt = -10.0f;
	pcm->grfzon = 0x0FFFFFFF;
}

void ClearCmFadeObjects(CM* pcm)
{
	pcm->cpaloFade = 0;
}

void AddCmFadeObject(CM* pcm, ALO* palo)
{
	if (pcm->cpaloFade < 8)
		pcm->apaloFade[pcm->cpaloFade++] = palo;
}

void RemoveCmFadeObject(CM* pcm, ALO* palo)
{
	for (int i = 0; i < pcm->cpaloFade; i++) {
		if (pcm->apaloFade[i] == palo)
		{
			memmove(&pcm->apaloFade[i], &pcm->apaloFade[i + 1], (pcm->cpaloFade - i - 1) * sizeof(ALO*));
			pcm->cpaloFade--;
			return;
		}
	}
}

int FFilterCamera(void* pv, SO* pso)
{
	// The original tests the complete packed CMK nibble, not only CMK_Fade.
	// CMK_Nil is -1, so compare against the one zero-valued mode explicitly.
	if (pso->cmk != CMK_Translucent)
		return 1;

	if (!pso->mpibspinpg.empty())
		return 1;

	return !pso->bspcCamera.absp.empty();
}

void UpdateCmFade(CM* pcm)
{
	glm::vec3 posMin = pcm->pos;
	glm::vec3 posMax = pcm->pos;

	for (int i = 0; i < pcm->cpaloFade; ++i)
	{
		ALO* paloFade = pcm->apaloFade[i];

		if (paloFade == nullptr || !paloFade->fBusy)
			continue;

		posMin = glm::min(posMin, paloFade->xf.posWorld);
		posMax = glm::max(posMax, paloFade->xf.posWorld);
	}

	std::vector <SO*> found;
	IntersectSwBoundingBox(g_psw, nullptr, &posMin, &posMax, (PFNFILTER)FFilterCamera, nullptr, found);

	for (SO* pso : found)
	{
		if (pso == nullptr)
			continue;

		for (int i = 0; i < pcm->cpaloFade; ++i)
		{
			ALO* paloFade = pcm->apaloFade[i];

			if (paloFade == nullptr || !paloFade->fForceCameraFade)
				continue;

			LSG alsg[32];
			const int clsg = ClsgClipEdgeToBsp(pso->bspc.absp.data(), &pcm->pos, &paloFade->xf.posWorld, (int*)pso->mpibspinpg.data(), 32, alsg);

			for (int ilsg = 0; ilsg < clsg; ++ilsg)
			{
				int inpg = alsg[ilsg].id.n;

				if (inpg < 0 || inpg >= pso->cnpg)
					continue;

				while (inpg < pso->cnpg)
				{
					NPG& npg = pso->anpg[inpg];

					if (npg.ipglob >= 0)
						pso->globset.aglobi[npg.ipglob].tUnfade = g_clock.tReal + 0.1f;

					++inpg;

					if (inpg >= pso->cnpg)
						break;

					// Negative CMK entries inherit the previous explicit material.
					if (pso->anpg[inpg].cmk >= 0)
						break;
				}
			}
		}
	}
}

void UpdateCmLast(CM* pcm, int fClearCut, float dt)
{
	if (pcm->fCutNext)
	{
		pcm->fCutNext = 0;
		pcm->fCut = 1;
		pcm->fRadCut = 0;
	}

	if (pcm->ccpr > 0)
	{
		CPLCY* pcplcy = pcm->acpr[0].pcplcy;
		CPLCY* pcplcyPrev = nullptr;
		SO* psoFocus = pcm->acpr[0].psoFocus;
		SO* psoFocusProcessed = nullptr;

		while (pcplcy)
		{
			CPDEFI* pcpdefi = nullptr;

			if (psoFocus && psoFocus != psoFocusProcessed)
			{
				pcpdefi = &pcm->cpdefiPrev;
				psoFocus->pvtso->pfnGetSoCpdefi(psoFocus, dt, pcpdefi);

				if (pcplcyPrev || psoFocus != pcm->psoFocusPrev)
					ResetCmAdjust(pcm, psoFocus, pcpdefi);

				AdjustCmCpdefi(pcm, (JT*)psoFocus, dt, pcpdefi);

				pcm->psoFocusPrev = psoFocus;
				psoFocusProcessed = psoFocus;
			}

			if (pcplcy->pvtcplcy->pfnUpdateCplcy)
			{
				JOY* pjoy = (g_grfjoyt & 1) ? &g_joy : &g_joyZero;
				pcplcy->pvtcplcy->pfnUpdateCplcy(pcplcy, pcpdefi, pjoy, dt);
			}

			if (pcm->ccpr < 1 || pcm->acpr[0].pcplcy == pcplcy)
				break;

			pcplcyPrev = pcplcy;
			pcplcy = pcm->acpr[0].pcplcy;
			psoFocus = pcm->acpr[0].psoFocus;
		}
	}

	if (pcm->fCut)
		SetCmFov(pcm, pcm->radFOVTarget);
	else if (pcm->radFOV != pcm->radFOVTarget)
	{
		pcm->radFOV = GSmoothA(pcm->radFOV, pcm->svRadFOV, pcm->radFOVTarget, dt, &pcm->smpaRadFOV, &pcm->svRadFOV);
		RecalcCm(pcm);
	}

	if (fClearCut)
	{
		pcm->fCut = 0;
		pcm->fRadCut = 0;
	}
}

void SetupCmRotateToCam(CM* pcm)
{
	// Flatten the camera's negative X axis onto the XY plane.
	glm::vec3 vecToCam(-pcm->mat[0].x, -pcm->mat[0].y, 0.0f);

	const float length = glm::length(vecToCam);

	if (length < 0.0001f)
		vecToCam = g_normalX;
	else
		vecToCam /= length;

	const glm::vec3 vecSide = glm::cross(g_normalZ, vecToCam);

	pcm->matRotateToCam[0] = vecToCam;
	pcm->matRotateToCam[1] = vecSide;
	pcm->matRotateToCam[2] = g_normalZ;

	pcm->matRotateTiltToCam[0] = -pcm->mat[0];
	pcm->matRotateTiltToCam[1] = -pcm->mat[1];
	pcm->matRotateTiltToCam[2] = pcm->mat[2];
}

void ConvertCmScreenToWorld(CM* pcm, const glm::vec3* pposScreen, glm::vec3* pposWorld)
{
	float depth = pposScreen->z;
	float x = -pposScreen->x;
	float y = pposScreen->y;

	if (depth > 0.0001f)
	{
		x *= depth;
		y *= depth;
	}

	glm::vec3 posLocal(depth, x * pcm->xScreenRange, y * pcm->yScreenRange);
	*pposWorld = pcm->pos + pcm->mat * posLocal;
}

void ConvertCmWorldToScreen(CM* pcm, const glm::vec3* pposWorld, glm::vec3* pposScreen)
{
	glm::vec3 posLocal = glm::transpose(pcm->mat) * (*pposWorld - pcm->pos);

	pposScreen->x = -posLocal.y / pcm->xScreenRange;
	pposScreen->y = posLocal.z / pcm->yScreenRange;
	pposScreen->z = posLocal.x;

	if (posLocal.x > 0.0001f)
	{
		pposScreen->x /= posLocal.x;
		pposScreen->y /= posLocal.x;
	}
}

void SetupCm(CM* pcm)
{
	SetupCmRotateToCam(pcm);

	if (g_psw != nullptr)
	{
		GRFZON grfzonNew{};
		ClipVismapPointNoHop(g_psw->pvismap, &pcm->pos, &grfzonNew);

		if (grfzonNew != 0)
			pcm->grfzon = grfzonNew;
	}
}

void CombineEyeLookAtProj(const glm::vec3& pposEye, const glm::mat3& pmatLookAt, const glm::mat4& pmatProj, glm::mat4& pmatOut)
{
	const glm::mat3 glBasis = ConvertGameCameraBasisToGl(pmatLookAt);

	glm::mat4 eye(1.0f);

	eye[0] = glm::vec4(glBasis[0], 0.0f);
	eye[1] = glm::vec4(glBasis[1], 0.0f);
	eye[2] = glm::vec4(glBasis[2], 0.0f);
	eye[3] = glm::vec4(pposEye, 1.0f);

	pmatOut = pmatProj * glm::inverse(eye);
}

void BuildFrustrum(const glm::mat3& pmatLookAt, float rx, float ry, glm::vec3* anormalFrustrum)
{
	const glm::vec3& X = pmatLookAt[0];
	const glm::vec3& Y = pmatLookAt[1];
	const glm::vec3& Z = pmatLookAt[2];

	// +X (right side)
	{
		glm::vec3 axis = glm::normalize(X + Z * rx);
		anormalFrustrum[0] = glm::normalize(glm::cross(Z, axis));
	}

	// -X (left side)
	{
		glm::vec3 axis = glm::normalize(X - Z * rx);
		anormalFrustrum[1] = glm::normalize(glm::cross(axis, Z));
	}

	// +Y (top side)
	{
		glm::vec3 axis = glm::normalize(Y + Z * ry);
		anormalFrustrum[2] = glm::normalize(glm::cross(axis, X));
	}

	// -Y (bottom side)
	{
		glm::vec3 axis = glm::normalize(Y - Z * ry);
		anormalFrustrum[3] = glm::normalize(glm::cross(X, axis));
	}
}

void UpdateCmMat4(CM* pcm)
{
	// Retail data at 0x002618c8-0x0026191f. The position, projection-range,
	// and projection-offset vectors are all zero in this build, so the retail
	// jolt is a 0.05-radian rotation around the camera's local X axis.
	constexpr float dtJoltMax = 0.5f;
	constexpr float swJolt = 50.0f;
	const glm::vec3 rotJolt(0.05f, 0.0f, 0.0f);

	const float dtJolt = g_clock.t - pcm->tJolt;
	glm::mat3 matView = pcm->mat;

	if (dtJolt >= 0.0f && dtJolt <= dtJoltMax)
	{
		const float u = dtJolt / dtJoltMax;
		// 1 - 2u + u^2 == (1-u)^2: a smooth quadratic decay.
		float envelope = 1.0f + u * (-2.0f + u);
		if (envelope < 0.0f)
			envelope = 0.0f;
		else if (envelope > 1.0f)
			envelope = 1.0f;

		const float jolt = sinf(dtJolt * swJolt) * envelope;
		glm::vec3 rot = rotJolt * jolt;
		glm::mat3 matJolt;
		LoadRotateMatrix(&rot, &matJolt);
		matView = pcm->mat * matJolt;
	}

	CombineEyeLookAtProj(pcm->pos, matView, pcm->matProj, pcm->matWorldToClip);
	pcm->matClipToWorld = glm::inverse(pcm->matWorldToClip);

	// Retail deliberately builds these legacy normals from the unjolted camera
	// basis, keeping gameplay visibility stable while only the rendered view shakes.
	BuildFrustrum(pcm->mat, pcm->xScreenRange, pcm->yScreenRange, pcm->anormalFrustrum);
	for (int component = 0; component < 3; ++component)
	{
		pcm->anormalFrustrumTranspose[component] = glm::vec4(
			pcm->anormalFrustrum[0][component],
			pcm->anormalFrustrum[1][component],
			pcm->anormalFrustrum[2][component],
			pcm->anormalFrustrum[3][component]);
	}

	ExtractFrustumPlanes(pcm->matWorldToClip, &pcm->frustum);
}

void SetCmPosMat(CM* pcm, const glm::vec3* ppos, const glm::mat3* pmat)
{
	if (ppos != nullptr)
		pcm->pos = *ppos;

	if (pmat != nullptr)
		pcm->mat = *pmat;

	UpdateCmMat4(pcm);

	pcm->sv = 0.0f;
	pcm->swTiltMat = 0.0f;
	pcm->swPanMat = 0.0f;
	pcm->swTiltPos = 0.0f;
	pcm->swPanPos = 0.0f;
}

void SetCmLookAt(CM* pcm, const glm::vec3* pposEye, const glm::vec3* pposCenter)
{
	SetCmLookAtSmooth(pcm, 0, pposEye, pposCenter, nullptr, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

void ConvertWorldToCylindVelocity(const glm::vec3* pposCenter, const glm::vec3* ppos, const glm::vec3* pv, float* pswPan, float* pswTilt, float* psv)
{
	glm::vec3 dpos = *ppos - *pposCenter;
	float radius = glm::length(dpos);

	if (radius < 0.0001f)
	{
		*pswPan = 0.0f;
		*pswTilt = 0.0f;
		*psv = 0.0f;
		return;
	}

	glm::mat3 mat;
	LoadLookAtMatrix(&dpos, &mat);

	glm::vec3 vLocal = glm::transpose(mat) * *pv;
	radius = glm::max(radius, 100.0f);

	*psv = vLocal.x;
	*pswPan = vLocal.y / radius;
	*pswTilt = vLocal.z / radius;
}

void ConvertCylindToWorldVelocity(const glm::vec3* pposCenter, const glm::vec3* ppos, float swPan, float swTilt, float sv, glm::vec3* pv)
{
	glm::vec3 dpos = *ppos - *pposCenter;
	float radius = glm::length(dpos);

	glm::mat3 mat;
	LoadLookAtMatrix(&dpos, &mat);

	glm::vec3 vLocal(sv, radius * swPan, radius * swTilt);
	*pv = mat * vLocal;
}

void ResetCmLookAtSmooth(CM* pcm, const glm::vec3* pposCenter)
{
	glm::vec3 v;
	ConvertCylindToWorldVelocity(&pcm->posCenterPrev, &pcm->pos, pcm->swPanPos, pcm->swTiltPos, pcm->sv, &v);
	ConvertWorldToCylindVelocity(pposCenter, &pcm->pos, &v, &pcm->swPanPos, &pcm->swTiltPos, &pcm->sv);
	pcm->posCenterPrev = *pposCenter;
}

void SetCmLookAtSmooth(CM* pcm, int grflas, const glm::vec3* pposEye, const glm::vec3* pposCenter, SO* psoFocus, float rSpring, float rDamping, float rLimit, float xScreenPref, float yScreenPref, float dt)
{
	(void)psoFocus;
	(void)rLimit;

	if (pcm->fCut)
		grflas = 0;

	float radPanPosTarget;
	float radTiltPosTarget;
	float sTarget;
	DecomposeSphere(pposEye, pposCenter, &radPanPosTarget, &radTiltPosTarget, &sTarget);

	float radPanMatTarget = radPanPosTarget + glm::pi<float>() + xScreenPref * pcm->radFOV * pcm->rAspect * 0.5f;
	float radTiltMatTarget = -radTiltPosTarget - yScreenPref * pcm->radFOV * 0.5f;

	float radPanPosCur;
	float radTiltPosCur;
	float sCur;
	DecomposeSphere(&pcm->pos, &pcm->posCenterPrev, &radPanPosCur, &radTiltPosCur, &sCur);

	float radPanMatCur;
	float radTiltMatCur;
	DecomposeRotateMatrixPanTilt(&pcm->mat, &radPanMatCur, &radTiltMatCur);

	float radPanPos = radPanPosTarget;
	float radTiltPos = radTiltPosTarget;
	float s = sTarget;
	float radPanMat = radPanMatTarget;
	float radTiltMat = radTiltMatTarget;

	if ((grflas & 1) != 0)
	{
		float dradPanPos = RadNormalize(radPanPosTarget - radPanPosCur);
		float dradTiltPos = radTiltPosTarget - radTiltPosCur;
		float ds = sTarget - sCur;
		float dradPanMat = RadNormalize(radPanMatTarget - radPanMatCur);
		float dradTiltMat = radTiltMatTarget - radTiltMatCur;

		if (glm::abs(dradPanPos) > 2.0f && glm::abs(dradPanMat) > 2.0f && dradPanPos * dradPanMat < 0.0f)
		{
			if (glm::abs(dradPanMat) < glm::abs(dradPanPos))
				dradPanMat += dradPanMat < 0.0f ? glm::two_pi<float>() : -glm::two_pi<float>();
			else
				dradPanPos += dradPanPos < 0.0f ? glm::two_pi<float>() : -glm::two_pi<float>();
		}

		float scale = 1.0f;
		scale = glm::max(scale, glm::abs(dradPanPos) * 0.6666667f);
		scale = glm::max(scale, glm::abs(dradTiltPos) * 1.3333334f);
		scale = glm::max(scale, glm::abs(ds) * 0.002f);
		scale = glm::max(scale, glm::abs(dradPanMat) * 0.6666667f);
		scale = glm::max(scale, glm::abs(dradTiltMat) * 1.3333334f);

		pcm->swPanMat += ((dradPanMat / scale) * rSpring + pcm->swPanMat * rDamping) * dt;
		pcm->swPanPos += ((dradPanPos / scale) * rSpring + pcm->swPanPos * rDamping) * dt;
		pcm->swTiltPos += ((dradTiltPos / scale) * rSpring + pcm->swTiltPos * rDamping) * dt;
		pcm->sv += ((ds / scale) * rSpring + pcm->sv * rDamping) * dt;
		pcm->swTiltMat += ((dradTiltMat / scale) * rSpring + pcm->swTiltMat * rDamping) * dt;

		radPanPos = radPanPosCur + pcm->swPanPos * dt;
		radTiltPos = radTiltPosCur + pcm->swTiltPos * dt;
		s = sCur + pcm->sv * dt;
		radPanMat = radPanMatCur + pcm->swPanMat * dt;
		radTiltMat = radTiltMatCur + pcm->swTiltMat * dt;
	}

	glm::vec3 dpos;
	SetVectorSphere(&dpos, radPanPos, radTiltPos, s);

	pcm->pos = *pposCenter + dpos;
	LoadRotateMatrixPanTilt(radPanMat, radTiltMat, &pcm->mat);
	UpdateCmMat4(pcm);

	pcm->posCenterPrev = *pposCenter;
}

void AdjustCmJoy(CM* pcm, JOY* pjoy, JOYID joyid, float* prad)
{
	float radJoy = 0.0f;

	if (joyid == JOYID_Left)
		radJoy = atan2f(-pjoy->x, pjoy->y);
	else if (joyid == JOYID_Right)
		radJoy = atan2f(-pjoy->x2, pjoy->y2);

	float radCamera = atan2f(pcm->mat[0].y, pcm->mat[0].x);
	*prad = RadNormalize(radJoy + radCamera);
}

void SetCmPolicy(CM* pcm, CPP cpp, CPLCY* pcplcy, SO* psoFocus, void* pv)
{
	int icpr = 0;

	while (icpr < pcm->ccpr && pcm->acpr[icpr].cpp >= cpp)
		++icpr;

	std::memmove(&pcm->acpr[icpr + 1], &pcm->acpr[icpr], static_cast<size_t>(pcm->ccpr - icpr) * sizeof(CPR));

	pcm->acpr[icpr].cpp = cpp;
	pcm->acpr[icpr].pcplcy = pcplcy;
	pcm->acpr[icpr].psoFocus = psoFocus;
	pcm->acpr[icpr].pv = pv;
	++pcm->ccpr;

	if (pcplcy->pvtcplcy->pfnSetCplcy != nullptr)
		pcplcy->pvtcplcy->pfnSetCplcy(pcplcy, pv);

	if (icpr == 0)
	{
		if (pcm->ccpr > 1)
		{
			CPR& cprPrevious = pcm->acpr[1];

			if (cprPrevious.pcplcy->pvtcplcy->pfnDeactivateCplcy != nullptr)
				cprPrevious.pcplcy->pvtcplcy->pfnDeactivateCplcy(cprPrevious.pcplcy, cprPrevious.pv);
		}

		pcm->tActivateCplcy = g_clock.t;

		if (pcplcy->pvtcplcy->pfnActivateCplcy != nullptr)
			pcplcy->pvtcplcy->pfnActivateCplcy(pcplcy, pv);
	}
}

void RevokeCmPolicy(CM* pcm, GRFRCP grfrcp, CPP cpp, CPLCY* pcplcy, SO* psoFocus, void* pv)
{
	CPLCY* pcplcyActiveRemoved = nullptr;
	void* pvActiveRemoved = nullptr;
	int ccprKept = 0;
	int ccprOld = pcm->ccpr;

	for (int icpr = 0; icpr < ccprOld; ++icpr)
	{
		CPR& cpr = pcm->acpr[icpr];

		bool fMatch = true;

		if ((grfrcp & 1) != 0 && cpr.cpp != cpp)
			fMatch = false;

		if ((grfrcp & 2) != 0 && cpr.pcplcy != pcplcy)
			fMatch = false;

		if ((grfrcp & 4) != 0 && cpr.psoFocus != psoFocus)
			fMatch = false;

		if ((grfrcp & 8) != 0 && cpr.pv != pv)
			fMatch = false;

		if (!fMatch)
		{
			pcm->acpr[ccprKept++] = cpr;
			continue;
		}

		if (icpr == 0)
		{
			pcplcyActiveRemoved = cpr.pcplcy;
			pvActiveRemoved = cpr.pv;
		}

		if (cpr.pcplcy->pvtcplcy->pfnRevokeCplcy != nullptr)
			cpr.pcplcy->pvtcplcy->pfnRevokeCplcy(cpr.pcplcy, cpr.pv);
	}

	pcm->ccpr = ccprKept;

	if (pcplcyActiveRemoved != nullptr)
	{
		if (pcplcyActiveRemoved->pvtcplcy->pfnDeactivateCplcy != nullptr)
			pcplcyActiveRemoved->pvtcplcy->pfnDeactivateCplcy(pcplcyActiveRemoved, pvActiveRemoved);

		if (pcm->ccpr > 0)
		{
			CPR& cprActive = pcm->acpr[0];
			pcm->tActivateCplcy = g_clock.t;

			if (cprActive.pcplcy->pvtcplcy->pfnActivateCplcy != nullptr)
				cprActive.pcplcy->pvtcplcy->pfnActivateCplcy(cprActive.pcplcy, cprActive.pv);
		}
	}
}

bool FInsideCmMrd(const CM* pcm, const glm::vec3& dpos, float sRadius, float sMRD, float& outAlpha)
{
	float outer = (sMRD * pcm->rMRDAdjust) + sRadius;
	float outerSq = outer * outer;

	float distSq = glm::dot(dpos, dpos);

	if (distSq > outerSq)
		return false;

	float inner = outer - pcm->rMRDAdjust * 400.0f;
	float innerSq = inner * inner;

	if (distSq > innerSq)
		outAlpha = (outerSq - distSq) / (outerSq - innerSq);
	else
		outAlpha = 1.0f;

	return true;
}


void SetCmCut(CM* pcm, float* pradCut)
{
	pcm->fCut = 1;
	if (pradCut != nullptr) 
	{
		pcm->fRadCut = 1;
		pcm->radCut = *pradCut;
	}
}

void SetCmCutNext(CM* pcm)
{
	pcm->fCutNext = 1;
}

void CutCm(CM* pcm)
{
	SetCmCut(pcm, nullptr);
}

void AdaptCm(CM* pcm, const glm::vec3* pposCenter, float xScreen, float yScreen)
{
	pcm->vCenter = glm::vec3(0.0f);

	glm::vec3 vecLocal(1.0f, -xScreen * pcm->xScreenRange, yScreen * pcm->yScreenRange);
	glm::vec3 posLineEnd = pcm->pos + pcm->mat * vecLocal * 100.0f;

	float u;
	float s;
	FindClosestPointOnLine(pposCenter, &pcm->pos, &posLineEnd, &u, &s);

	glm::vec3 posClosest = glm::mix(pcm->pos, posLineEnd, u);
	pcm->dposCenter = posClosest - *pposCenter;
}

void SetCm(CM* pcm, const glm::vec3* pposCenter, const glm::vec3* pposEye, float xScreen, float yScreen)
{
	pcm->pos = *pposEye;

	SpringCm(pcm, &pcm->dposCenter, &pcm->vCenter, &g_vecZero,
		SPRING_CptnPosition, DAMP_CptnPosition, g_clock.dt);

	glm::vec3 posCenter = *pposCenter + pcm->dposCenter;

	float radPan;
	float radTilt;
	float s;
	DecomposeSphere(&posCenter, pposEye, &radPan, &radTilt, &s);

	radPan += atanf(xScreen * pcm->xScreenRange);
	radTilt -= atanf(yScreen * pcm->yScreenRange);

	LoadRotateMatrixPanTilt(radPan, radTilt, &pcm->mat);
	UpdateCmMat4(pcm);
}

int FFindCmClearInterval(float duMin, int cpso, SO** apso, glm::vec3* pposFrom, glm::vec3* pposTo, float* puFrom, float* puTo)
{
	LSG alsg[64];
	int clsg = 0;

	for (int ipso = 0; ipso < cpso && clsg < 64; ++ipso)
	{
		SO* pso = apso[ipso];

		if (pso == nullptr || pso->bspcCamera.absp.size() == 0)
			continue;

		clsg += ClsgClipEdgeToBsp(pso->bspcCamera.absp.data(), pposFrom, pposTo, nullptr, 64 - clsg, alsg + clsg);
	}

	clsg = ClsgMergeAlsg(clsg, alsg);

	for (int ilsg = 0; ilsg <= clsg; ++ilsg)
	{
		float uFrom = ilsg > 0 ? alsg[ilsg - 1].au[1] : 0.0f;
		float uTo = ilsg < clsg ? alsg[ilsg].au[0] : 1.0f;

		if (uTo - uFrom <= duMin)
			continue;

		if (puFrom != nullptr)
			*puFrom = uFrom;

		if (puTo != nullptr)
			*puTo = uTo;

		return 1;
	}

	return 0;
}

void FindCmClosestClearPos(float sRadius, int cpso, SO** apso, glm::vec3* ppos, glm::vec3* pposClear)
{
	const float duPadding = 5.0f / sRadius;
	const float radTiltStep = glm::pi<float>() * 0.125f;

	glm::vec3 posBest = *ppos;
	float uBest = FLT_MAX;

	for (int iTilt = -4; iTilt <= 4; ++iTilt)
	{
		float cSample = s_acCmClearSamples[iTilt + 4];

		if (cSample <= 0.0f)
			continue;

		float radTilt = static_cast<float>(iTilt) * radTiltStep;

		for (int iSample = 0; static_cast<float>(iSample) < cSample; ++iSample)
		{
			float radPan = static_cast<float>(iSample) * glm::pi<float>() / cSample;

			glm::vec3 dpos;
			SetVectorSphere(&dpos, radPan, radTilt, sRadius);

			glm::vec3 posTest = *ppos + dpos;
			float uFrom;
			float uTo;

			if (!FFindCmClearInterval(duPadding * 2.0f, cpso, apso, ppos, &posTest, &uFrom, &uTo))
				continue;

			if (uFrom > 0.0f)
				uFrom += duPadding;

			if (uFrom >= uBest)
				continue;

			posBest = glm::mix(*ppos, posTest, uFrom);
			uBest = uFrom;
		}
	}

	*pposClear = posBest;
}

void SquishCmEye(glm::vec3* pposEyePrev, float sdposMax, void* pvContext, int cpso, SO** apso, PFNSQUISHCMEYE pfnConstraint, float sRadius, const glm::vec3* pposEyeNext, glm::vec3* pposEyeClip)
{
	SBI asbi[64];
	int csbi = 0;

	for (int ipso = 0; ipso < cpso && csbi < 64; ++ipso)
	{
		SO* pso = apso[ipso];

		if (pso == nullptr || pso->bspcCamera.absp.size() == 0)
			continue;

		csbi += CsbiIntersectSphereBsp(pposEyePrev, sRadius, pso->bspcCamera.cbspFull, pso->bspcCamera.absp.data(), nullptr, &pso->geomCameraWorld, 64 - csbi, asbi + csbi);
	}

	if (csbi == 0)
	{
		*pposEyeClip = *pposEyeNext;
		return;
	}

	// A tessellated wall commonly reports the same plane through more than one
	// SURF.  Passing those parallel constraints to SolveInequalities produces a
	// singular Gram matrix and lets tiny contact-order changes move the camera
	// between different solutions on consecutive frames.  Retail's active-set
	// solver effectively keeps one active copy; do that explicitly here.
	std::vector<glm::vec3> anormal;
	std::vector<float> asvMin;
	anormal.reserve(csbi);
	asvMin.reserve(csbi);

	for (int i = 0; i < csbi; ++i)
	{
		const glm::vec3 normal = asbi[i].normal;
		const float svMin = pfnConstraint(pvContext, sRadius, pposEyeNext, &asbi[i]);

		if (!std::isfinite(normal.x) || !std::isfinite(normal.y) ||
			!std::isfinite(normal.z) || !std::isfinite(svMin))
			continue;

		int iEquivalent = -1;
		for (int j = 0; j < static_cast<int>(anormal.size()); ++j)
		{
			if (glm::dot(normal, anormal[j]) > 0.9999f)
			{
				iEquivalent = j;
				break;
			}
		}

		if (iEquivalent < 0)
		{
			anormal.push_back(normal);
			asvMin.push_back(svMin);
		}
		else
		{
			// A negative value is a violated constraint, so retain the most
			// negative (most restrictive) copy of a parallel contact.
			if (svMin < asvMin[iEquivalent])
				asvMin[iEquivalent] = svMin;
		}
	}

	const int cConstraint = static_cast<int>(anormal.size());

	if (cConstraint == 0)
	{
		*pposEyeClip = *pposEyeNext;
		return;
	}

	glm::vec3 dposClip(0.0f);

	// Enforce the same inequalities as the retail active-set solve:
	//     asvMin[i] + dot(dposClip, normal[i]) >= 0
	// Projecting onto each violated half-space is deterministic for duplicate
	// and nearly dependent planes, unlike the port's singular Crout solve.
	for (int iteration = 0; iteration < 8; ++iteration)
	{
		bool fAdjusted = false;

		for (int i = 0; i < cConstraint; ++i)
		{
			const float gConstraint = asvMin[i] + glm::dot(dposClip, anormal[i]);
			if (gConstraint < -0.0001f)
			{
				dposClip -= anormal[i] * gConstraint;
				fAdjusted = true;
			}
		}

		if (!fAdjusted)
			break;
	}

	float sdposClip = glm::length(dposClip);

	if (sdposClip > sdposMax && sdposClip > 0.0001f)
		dposClip *= sdposMax / sdposClip;

	*pposEyeClip = *pposEyeNext + dposClip;
}

bool FCmTunnelHome(void* pv, CM* pcm)
{
	return pcm->cptn.fHome != 0;
}

float SvSquishCmEyeConstraint(void* pvContext, float sRadius, const glm::vec3* pvEye, const SBI* psbi)
{
	CM* pcm = static_cast<CM*>(pvContext);
	float sv;

	(void)sRadius;

	GSmooth(psbi->gDist, pcm->sRadiusNearClip - S_CmSquishEye, g_clock.dtReal, &s_smpSquishEye, &sv);

	return glm::dot(*pvEye, psbi->normal) - sv;
}

void ClipCmEye(CM* pcm, const glm::vec3* pposEyePrev, glm::vec3* pposEyeNext, glm::vec3* pposEyeClip)
{
	const glm::vec3 posEyeRequested = *pposEyeNext;
	glm::vec3 vEye = (posEyeRequested - *pposEyePrev) / g_clock.dtReal;

	glm::vec3 posMin = *pposEyePrev + s_dposCmSquishMin;
	glm::vec3 posMax = *pposEyePrev + s_dposCmSquishMax;

	std::vector <SO*> apso;
	IntersectSwBoundingBox(g_psw, nullptr, &posMin, &posMax, (PFNFILTER)FFilterCamera, nullptr, apso);

	glm::vec3 vEyeClip;
	SquishCmEye(const_cast<glm::vec3*>(pposEyePrev), glm::length(vEye) + SV_CmSquishEyeSlack, pcm, static_cast<int>(apso.size()), apso.data(), SvSquishCmEyeConstraint, S_CmSquishEye, &vEye, &vEyeClip);

	*pposEyeClip = *pposEyePrev + vEyeClip * g_clock.dtReal;
}

void PushCmLookk(CM* pcm, LOOKK lookk)
{
	PushCplookLookk(&pcm->cplook, lookk);
}

LOOKK LookkPopCm(CM* pcm)
{
	return LookkPopCplook(&pcm->cplook);
}

LOOKK LookkCurCm(CM* pcm)
{
	return LookkCurCplook(&pcm->cplook);
}

void SetCmSniperFocus(CM* pcm, ALO* palo, float rScreen, float sRadius)
{
	pcm->cplook.sRadiusSniper = sRadius;
	pcm->cplook.paloFocusSniper = palo;
	pcm->cplook.rScreenSniper = rScreen;
}

void JoltCm(CM* pcm)
{
	pcm->tJolt = g_clock.t;
}

bool FVisibleCmSphere(const CM* pcm, const glm::vec3& pos, float sRadius, float sMRD, float& outAlpha)
{
	if (!SphereInFrustum(pcm->frustum, pos, sRadius))
		return false;

	glm::vec3 dpos = pos - pcm->pos;
	return FInsideCmMrd(pcm, dpos, sRadius, sMRD, outAlpha);
}

void BuildLookAt(glm::vec3 &posEye, glm::vec3 &directionEye, glm::vec3 &upEye ,glm::mat4 &pmatLookAt)
{
	pmatLookAt = glm::identity <glm::mat4>();
	pmatLookAt = glm::lookAt(posEye, posEye + directionEye, upEye);
}

void UnlockCm(int nParam)
{
	g_cmlk = CMLK_Nil;
}

glm::mat3 ConvertGameCameraBasisToGl(const glm::mat3& gameBasis)
{
	glm::mat3 glBasis(1.0f);

	glBasis[0] = -gameBasis[1]; // right
	glBasis[1] = gameBasis[2]; // up
	glBasis[2] = -gameBasis[0]; // backward: OpenGL looks down -Z

	return glBasis;
}

glm::mat3 ConvertGlCameraBasisToGame(const glm::mat3& glBasis)
{
	glm::mat3 gameBasis(1.0f);

	gameBasis[0] = -glBasis[2];
	gameBasis[1] = -glBasis[0];
	gameBasis[2] = glBasis[1];

	return gameBasis;
}

void ExtractFrustumPlanes(const glm::mat4& viewProj, FRUSTUM* pfrustum)
{
	// Transpose the matrix once to access rows
	glm::mat4 m = glm::transpose(viewProj);

	// Left
	pfrustum->planes[0] = m[3] + m[0];
	// Right
	pfrustum->planes[1] = m[3] - m[0];
	// Bottom
	pfrustum->planes[2] = m[3] + m[1];
	// Top
	pfrustum->planes[3] = m[3] - m[1];
	// Near
	pfrustum->planes[4] = m[3] + m[2];
	// Far
	pfrustum->planes[5] = m[3] - m[2];

	// Normalize the planes
	for (int i = 0; i < 6; i++)
	{
		float length = glm::length(glm::vec3(pfrustum->planes[i]));

		if (length != 0.0f)
			pfrustum->planes[i] /= length;
	}
}

bool SphereInFrustum(const FRUSTUM& frustum, const glm::vec3& center, float radius)
{
	for (int i = 0; i < 6; i++)
	{
		float distance = glm::dot(glm::vec3(frustum.planes[i]), center) + frustum.planes[i].w;

		if (distance < -radius)
			return false;
	}

	return true;
}

void DeleteCm(CM *pcm)
{
	delete pcm;
}

CMLK g_cmlk;
glm::vec3 g_posEyeDefault{0.0, -2000, 500};
glm::vec3 g_posCenterDefault(0.0);
float R_SetCmDamping = -8.0;
float R_SetCmSpring = 15.0;
float s_acCmClearSamples[9] =
{
	1.0f,
	5.0f,
	10.0f,
	12.0f,
	15.0f,
	12.0f,
	10.0f,
	5.0f,
	1.0f
};

glm::vec3 s_dposCmSquishMin = {-250, -250, -250};
glm::vec3 s_dposCmSquishMax = {250, 250, 250};
// Retail data at 0x00261958: { 2500.0f, 0.0f, 0.25f }.
// svFast is also the camera collision sphere radius, while svSlow is the
// additional correction-length allowance used by ClipCmEye.
float SV_CmSquishEyeSlack = 0.0f;
float S_CmSquishEye = 2500.0f;
SMP s_smpSquishEye = {2500.0f, 0.0f, 0.25f};
SMPA s_smpaRadFOV = {0.5, 0.0, 0.1, 10.0};
float g_uFogMax = 0.5;
float g_sNearFog = 500;
float g_sFarFog = 20000;
glm::vec4 g_rgbaFog = glm::vec4{0.0};
