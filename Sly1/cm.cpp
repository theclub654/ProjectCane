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
	pcm->right = glm::vec3(0.0);
	pcm->lookAt = glm::identity <glm::mat4>();
	RecalcCmFrustrum(pcm);
	pcm->cpman.pvtcpman = &g_vtcpman;
	pcm->cpman.yaw = -90;
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
	pcm->yScreenRange = pcm->radFOV * 0.5;
	pcm->xScreenRange = pcm->yScreenRange * pcm->rAspect;
	pcm->rMRDAdjust   = pcm->rMRD * (1.0 / pcm->radFOV);
	pcm->sRadiusNearClip = pcm->yScreenRange * sqrt(pcm->rAspect * pcm->rAspect + 1.0) * pcm->sNearClip + 1.0;

	//BuildSimpleProjectionMatrix(640.0 / (pcm->yScreenRange * pcm->rAspect * 4096.0), 224.0 / (pcm->yScreenRange * 4096.0), 0.0, 0.0, pcm->sNearClip, pcm->sFarClip, pcm->matProj);
	
	BuildProjectionMatrix(pcm->radFOV, g_gl.width, g_gl.height, pcm->sNearClip, pcm->sFarClip, pcm->matProj);
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
	pmat[3][2] = sNear * (1.0 - (sNear + sFar) / (sNear - sFar));
}

void BuildProjectionMatrix(float fov, float width, float height, float near, float far, glm::mat4& pmat)
{
	pmat = glm::identity <glm::mat4>();
	pmat = glm::perspective(fov, width / height, near , 100000000.f);
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

void BuildLookAt(glm::vec3 posEye, glm::vec3 directionEye, glm::vec3 upEye ,glm::mat4 &pmatLookAt)
{
	pmatLookAt = glm::identity <glm::mat4>();
	pmatLookAt = glm::lookAt(posEye, posEye + directionEye, upEye);
}

void UpdateCmMat4(CM* pcm)
{
	BuildProjectionMatrix(pcm->radFOV, g_gl.width, g_gl.height, pcm->sNearClip, pcm->sFarClip, pcm->matProj);
	BuildLookAt(pcm->pos, pcm->direction, pcm->up ,pcm->lookAt);
}

int FInsideCmMrd(CM* pcm, float sRadius, float sMRD, float* puAlpha)
{

	return 0;
}

void DrawCm(CM* pcm, GLSHADER shader)
{
	shader.Use();

	// Sends proj matrix to GPU shader
	int projLocation = glGetUniformLocation(shader.ID, "proj");
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(pcm->matProj));

	// Sends view matrix to GPU shader
	int viewLocation = glGetUniformLocation(shader.ID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(pcm->lookAt));
}

void DeleteCm(LO* plo)
{
	delete(CM*)plo;
}