#include "cm.h"

CM* NewCm()
{
	return new CM{};
}

void InitCm(CM* pcm)
{
	InitLo(pcm);

	pcm->radFOV = 1.0;
	pcm->rAspect = 1.298701;
	pcm->sNearClip = 100.0;
	pcm->sFarClip = 20000.0;
	pcm->sNearFog = 500.0;
	pcm->sFarFog = 20000.0;
	pcm->uFogMax = 0.5;
	pcm->rMRD = 1.0;
	pcm->worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

	pcm->direction = glm::vec3(0.0);
	pcm->direction.x = cos(glm::radians(-pcm->yaw)) * cos(glm::radians(pcm->pitch));
	pcm->direction.y = sin(glm::radians(-pcm->yaw)) * cos(glm::radians(pcm->pitch));
	pcm->direction.z = sin(glm::radians(pcm->pitch));
	pcm->direction = glm::normalize(pcm->direction);
	pcm->right = glm::normalize(glm::cross(pcm->direction, pcm->worldUp));
	pcm->up = glm::normalize(glm::cross(pcm->right, pcm->direction));

	pcm->right = glm::vec3(0.0);
	pcm->lookAt = glm::identity <glm::mat4>();
	pcm->yaw = -90;
	pcm->firstClick = true;

	RecalcCmFrustrum(pcm);
	pcm->cpman.pvtcpman = &g_vtcpman;
	InitCplcy(&pcm->cpman, pcm);
	(pcm->cplook).pvtcplook = &g_vtcplook;
	InitCplook(&pcm->cplook, pcm);
	(pcm->cpalign).pvtcpalign = &g_vtcpalign;
	InitCpalign(&pcm->cpalign, pcm);
	(pcm->cpaseg).pvtcpaseg = &g_vtcpaseg;
	InitCplcy(&pcm->cpaseg, pcm);
	(pcm->cptn).pvtcptn = &g_vtcptn;
	InitCplcy(&pcm->cptn, pcm);
}

int GetCmSize()
{
	return sizeof(CM);
}

void CloneCm(CM* pcm, CM* pcmBase)
{
	LO lo = *pcm;
	*pcm = *pcmBase;
	memcpy(pcm, &lo, sizeof(LO));

	CloneLo(pcm, pcmBase);
}

void RecalcCmFrustrum(CM* pcm)
{
	pcm->yScreenRange = tanf(pcm->radFOV * 0.5);
	pcm->xScreenRange = pcm->yScreenRange * pcm->rAspect;
	pcm->rMRDAdjust   = pcm->rMRD * (1.0 / pcm->radFOV);
	pcm->sRadiusNearClip = pcm->yScreenRange * sqrt(pcm->rAspect * pcm->rAspect + 1.0) * pcm->sNearClip + 1.0;
	
	BuildProjectionMatrix(&pcm->radFOV, &g_gl.width, &g_gl.height, &pcm->sNearClip, &pcm->sFarClip, pcm->matProj);
	BuildCmFgfn(pcm, 1.0, &pcm->fgfn);
}

void BuildProjectionMatrix(float *fov, float *width, float *height, float *near, float *far, glm::mat4 &pmat)
{
	pmat = glm::identity <glm::mat4>();
	pmat = glm::perspective(*fov, *width / *height, *near , 1000000000.0f);
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

void SetCmPosMat(CM* pcm, glm::vec3* ppos, glm::mat3* pmat)
{
	if (ppos != nullptr)
		pcm->pos = *ppos;

	if (pmat != nullptr)
		pcm->lookAt = *pmat;

	UpdateCmMat4(pcm);

	pcm->sv = 0.0;
	pcm->swTiltMat = 0.0;
	pcm->swPanMat = 0.0;
	pcm->swTiltPos = 0.0;
	pcm->swPanPos = 0.0;

	if (ppos != nullptr)
		pcm->posClear = *ppos;

	if (pmat != nullptr)
		pcm->matClear = *pmat;
}

void* GetCmMat(CM* pcm)
{
	return &pcm->lookAt;
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
	pcm->radFOV = radFOV;
	RecalcCmFrustrum(pcm);
}

void* GetCmNearClip(CM* pcm)
{
	return &pcm->sNearClip;
}

void SetCmNearClip(CM* pcm, float sNearClip)
{
	pcm->sNearClip = sNearClip;
	RecalcCmFrustrum(pcm);
}

void* GetCmFarClip(CM* pcm)
{
	return &pcm->sFarClip;
}

void SetCmFarClip(CM* pcm, float sFarClip)
{
	pcm->sFarClip = sFarClip;
	RecalcCmFrustrum(pcm);
}

void* GetCmNearFog(CM* pcm)
{
	return &pcm->sNearFog;
}

void SetCmNearFog(CM* pcm, float sNearFog)
{
	pcm->sNearFog = sNearFog;
	RecalcCmFrustrum(pcm);
}

void* GetCmFarFog(CM* pcm)
{
	return &pcm->sFarFog;
}

void SetCmFarFog(CM* pcm, float sFarFog)
{
	pcm->sFarFog = sFarFog;
	RecalcCmFrustrum(pcm);
}

void* GetCmUFogMax(CM* pcm)
{
	return &pcm->uFogMax;
}

void SetCmUFogMax(CM* pcm, float uFogMax)
{
	pcm->uFogMax = uFogMax;
	RecalcCmFrustrum(pcm);
}

void* GetCmRgbaFog(CM* pcm)
{
	return &pcm->rgbaFog;
}

void SetCmRgbaFog(CM* pcm, RGBA prgbaFog)
{
	pcm->rgbaFog = prgbaFog;
	RecalcCmFrustrum(pcm);
}

void* GetCmMrdRatio(CM* pcm)
{
	return &pcm->rMRD;
}

void SetCmMrdRatio(CM* pcm, float rMRD)
{
	pcm->rMRD = rMRD;
	RecalcCmFrustrum(g_pcm);
}

void BuildLookAt(glm::vec3 &posEye, glm::vec3 &directionEye, glm::vec3 &upEye ,glm::mat4 &pmatLookAt)
{
	pmatLookAt = glm::identity <glm::mat4>();
	pmatLookAt = glm::lookAt(posEye, posEye + directionEye, upEye);
}

void SetupCm(CM *pcm)
{
	if (g_psw != nullptr)
	{
		GRFZON grfzonNew{};
		ClipVismapPointNoHop(g_psw->pvismap, &pcm->pos, &grfzonNew);

		if (grfzonNew != 0)
			pcm->grfzon = grfzonNew;
	}
}

void CombineEyeLookAtProj(glm::mat4 *pmatLookAt, glm::mat4 *pmatProj, glm::mat4 &pmat)
{
	pmat = *pmatProj * *pmatLookAt;
}

void UpdateCmMat4(CM* pcm)
{
	BuildProjectionMatrix(&pcm->radFOV, &g_gl.width, &g_gl.height, &pcm->sNearClip, &pcm->sFarClip, pcm->matProj);
	BuildLookAt(pcm->pos, pcm->direction, pcm->up ,pcm->lookAt);

	CombineEyeLookAtProj(&pcm->lookAt, &pcm->matProj, pcm->matWorldToClip);

	BuildFrustrum(pcm->matWorldToClip, pcm->frustum);
}

bool FInsideCmMrd(CM* pcm, float sRadius, float sMRD, float *puAlpha)
{

	return 0;
}

void DeleteCm(CM *pcm)
{
	delete pcm;
}