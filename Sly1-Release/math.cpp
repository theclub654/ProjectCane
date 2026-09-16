#include "math.h"

void LoadMatrixFromPosRot(glm::vec3* ppos, glm::mat3* pmat, glm::mat4* pmatDst)
{
	*pmatDst = glm::mat4(1.0f);

	(*pmatDst)[0] = glm::vec4((*pmat)[0], 0.0f);
	(*pmatDst)[1] = glm::vec4((*pmat)[1], 0.0f);
	(*pmatDst)[2] = glm::vec4((*pmat)[2], 0.0f);
	(*pmatDst)[3] = glm::vec4(*ppos, 1.0f);
}

void LoadMatrixFromPosRotScale(glm::vec3 *vecPos, glm::mat3 *matRot, glm::vec3 *vecScale, glm::mat4 *pmat)
{
	*pmat = glm::mat4(1.0f);

	(*pmat)[0][0] = (*matRot)[0][0] * vecScale->x;
	(*pmat)[0][1] = (*matRot)[0][1] * vecScale->x;
	(*pmat)[0][2] = (*matRot)[0][2] * vecScale->x;

	(*pmat)[1][0] = (*matRot)[1][0] * vecScale->y;
	(*pmat)[1][1] = (*matRot)[1][1] * vecScale->y;
	(*pmat)[1][2] = (*matRot)[1][2] * vecScale->y;

	(*pmat)[2][0] = (*matRot)[2][0] * vecScale->z;
	(*pmat)[2][1] = (*matRot)[2][1] * vecScale->z;
	(*pmat)[2][2] = (*matRot)[2][2] * vecScale->z;

	(*pmat)[3][0] = vecPos->x;
	(*pmat)[3][1] = vecPos->y;
	(*pmat)[3][2] = vecPos->z;
	(*pmat)[3][3] = 1.0f;
}

void LoadMatrixFromPosRotInverse(glm::vec3* pposSrc, glm::mat3* pmatSrc, glm::mat4* pmatDst)
{
	const glm::mat3 invRotation = glm::transpose(*pmatSrc);
	const glm::vec3 invPosition = -(invRotation * (*pposSrc));

	*pmatDst = glm::mat4(1.0f);

	(*pmatDst)[0] = glm::vec4(invRotation[0], 0.0f);
	(*pmatDst)[1] = glm::vec4(invRotation[1], 0.0f);
	(*pmatDst)[2] = glm::vec4(invRotation[2], 0.0f);
	(*pmatDst)[3] = glm::vec4(invPosition, 1.0f);
}

void LoadRotateMatrixEuler(glm::vec3* peul, glm::mat3* pmatRot)
{
	float sinX = sinf(peul->x), cosX = cosf(peul->x);
	float sinY = sinf(peul->y), cosY = cosf(peul->y);
	float sinZ = sinf(peul->z), cosZ = cosf(peul->z);

	(*pmatRot)[0][0] = cosY * cosZ;
	(*pmatRot)[0][1] = cosY * sinZ;
	(*pmatRot)[0][2] = -sinY;

	(*pmatRot)[1][0] = sinX * sinY * cosZ - cosX * sinZ;
	(*pmatRot)[1][1] = sinX * sinY * sinZ + cosX * cosZ;
	(*pmatRot)[1][2] = sinX * cosY;

	(*pmatRot)[2][0] = cosX * sinY * cosZ + sinX * sinZ;
	(*pmatRot)[2][1] = cosX * sinY * sinZ - sinX * cosZ;
	(*pmatRot)[2][2] = cosX * cosY;
}

void LoadRotateMatrix(glm::vec3* pvec, glm::mat3* pmat)
{
	float rad = glm::length(*pvec);

	if (rad > 0.0001f)
	{
		glm::vec3 normal = *pvec / rad;
		LoadRotateMatrixRad(rad, &normal, pmat);
	}
	else
		*pmat = g_matIdentity;
}

void LoadLookAtMatrix(glm::vec3* pdpos, glm::mat3* pmat)
{
	BuildOrthonormalMatrixZ(pdpos, &g_normalZ, pmat);
}

void DecomposeRotateMatrixRad(glm::mat3* pmat, float* prad, glm::vec3* pnormal)
{
	glm::mat3 m = glm::mat3(*pmat);

	float c = ((m[0][0] + m[1][1] + m[2][2]) - 1.0f) * 0.5f;

	if (c > 0.999999f)
	{
		*prad = 0.0f;
		*pnormal = glm::vec3(0.0f, 0.0f, 1.0f);
		return;
	}

	float x = std::sqrt(std::max(0.0f, std::abs((m[0][0] - c) / (1.0f - c))));
	float y = std::sqrt(std::max(0.0f, std::abs((m[1][1] - c) / (1.0f - c))));
	float z = std::sqrt(std::max(0.0f, 1.0f - x * x - y * y));

	*pnormal = glm::vec3(x, y, z);

	if (c >= -0.999999f)
	{
		*prad = std::acos(c);

		if (m[0][1] < m[1][0])
			pnormal->z = -pnormal->z;

		if (m[2][0] < m[0][2])
			pnormal->y = -pnormal->y;

		if (m[1][2] < m[2][1])
			pnormal->x = -pnormal->x;

		return;
	}

	*prad = glm::pi<float>();

	float nx = pnormal->x;
	float ny = pnormal->y;
	float nz = pnormal->z;

	if (ny < nx)
	{
		if (nz < nx)
		{
			if (m[0][1] < 0.0f)
				pnormal->y = -ny;

			if (m[0][2] < 0.0f)
				pnormal->z = -pnormal->z;

			return;
		}

		if (m[2][0] < 0.0f)
			pnormal->x = -nx;

		if (m[2][1] < 0.0f)
			pnormal->y = -pnormal->y;

		return;
	}

	if (nz < ny)
	{
		if (m[1][0] < 0.0f)
			pnormal->x = -nx;

		if (m[1][2] < 0.0f)
			pnormal->z = -pnormal->z;

		return;
	}

	if (m[2][0] < 0.0f)
		pnormal->x = -nx;

	if (m[2][1] < 0.0f)
		pnormal->y = -pnormal->y;
}

void TiltMatUpright(const glm::mat3* pmat, glm::mat3* pdmat, glm::mat3* pmatUpright)
{
	glm::mat3 dmatLocal;

	glm::mat3* pmatD = pdmat != nullptr ? pdmat : &dmatLocal;

	glm::vec3 localZ = (*pmat)[2];

	BuildRotateVectorsMatrix(&localZ, &g_normalZ, pmatD);

	if (pmatUpright != nullptr)
		*pmatUpright = (*pmatD) * (*pmat);
}

void BuildSimpleProjectionMatrix(float rx, float ry, float dxOffset, float dyOffset, float sNear, float sFar, glm::mat4& outMat)
{
	outMat = glm::mat4(0.0f);

	const float zScale = (sNear + sFar) / (sNear - sFar);

	outMat[0][0] = rx;
	outMat[1][1] = ry;

	outMat[2][0] = dxOffset;
	outMat[2][1] = dyOffset;
	outMat[2][2] = zScale;
	outMat[2][3] = 1.0f;

	outMat[3][2] = sNear * (1.0f - zScale);
}

void BuildOrthonormalMatrixZ(glm::vec3* pvecX, glm::vec3* pvecZ, glm::mat3* pmat)
{
	glm::vec3 vecX = *pvecX;

	if (glm::length(vecX) < 0.0001f)
		vecX = g_normalX;
	else
		vecX = glm::normalize(vecX);

	glm::vec3 vecY = glm::cross(*pvecZ, vecX);

	if (glm::length(vecY) < 0.0001f)
		GetNormalVector(vecX, vecY);
	else
		vecY = glm::normalize(vecY);

	glm::vec3 vecZ = glm::cross(vecX, vecY);

	(*pmat)[0] = vecX;
	(*pmat)[1] = vecY;
	(*pmat)[2] = vecZ;
}

void CalculateDmat4(const glm::mat4& pmat0, const glm::mat4& pmat1, glm::mat4& pdmat)
{
	glm::mat4 matInv;

	glm::vec3 pos = glm::vec3(pmat0[3]);
	glm::mat3 rot = glm::mat3(pmat0);

	LoadMatrixFromPosRotInverse(&pos, &rot, &matInv);

	pdmat = pmat1 * matInv;
}

void DecomposeRotateMatrixEuler(const glm::mat4& mat, glm::vec3& eul)
{
    eul.y = std::asin(-mat[0][2]);

    if (eul.y < 1.5706964f)
    {
        if (eul.y > -1.5706964f)
        {
            eul.x = std::atan2(mat[1][2], mat[2][2]);
            eul.z = std::atan2(mat[0][1], mat[0][0]);
            return;
        }

        eul.x = 0.0f;
        eul.z = -std::atan2(-mat[1][0], mat[2][0]);
        return;
    }

    eul.x = 0.0f;
    eul.z = std::atan2(-mat[1][0], mat[2][0]);
}
void LoadScaleMatrixScalar(glm::vec3* ppos, float rScale, glm::mat4* outMatScale)
{
	glm::vec3 scale(rScale, rScale, rScale);
	LoadScaleMatrixVector(ppos, nullptr, &scale, outMatScale);
}

void LoadScaleMatrixVector(glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pvecScale, glm::mat4* outMatScale)
{
	if (!outMatScale) return;

	// Defaults match original globals: g_vecZero, g_matIdentity
	const glm::vec3 pos = ppos ? *ppos : glm::vec3(0.0f);
	const glm::mat3 rot = pmat ? *pmat : glm::mat3(1.0f);
	const glm::vec3 scale = pvecScale ? *pvecScale : glm::vec3(1.0f);

	// Build M from pos + rot (inline)
	glm::mat4 M(1.0f);
	M[0] = glm::vec4(rot[0], 0.0f);
	M[1] = glm::vec4(rot[1], 0.0f);
	M[2] = glm::vec4(rot[2], 0.0f);
	M[3] = glm::vec4(pos, 1.0f);

	// Inverse(M) - same role as LoadMatrixFromPosRotInverse
	const glm::mat4 Minv = glm::inverse(M);

	// Scale matrix (diagonal)
	glm::mat4 S(1.0f);
	S[0][0] = scale.x;
	S[1][1] = scale.y;
	S[2][2] = scale.z;

	// Final conjugated scale
	*outMatScale = M * S * Minv;
}

void BuildRotateVectorsMatrix(const glm::vec3* pvec1, const glm::vec3* pvec2, glm::mat3* pmat)
{
	const float EPS = 1e-4f; // matches the original (~0.0001 thresholds)

	// 1) Normalize inputs with the same fallbacks the PS2 used
	glm::vec3 a = *pvec1;
	glm::vec3 b = *pvec2;

	float aLen = glm::length(a);
	if (aLen < EPS) a = g_normalZ; else a /= aLen;

	float bLen = glm::length(b);
	if (bLen < EPS) b = g_normalZ; else b /= bLen;

	// 2) Build candidate axis = a x b
	glm::vec3 axis = glm::cross(a, b);
	float axisLen = glm::length(axis);

	// If nearly parallel, replicate the original's fallback order:
	// try cross(a, g_normalX), else cross(a, g_normalY); last-resort guard.
	if (axisLen < EPS) {
		axis = glm::cross(a, g_normalX);
		axisLen = glm::length(axis);

		if (axisLen < EPS) {
			axis = glm::cross(a, g_normalY);
			axisLen = glm::length(axis);

			if (axisLen < EPS) {
				axis = g_normalX; // ultimate guard
				axisLen = 1.0f;
			}
		}
	}

	axis /= axisLen; // normalize axis

	// 3) Angle from dot (original does acosf of the clamped dot)
	float c = glm::clamp(glm::dot(a, b), -1.0f, 1.0f);
	float rad = std::acos(c);

	// 4) Fill matrix via the same function the original calls
	LoadRotateMatrixRad(rad, &axis, pmat);
}

void LoadRotateMatrixRad(float rad, const glm::vec3* pnormal, glm::mat3* pmat)
{
	float gSin = 0.0f;
	float gCos = 1.0f;
	CalculateSinCos(rad, &gSin, &gCos);

	const float x = pnormal->x;
	const float y = pnormal->y;
	const float z = pnormal->z;
	const float t = 1.0f - gCos;

	// Original MATRIX3-style layout, stored into GLM column-major mat3.
	(*pmat)[0][0] = t * x * x + gCos;
	(*pmat)[0][1] = t * x * y + gSin * z;
	(*pmat)[0][2] = t * x * z - gSin * y;

	(*pmat)[1][0] = t * y * x - gSin * z;
	(*pmat)[1][1] = t * y * y + gCos;
	(*pmat)[1][2] = t * y * z + gSin * x;

	(*pmat)[2][0] = t * z * x + gSin * y;
	(*pmat)[2][1] = t * z * y - gSin * x;
	(*pmat)[2][2] = t * z * z + gCos;
}

void LoadRotateMatrixPanTilt(float radPan, float radTilt, glm::mat3* pmat)
{
	glm::mat3 matPan;
	glm::mat3 matTilt;

	LoadRotateMatrixRad(radPan, &g_normalZ, &matPan);
	LoadRotateMatrixRad(-radTilt, &g_normalY, &matTilt);

	*pmat = matPan * matTilt;
}

void DecomposeSphere(const glm::vec3* pposEye, const glm::vec3* pposCenter, float* pradPan, float* pradTilt, float* ps)
{
	glm::vec3 dpos;

	if (pposCenter == nullptr) {
		dpos = *pposEye;
	}
	else {
		dpos = *pposEye - *pposCenter;
	}

	if (pradPan != nullptr) {
		*pradPan = RadNormalize(atan2f(dpos.y, dpos.x));
	}

	if (pradTilt != nullptr) {
		float sFlat = glm::length(glm::vec2(dpos.x, dpos.y));
		*pradTilt = RadNormalize(atan2f(dpos.z, sFlat));
	}

	if (ps != nullptr) {
		*ps = glm::length(dpos);
	}
}

void CalculateSinCos(float rad, float* pgSin, float* pgCos)
{
	*pgSin = std::sin(rad);
	*pgCos = std::cos(rad);
}

void CalculateDmat(const glm::mat3* pmat0, const glm::mat3* pmat1, glm::mat3* pdmat)
{
	*pdmat = (*pmat1) * glm::transpose(*pmat0);
}

void CalculateVectorPanTilt(const glm::vec3& vec, float* pradPan, float* pradTilt)
{
	if (pradPan)
		*pradPan = RadNormalize(std::atan2(vec.y, vec.x));

	if (pradTilt)
		*pradTilt = RadNormalize(std::atan2(vec.z, glm::length(glm::vec2(vec.x, vec.y))));
}

void GetNormalVectors(glm::vec3* pvec, glm::vec3* avec, glm::vec3* pnormalTry1, glm::vec3* pnormalTry2)
{
	glm::vec3 normal1 = glm::cross(*pnormalTry1, *pvec);

	if (glm::dot(normal1, normal1) < 0.0001f)
		normal1 = glm::cross(*pnormalTry2, *pvec);

	glm::vec3 normal2 = glm::cross(*pvec, normal1);

	avec[0] = glm::normalize(normal1);
	avec[1] = glm::normalize(normal2);
}

void GetNormalVector(const glm::vec3& vecSrc, glm::vec3& vecDst)
{
	glm::vec3 vec;

	const float ax = std::abs(vecSrc.x);
	const float ay = std::abs(vecSrc.y);
	const float az = std::abs(vecSrc.z);

	if (ax <= ay && ax <= az)
		vec = glm::vec3(0.0f, vecSrc.z, -vecSrc.y);
	else if (ay <= az)
		vec = glm::vec3(vecSrc.z, 0.0f, -vecSrc.x);
	else
		vec = glm::vec3(vecSrc.y, -vecSrc.x, 0.0f);

	vecDst = glm::normalize(glm::cross(vecSrc, vec));
}

void SetVectorCylind(glm::vec3* pvec, float rad, float sXY, float sZ)
{
	pvec->x = std::cos(rad) * sXY;
	pvec->y = std::sin(rad) * sXY;
	pvec->z = sZ;
}

void SetVectorSphere(glm::vec3* pvec, float radPan, float radTilt, float s)
{
	const float sinPan = std::sin(radPan);
	const float cosPan = std::cos(radPan);
	const float sinTilt = std::sin(radTilt);
	const float cosTilt = std::cos(radTilt);

	pvec->x = cosTilt * cosPan * s;
	pvec->y = cosTilt * sinPan * s;
	pvec->z = sinTilt * s;
}

float RadBetweenVectors(glm::vec3* pvecStart, glm::vec3* pvecFinish, glm::vec3* pnormal)
{
	const float startLength = glm::length(*pvecStart);
	const float finishLength = glm::length(*pvecFinish);

	if (startLength < 0.0001f || finishLength < 0.0001f)
	{
		if (pnormal)
			*pnormal = g_normalX;

		return 0.0f;
	}

	if (pnormal)
	{
		*pnormal = glm::cross(*pvecStart, *pvecFinish);

		const float normalLength = glm::length(*pnormal);

		if (normalLength > 1.0e-8f)
			*pnormal /= normalLength;
		else
			GetNormalVector(*pvecStart, *pnormal);
	}

	const float cosine = glm::clamp(
		glm::dot(*pvecStart, *pvecFinish) / (startLength * finishLength),
		-1.0f,
		1.0f);

	return std::acos(cosine);
}

void ConvertDeulToW(glm::vec3* peul, glm::vec3* pdeul, glm::vec3* pw)
{
	float sinY = glm::sin(peul->y);
	float cosY = glm::cos(peul->y);
	float sinZ = glm::sin(peul->z);
	float cosZ = glm::cos(peul->z);

	pw->x = cosY * cosZ * pdeul->x - sinZ * pdeul->y;
	pw->y = cosY * sinZ * pdeul->x + cosZ * pdeul->y;
	pw->z = -sinY * pdeul->x + pdeul->z;
}

void FitClq(float g0, float g1, float u, float gU, CLQ* pclq)
{
	pclq->g0 = g0;

	const float g2 = ((gU - g0) / u - (g1 - g0)) / (u - 1.0f);

	pclq->g2 = g2;
	pclq->g1 = (g1 - g0) - g2;
}

void NormalizeRotateMatrix3(glm::mat3* pmat)
{
	glm::vec3 x = (*pmat)[0];
	glm::vec3 y = (*pmat)[1];

	if (glm::length2(x) <= 0.000001f)
		return;

	x = glm::normalize(x);

	glm::vec3 z = glm::cross(x, y);

	if (glm::length2(z) <= 0.000001f)
		return;

	z = glm::normalize(z);

	y = glm::cross(z, x);

	if (glm::length2(y) <= 0.000001f)
		return;

	y = glm::normalize(y);

	(*pmat)[0] = x;
	(*pmat)[1] = y;
	(*pmat)[2] = z;
}

float SProjectVector(glm::vec3* pvec, glm::vec3* pnormal, glm::vec3* pvecNormal, glm::vec3* pvecTangent)
{
	float s = glm::dot(*pvec, *pnormal);

	glm::vec3 vecNormal = *pnormal * s;

	if (pvecNormal != nullptr)
		*pvecNormal = vecNormal;

	if (pvecTangent != nullptr)
		*pvecTangent = *pvec - vecNormal;

	return s;
}

void FindClosestPointBetweenLines(glm::vec3* pvecA, glm::vec3* pvecB, glm::vec3* pvecC, glm::vec3* pvecD, float* au, float* ps, glm::vec3* pnormal)
{
	float auLocal[2] = {};
	float sLocal = 0.0f;
	glm::vec3 normalLocal(0.0f);
	if (au == nullptr)
		au = auLocal;
	if (ps == nullptr)
		ps = &sLocal;
	if (pnormal == nullptr)
		pnormal = &normalLocal;

	glm::vec3 dpos = *pvecC - *pvecA;
	glm::vec3 dir0 = *pvecB - *pvecA;
	glm::vec3 dir1 = *pvecD - *pvecC;

	glm::vec3 normal = glm::cross(dir0, dir1);
	float normalLenSq = glm::dot(normal, normal);

	if (normalLenSq < 0.0001f)
	{
		normal = dpos;
		au[0] = 0.0f;
		au[1] = 0.0f;
	}
	else
	{
		glm::vec3 cross0 = glm::cross(dpos, dir1);
		glm::vec3 cross1 = glm::cross(dpos, dir0);

		au[0] = glm::dot(cross0, normal) / normalLenSq;
		au[1] = glm::dot(cross1, normal) / normalLenSq;

		if (glm::dot(dpos, normal) < 0.0f)
			normal = -normal;
	}

	float normalLen = glm::length(normal);

	if (normalLen < 0.0001f)
		normal = glm::vec3(0.0f, 0.0f, 1.0f);
	else
		normal /= normalLen;

	*ps = glm::dot(normal, dpos);
	*pnormal = normal;
}

void LimitVectorLength(glm::vec3* pvecSrc, float sMax, glm::vec3* pvecDst)
{
	float length = glm::length(*pvecSrc);

	if (sMax < length) {
		*pvecDst = *pvecSrc * (sMax / length);
	}
	else if (pvecSrc != pvecDst) {
		*pvecDst = *pvecSrc;
	}
}

void DecomposeRotateMatrixPanTilt(const glm::mat3* pmat, float* pradPan, float* pradTilt)
{
	// LoadRotateMatrixPanTilt builds the heading/forward axis in column 0.
	// Reading column 1 adds a quarter turn to the recovered pan, so repeated
	// compose/decompose cycles rotate the camera by roughly 90 degrees per frame.
	const glm::vec3& forward = (*pmat)[0];

	*pradPan = std::atan2(forward.y, forward.x);

	const float xyLength = std::sqrt(forward.x * forward.x + forward.y * forward.y);
	*pradTilt = std::atan2(forward.z, xyLength);
}

void CalcHeadingVector(float rad, glm::vec3* pvec)
{
	*pvec = glm::vec3(cosf(rad), sinf(rad), 0.0f);
}

void FindClosestPointBetweenLineSegments(glm::vec3* pvecA, glm::vec3* pvecB, glm::vec3* pvecC, glm::vec3* pvecD, float* au, float* ps, glm::vec3* pnormal)
{
	float auLocal[2] = {};
	float sLocal = 0.0f;
	glm::vec3 normalLocal(0.0f);
	float* pau = au != nullptr ? au : auLocal;
	float* psOut = ps != nullptr ? ps : &sLocal;
	glm::vec3* pnormalOut = pnormal != nullptr ? pnormal : &normalLocal;

	FindClosestPointBetweenLines(pvecA, pvecB, pvecC, pvecD, pau, psOut, pnormalOut);

	pau[0] = glm::clamp(pau[0], 0.0f, 1.0f);
	pau[1] = glm::clamp(pau[1], 0.0f, 1.0f);

	glm::vec3 posAB = glm::mix(*pvecA, *pvecB, pau[0]);
	glm::vec3 posCD = glm::mix(*pvecC, *pvecD, pau[1]);
	glm::vec3 dpos = posCD - posAB;

	*psOut = glm::length(dpos);

	if (*psOut < 0.0001f)
		*pnormalOut = g_normalX;
	else
		*pnormalOut = dpos / *psOut;
}

glm::vec3 PosSmooth(glm::vec3 posCur, glm::vec3 posTarget, float dt, SMP* psmp, glm::vec3* pv)
{
	glm::vec3 dpos = posTarget - posCur;
	float distance = glm::length(dpos);

	if (distance <= 0.0001f) {
		if (pv != nullptr)
			*pv = glm::vec3(0.0f);

		return posTarget;
	}

	float distanceVelocity = 0.0f;
	float distanceNext = GSmooth(0.0f, distance, dt, psmp, &distanceVelocity);

	if (pv != nullptr)
		*pv = dpos * (distanceVelocity / distance);

	return posCur + dpos * (distanceNext / distance);
}

void SmoothMatrix(const glm::mat3* pmatPrev, const glm::mat3* pmatNext, SMP* psmp, float dt, glm::mat3* pmatSmooth, glm::vec3* pwSmooth)
{
	glm::mat3 dmat;
	glm::vec3 axis;
	float rad;
	float sw;

	CalculateDmat(pmatPrev, pmatNext, &dmat);
	DecomposeRotateMatrixRad(&dmat, &rad, &axis);

	float radSmooth = GSmooth(0.0f, rad, dt, psmp, &sw);

	if (pmatSmooth != nullptr) {
		if (radSmooth == rad) {
			*pmatSmooth = *pmatNext;
		}
		else {
			glm::mat3 dmatSmooth;

			LoadRotateMatrixRad(radSmooth, &axis, &dmatSmooth);
			*pmatSmooth = dmatSmooth * *pmatPrev;
		}
	}

	if (pwSmooth != nullptr)
		*pwSmooth = axis * sw;
}

float CosRotateMatrixMagnitude(glm::mat3* pmat)
{
	float trace = (*pmat)[0][0] + (*pmat)[1][1] + (*pmat)[2][2];
	return GLimitAbs((trace - 1.0f) * 0.5f, 1.0f);
}

void DecomposeCylind(glm::vec3* pposEye, glm::vec3* pposCenter, float* prad, float* psXY, float* psZ)
{
	glm::vec3 dpos = *pposEye;

	if (pposCenter != nullptr)
		dpos -= *pposCenter;

	if (prad != nullptr)
		*prad = RadNormalize(std::atan2(dpos.y, dpos.x));

	if (psXY != nullptr)
		*psXY = glm::length(glm::vec2(dpos.x, dpos.y));

	if (psZ != nullptr)
		*psZ = dpos.z;
}

void LoadRotateVectorMatrix(const glm::vec3* pvecFrom, const glm::vec3* pvecTo, glm::mat3* pmat)
{
	const float sFrom = glm::length(*pvecFrom);
	const float sTo = glm::length(*pvecTo);

	if (sFrom < 0.0001f || sTo < 0.0001f)
	{
		*pmat = glm::mat3(1.0f);
		return;
	}

	const glm::vec3 cross = glm::cross(*pvecFrom, *pvecTo);
	const float sCross = glm::length(cross);

	glm::vec3 normal;

	if (sCross < 0.0001f)
		normal = g_normalX;
	else
		normal = cross / sCross;

	const float denominator = sFrom * sTo;
	const float sinRad = sCross / denominator;
	const float cosRad = glm::clamp(glm::dot(*pvecFrom, *pvecTo) / denominator, -1.0f, 1.0f);
	const float oneMinusCos = 1.0f - cosRad;

	const float x = normal.x;
	const float y = normal.y;
	const float z = normal.z;

	// GLM's scalar mat3 constructor is column-major.
	*pmat = glm::mat3(
		cosRad + oneMinusCos * x * x,
		oneMinusCos * x * y + sinRad * z,
		oneMinusCos * x * z - sinRad * y,

		oneMinusCos * x * y - sinRad * z,
		cosRad + oneMinusCos * y * y,
		oneMinusCos * y * z + sinRad * x,

		oneMinusCos * x * z + sinRad * y,
		oneMinusCos * y * z - sinRad * x,
		cosRad + oneMinusCos * z * z
	);
}

void BuildOrthonormalMatrixY(const glm::vec3* pvecX, const glm::vec3* pvecY, glm::mat3* pmat)
{
	glm::vec3 axisX = *pvecX;

	if (glm::dot(axisX, axisX) < 0.0001f)
		axisX = g_normalX;
	else
		axisX = glm::normalize(axisX);

	glm::vec3 axisZ = glm::cross(axisX, *pvecY);

	if (glm::dot(axisZ, axisZ) < 0.0001f)
		GetNormalVector(axisX, axisZ);
	else
		axisZ = glm::normalize(axisZ);

	(*pmat)[0] = axisX;
	(*pmat)[1] = glm::cross(axisZ, axisX);
	(*pmat)[2] = axisZ;
}

glm::vec3 g_vecOne{1.0};
glm::vec3 g_normalX = {1.0f, 0.0f, 0.0f};
glm::vec3 g_normalY = {0.0f, 1.0f, 0.0f};
glm::vec3 g_normalZ = {0.0f, 0.0f, 1.0f};
glm::vec3 g_vecZero = {0.0f, 0.0f, 0.0f};
glm::mat3 g_matIdentity{1.0};
