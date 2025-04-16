#include "cm.h"

CM* NewCm()
{
	return new CM{};
}

void InitCm(CM* pcm)
{
	InitLo(pcm);

	pcm->rgbaFog = glm::vec4(0.0);
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
}

void BuildProjectionMatrix(float *fov, float *width, float *height, float *near, float *far, glm::mat4 &pmat)
{
	pmat = glm::identity <glm::mat4>();
	pmat = glm::perspective(*fov, *width / *height, *near , 10000000000.0f);
}

void BuildSimpleProjectionMatrix(float rx, float ry, float dxOffset, float dyOffset, float sNear, float sFar, glm::mat4& pmat)
{
	float fVar1 = (sNear + sFar) / (sNear - sFar);

	pmat[0][0] = rx;
	pmat[1][1] = ry;
	pmat[2][0] = dxOffset;
	pmat[2][1] = dyOffset;
	pmat[2][2] = fVar1;
	pmat[2][3] = 1.0f;
	pmat[3][2] = sNear * (1.0f - fVar1);
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
	float R = prgbaFog.bRed / 255.0;
	float G = prgbaFog.bGreen / 255.0;
	float B = prgbaFog.bBlue / 255.0;
	float A = prgbaFog.bAlpha / 255.0;

	pcm->rgbaFog = glm::vec4(R, G, B, A);
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

void CombineEyeLookAtProj(const glm::vec3& eyePos, const glm::mat3& lookAt, const glm::mat4& proj, glm::mat4& pmat)
{
	glm::mat4 view = glm::mat4(1.0f); // identity matrix

	// Fill in rotation (lookAt matrix)
	view[0] = glm::vec4(-lookAt[1], 0.0f); // right (-Y)
	view[1] = glm::vec4(-lookAt[2], 0.0f); // up    (-Z)
	view[2] = glm::vec4(lookAt[0], 0.0f); // forward (X)

	// Fill in translation (eye position)
	glm::vec3 eye = eyePos;
	view[3] = glm::vec4(eye, 1.0f);

	// Invert view matrix (convert from object to camera space)
	glm::mat4 viewInv = glm::inverse(view);

	// Combine projection and view
	pmat = proj * viewInv;
}

// Transpose frustum normals
void TransposeFrustrumNormals(const glm::vec3* anormalFrustrum, glm::vec4* outTransposed)
{
	for (int i = 0; i < 3; ++i) 
	{
		outTransposed[i] = glm::vec4(
			anormalFrustrum[0][i], // X/Y/Z of normal 0
			anormalFrustrum[1][i], // X/Y/Z of normal 1
			anormalFrustrum[2][i], // X/Y/Z of normal 2
			anormalFrustrum[3][i]  // X/Y/Z of normal 3
		);
	}
}

void UpdateCmMat4(CM* pcm)
{
	BuildProjectionMatrix(&pcm->radFOV, &g_gl.width, &g_gl.height, &pcm->sNearClip, &pcm->sFarClip, pcm->matProj);
	BuildLookAt(pcm->pos, pcm->direction, pcm->up ,pcm->lookAt);

	pcm->frustum = ExtractFrustumPlanes(pcm->matProj * pcm->lookAt);
}

int FInsideCmMrd(const CM* pcm, const glm::vec4& dpos, float sRadius, float sMRD, float* puAlpha)
{
	float fVar1;
	glm::vec3 alphaVec;
	float fVar2, fVar3;

	fVar3 = sMRD * pcm->rMRDAdjust + sRadius;

	glm::vec3 adjustedAlphaVec = alphaVec * alphaVec;
	fVar1 = glm::length(adjustedAlphaVec); // Length squared

	fVar2 = fVar3 * fVar3;
	if (fVar2 < fVar1) {
		return 0; // Not inside the region
	}

	fVar3 = fVar3 - pcm->rMRDAdjust * 400.0f;
	fVar3 = fVar3 * fVar3;
	if (fVar3 < fVar1) {
		fVar3 = (fVar2 - fVar1) / (fVar2 - fVar3);
	}
	else {
		fVar3 = 1.0f;
	}

	*puAlpha = fVar3; // Set the alpha value based on distance

	return 1; // Inside the region
}

void DeleteCm(CM *pcm)
{
	delete pcm;
}