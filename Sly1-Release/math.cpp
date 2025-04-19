#include "math.h"

void LoadMatrixFromPosRot(glm::vec3 &ppos, glm::mat3 &pmat, glm::mat4 &pmatDst)
{
	pmatDst = pmat;

	pmatDst[3][0] = ppos[0];
	pmatDst[3][1] = ppos[1];
	pmatDst[3][2] = ppos[2];
	pmatDst[3][3] = 1.0;
}

void LoadMatrixFromPosRotScale(glm::vec3 &vecPos, glm::mat3 &matRot, glm::vec3 &vecScale, glm::mat4 &pmat)
{
	pmat[0][0] = matRot[0][0] * vecScale.x;
	pmat[0][1] = matRot[0][1] * vecScale.x;
	pmat[0][2] = matRot[0][2] * vecScale.x;
	pmat[0][3] = 0.0;

	pmat[1][0] = matRot[1][0] * vecScale.y;
	pmat[1][1] = matRot[1][1] * vecScale.y;
	pmat[1][2] = matRot[1][2] * vecScale.y;
	pmat[1][3] = 0.0;

	pmat[2][0] = matRot[2][0] * vecScale.z;
	pmat[2][1] = matRot[2][1] * vecScale.z;
	pmat[2][2] = matRot[2][2] * vecScale.z;
	pmat[2][3] = 0.0;

	pmat[3][0] = vecPos.x;
	pmat[3][1] = vecPos.y;
	pmat[3][2] = vecPos.z;
	pmat[3][3] = 1.0;
}

void LoadMatrixFromPosRotInverse(glm::vec3 &pposSrc, glm::mat3 &pmatSrc, glm::mat4 &pmatDst)
{
	pmatDst[0][0] = pmatSrc[0][0];
	pmatDst[0][1] = pmatSrc[0][1];
	pmatDst[0][2] = pmatSrc[0][2];

	pmatDst[1][0] = pmatSrc[1][0];
	pmatDst[1][1] = pmatSrc[1][1];
	pmatDst[1][2] = pmatSrc[1][2];

	pmatDst[2][0] = pmatSrc[2][0];
	pmatDst[2][1] = pmatSrc[2][1];
	pmatDst[2][2] = pmatSrc[2][2];
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