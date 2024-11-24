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
	pcm->mat = glm::identity <glm::mat4>();
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

void BuildCmFgfn(CM* pcm, float uFog, FGFN* pfgfn)
{
	FitRecipFunction(1.0, 4080.0, pcm->sFarFog / pcm->sNearFog, (1.0 - pcm->uFogMax * uFog) * 4080.0, &pfgfn->duFogBias, &pfgfn->ruFog);

	pfgfn->duFogBias = pfgfn->duFogBias + 8388608.0;
	pfgfn->duFogPlusClipBias = (pfgfn->duFogBias + 8388608.0) + 36864.0;
	pfgfn->sNearFog = pcm->sNearFog;
}

void RecalcCmFrustrum(CM* pcm)
{
	pcm->yScreenRange = tanf(pcm->radFOV * 0.5);
	pcm->xScreenRange = pcm->yScreenRange * pcm->rAspect;
	pcm->rMRDAdjust = pcm->rMRD * (1.0 / pcm->radFOV);
	pcm->sRadiusNearClip = pcm->yScreenRange * sqrt(pcm->rAspect * pcm->rAspect + 1.0) * pcm->sNearClip + 1.0;

	BuildSimpleProjectionMatrix(640.0 / (pcm->yScreenRange * pcm->rAspect * 4096.0), 224.0 / (pcm->yScreenRange * 4096.0), 0.0, 0.0, pcm->sNearClip, pcm->sFarClip, pcm->matProj);
	BuildCmFgfn(pcm, 1.0, &pcm->fgfn);
	UpdateCmMat4(pcm);
}

void BuildSimpleProjectionMatrix(float rx, float ry, float dxOffset, float dyOffset, float sNear, float sFar, glm::mat4 &pmat)
{
	pmat[0][0] = rx;
	pmat[1][1] = ry;
	pmat[2][0] = dxOffset;
	pmat[2][1] = dyOffset;
	pmat[2][2] = (sNear + sFar) / (sNear - sFar);
	pmat[2][3] = 1.0;
	pmat[3][2] = sNear * (1.0 - pmat[2][2]);
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
		pcm->mat = *pmat;

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

void CombineEyeLookAtProj(glm::vec3 pposEye, glm::mat3 pmatLookAt, glm::mat4 pmatProj, glm::mat4 &pmat)
{
	
}

void UpdateCmMat4(CM* pcm)
{
	CombineEyeLookAtProj(pcm->pos, pcm->mat, pcm->matProj, pcm->matWorldToClip);
}

int FInsideCmMrd(CM* pcm, float sRadius, float sMRD, float* puAlpha)
{

	return 0;
}

void DrawCm(CM* pcm, GLSHADER shader)
{
	shader.Use();

	// Sends view matrix to GPU shader
	int matWorldToClipLocation = glGetUniformLocation(shader.ID, "matWorldToClip");
	glUniformMatrix4fv(matWorldToClipLocation, 1, GL_FALSE, glm::value_ptr(pcm->matWorldToClip));

	//// Sends projection matrix to GPU shader
	int matClipToWorldLoation = glGetUniformLocation(shader.ID, "matClipToWorld");
	glUniformMatrix4fv(matClipToWorldLoation, 1, GL_FALSE, glm::value_ptr(pcm->matClipToWorld));
}

void DeleteCm(LO* plo)
{
	delete(CM*)plo;
}