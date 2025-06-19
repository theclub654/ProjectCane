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
	pcm->direction = glm::vec3(0.0f, 0.0f, -1.0f);

	pcm->lookAt = glm::identity <glm::mat4>();
	pcm->yaw = -90;
	pcm->pitch = 0.0f;

	RecalcCm(pcm);
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
	CloneLo(pcm, pcmBase);

	pcm->pos = pcmBase->pos;
	pcm->direction = pcmBase->direction;
	pcm->up = pcmBase->up;
	pcm->right = pcmBase->right;
	pcm->worldUp = pcmBase->worldUp;
	pcm->yaw = pcmBase->yaw;
	pcm->pitch = pcmBase->pitch;
	pcm->firstClick = pcmBase->firstClick;

	for (int i = 0; i < 3; ++i)
		pcm->anormalFrustrumTranspose[i] = pcmBase->anormalFrustrumTranspose[i];

	pcm->frustum = pcmBase->frustum;
	pcm->lookAt = pcmBase->lookAt;
	pcm->rMRDAdjust = pcmBase->rMRDAdjust;
	pcm->matProj = pcmBase->matProj;
	pcm->matWorldToClip = pcmBase->matWorldToClip;
	pcm->matClipToWorld = pcmBase->matClipToWorld;

	for (int i = 0; i < 4; ++i)
		pcm->anormalFrustrum[i] = pcmBase->anormalFrustrum[i];

	pcm->rMRD = pcmBase->rMRD;
	pcm->radFOV = pcmBase->radFOV;
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

void RecalcCm(CM* pcm)
{
	pcm->rMRDAdjust = pcm->rMRD * (1.0 / g_renderDistance);

	float camAspect = (g_gl.aspectMode == FitToScreen) ? (g_gl.width / g_gl.height) : g_gl.aspectRatio;

	BuildProjectionMatrix(pcm->radFOV, camAspect, pcm->sNearClip, pcm->sFarClip, pcm->matProj);
	UpdateCmMat4(pcm);
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

void BuildProjectionMatrix(float fov, float aspectRatio, float near, float far, glm::mat4& pmat)
{
	far = 10000000000.0f;

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
	RecalcCm(pcm);
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
	float R = prgbaFog.bRed   / 255.0;
	float G = prgbaFog.bGreen / 255.0;
	float B = prgbaFog.bBlue  / 255.0;
	float A = prgbaFog.bAlpha / 255.0;

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

void BuildLookAt(glm::vec3 &posEye, glm::vec3 &directionEye, glm::vec3 &upEye ,glm::mat4 &pmatLookAt)
{
	pmatLookAt = glm::identity <glm::mat4>();
	pmatLookAt = glm::lookAt(posEye, posEye + directionEye, upEye);
}

void UnlockCm(int nParam)
{
	g_cmlk = CMLK_Nil;
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
	BuildLookAt(pcm->pos, pcm->direction, pcm->up ,pcm->lookAt);

	pcm->matWorldToClip = pcm->matProj * pcm->lookAt;

	ExtractFrustumPlanes(pcm->matWorldToClip, &pcm->frustum);
}

bool FInsideCmMrd(const CM* pcm, const glm::vec3& dpos, float sRadius, float sMRD, float& outAlpha)
{
	if (g_cmlk == CMLK_Grfzon) {
		sMRD = 1e10f; // basically disables render distance fade
	}

	// Calculate outer radius
	float outerRadius = sMRD * pcm->rMRDAdjust * g_renderDistance + sRadius;
	float outerRadiusSq = outerRadius * outerRadius;

	// Calculate position of point to test
	glm::vec3 targetPos;
	if (g_cmlk == CMLK_Mrd) {
		targetPos = pcm->pos + glm::vec3(dpos);
	}
	else {
		targetPos = glm::vec3(dpos); // dpos is already world space
	}

	// Squared distance to CM lock position
	glm::vec3 delta = targetPos - glm::vec3(0.0);
	float distanceSq = glm::dot(delta, delta);

	// Outside the outer boundary
	if (distanceSq > outerRadiusSq) {
		return false;
	}

	// Fade zone calculation
	float innerRadius = outerRadius - pcm->rMRDAdjust * 400.0f;
	float innerRadiusSq = innerRadius * innerRadius;

	// If in fade zone, calculate alpha
	if (distanceSq > innerRadiusSq) {
		float alpha = (outerRadiusSq - distanceSq) / (outerRadiusSq - innerRadiusSq);
		alpha = std::clamp(alpha, 0.0f, 1.0f);
		outAlpha = alpha * alpha * (3.0f - 2.0f * alpha);
	}
	else {
		outAlpha = 1.0f; // fully visible
	}

	return true;
}

void DeleteCm(CM *pcm)
{
	delete pcm;
}

CMLK g_cmlk;
float g_renderDistance = 1.0;