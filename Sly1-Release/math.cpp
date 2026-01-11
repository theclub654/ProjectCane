#include "math.h"

void LoadMatrixFromPosRot(glm::vec3& ppos, glm::mat3& pmat, glm::mat4& pmatDst)
{
	pmatDst[0] = glm::vec4(pmat[0][0], pmat[0][1], pmat[0][2], 0.0f);
	pmatDst[1] = glm::vec4(pmat[1][0], pmat[1][1], pmat[1][2], 0.0f);
	pmatDst[2] = glm::vec4(pmat[2][0], pmat[2][1], pmat[2][2], 0.0f);

	pmatDst[3] = glm::vec4(ppos.x, ppos.y, ppos.z, 1.0f);
}

void LoadMatrixFromPosRotScale(glm::vec3& vecPos, glm::mat3& matRot, glm::vec3& vecScale, glm::mat4& pmat)
{
	pmat = glm::mat4(1.0f); // Initialize to identity

	// Apply scaled rotation
	pmat[0][0] = matRot[0][0] * vecScale.x;
	pmat[0][1] = matRot[0][1] * vecScale.x;
	pmat[0][2] = matRot[0][2] * vecScale.x;
	pmat[1][0] = matRot[1][0] * vecScale.y;
	pmat[1][1] = matRot[1][1] * vecScale.y;
	pmat[1][2] = matRot[1][2] * vecScale.y;
	pmat[2][0] = matRot[2][0] * vecScale.z;
	pmat[2][1] = matRot[2][1] * vecScale.z;
	pmat[2][2] = matRot[2][2] * vecScale.z;

	// Set translation (position)
	pmat[3][0] = vecPos.x;
	pmat[3][1] = vecPos.y;
	pmat[3][2] = vecPos.z;
}

void LoadMatrixFromPosRotInverse(glm::vec3& pposSrc, glm::mat3& pmatSrc, glm::mat4& pmatDst)
{
	glm::mat3 invRotation = glm::transpose(pmatSrc);

	// Invert translation: rotate and negate the original position
	glm::vec3 invPosition = -(invRotation * (pposSrc));

	// Build the final 4x4 matrix
	pmatDst = glm::mat4(1.0f); // Identity first

	(pmatDst)[0][0] = invRotation[0][0];
	(pmatDst)[1][0] = invRotation[0][1];
	(pmatDst)[2][0] = invRotation[0][2];

	(pmatDst)[0][1] = invRotation[1][0];
	(pmatDst)[1][1] = invRotation[1][1];
	(pmatDst)[2][1] = invRotation[1][2];

	(pmatDst)[0][2] = invRotation[2][0];
	(pmatDst)[1][2] = invRotation[2][1];
	(pmatDst)[2][2] = invRotation[2][2];

	(pmatDst)[3][0] = invPosition.x;
	(pmatDst)[3][1] = invPosition.y;
	(pmatDst)[3][2] = invPosition.z;
	(pmatDst)[3][3] = 1.0f;

	// Set remaining elements explicitly to zero
	(pmatDst)[0][3] = 0.0f;
	(pmatDst)[1][3] = 0.0f;
	(pmatDst)[2][3] = 0.0f;
}

void BuildOrthonormalMatrixZ(glm::vec3& pvecX, glm::vec3& pvecZ, glm::mat4& pmat)
{
	const glm::vec3 g_normalX = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 x = pvecX;
	float xLen2 = glm::length2(x);

	if (xLen2 < 0.0001f) {
		x = g_normalX;
	}
	else {
		x = glm::normalize(x);
	}

	// Compute Y = Z × X
	glm::vec3 y = glm::cross(pvecZ, x);
	float yLen2 = glm::length2(y);

	if (yLen2 < 0.0001f) {
		// Fallback if inputZ and x are nearly parallel
		if (std::abs(x.x) < 0.9f)
			y = glm::cross(x, glm::vec3(1.0f, 0.0f, 0.0f));
		else
			y = glm::cross(x, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else {
		y = glm::normalize(y);
	}

	// Recompute Z = X × Y to ensure orthogonality
	glm::vec3 z = glm::cross(x, y);

	// Store axes in columns of a 4x4 matrix (GLM is column-major)
	pmat = glm::mat4(1.0f); // Identity matrix
	pmat[0] = glm::vec4(x, 0.0f);  // X axis
	pmat[1] = glm::vec4(y, 0.0f);  // Y axis
	pmat[2] = glm::vec4(z, 0.0f);  // Z axis
}

glm::vec3 DecomposeRotateMatrixEuler(const glm::mat3& R)
{
	const float y = asinf(-(R[2][0]));
	glm::vec3 eul;
	eul.y = y;

	constexpr float k = 1.570696f;

	if (y < k)
	{
		if (y > -k)
		{
			eul.x = atan2f(R[2][1], R[2][2]);
			eul.z = atan2f(R[1][0], R[0][0]);
			return eul;
		}
		// South pole gimbal lock (y <= -k)
		eul.x = 0.0f;
		eul.z = -atan2f(-(R[0][1]), R[0][2]);
		return eul;
	}

	// North pole gimbal lock (y >= k)
	eul.x = 0.0f;
	// z = atan2( -m[1,0], m[2,0] )
	eul.z = atan2f(-(R[0][1]), R[0][2]);
	return eul;
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

	// Inverse(M) – same role as LoadMatrixFromPosRotInverse
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

	// If nearly parallel, replicate the original’s fallback order:
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
	// 1) sin/cos via your original routine
	float gSin = 0.0f, gCos = 1.0f;
	CalculateSinCos(rad, &gSin, &gCos);

	// 2) normalize axis (defensive, mirror PS2 behavior of using +Z if degenerate)
	glm::vec3 u = *pnormal;
	float len2 = glm::dot(u, u);
	if (len2 > 0.0f) {
		u *= 1.0f / std::sqrt(len2);
	}
	else {
		u = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	const float t = 1.0f - gCos;
	const float x = u.x, y = u.y, z = u.z;

	// row-major components of R
	const float r00 = t * x * x + gCos;
	const float r01 = t * x * y - gSin * z;
	const float r02 = t * x * z + gSin * y;

	const float r10 = t * y * x + gSin * z;
	const float r11 = t * y * y + gCos;
	const float r12 = t * y * z - gSin * x;

	const float r20 = t * z * x - gSin * y;
	const float r21 = t * z * y + gSin * x;
	const float r22 = t * z * z + gCos;

	// 4) store into glm::mat3 (COLUMN-MAJOR):
	// column 0 = (r00, r10, r20), column 1 = (r01, r11, r21), column 2 = (r02, r12, r22)
	(*pmat)[0][0] = r00;  (*pmat)[0][1] = r10;  (*pmat)[0][2] = r20;
	(*pmat)[1][0] = r01;  (*pmat)[1][1] = r11;  (*pmat)[1][2] = r21;
	(*pmat)[2][0] = r02;  (*pmat)[2][1] = r12;  (*pmat)[2][2] = r22;
}

void CalculateSinCos(float rad, float* pgSin, float* pgCos)
{
	*pgSin = std::sin(rad);
	*pgCos = std::cos(rad);
}

glm::vec3 g_normalX = {1.0f, 0.0f, 0.0f};
glm::vec3 g_normalY = {0.0f, 1.0f, 0.0f};
glm::vec3 g_normalZ = {0.0f, 0.0f, 1.0f};
glm::vec3 g_vecZero = {0.0f, 0.0f, 0.0f};