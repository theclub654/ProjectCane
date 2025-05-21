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

void LoadMatrixFromPosRotInverse(glm::vec3 &pposSrc, glm::mat3 &pmatSrc, glm::mat4 &pmatDst)
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

void BuildOrthonormalMatrixZ(glm::vec3 &pvecX, glm::vec3 &pvecZ, glm::mat4 &pmat)
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

void BuildRotateVectorsMatrix(const glm::vec3& vec1, const glm::vec3& vec2, glm::mat3& outMatrix)
{
	glm::vec3 fwd = glm::normalize(vec1); // Forward direction (toward the camera)
	glm::vec3 right = glm::cross(vec2, fwd); // Right vector

	if (glm::length2(right) < 0.0001f) {
		// If forward and up are nearly parallel, use a fallback right vector
		right = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), fwd);
		if (glm::length2(right) < 0.0001f) {
			right = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), fwd);
		}
	}

	right = glm::normalize(right); // Normalize the right vector
	glm::vec3 up = glm::normalize(glm::cross(fwd, right)); // Recalculate up vector to maintain orthogonality

	// Build the rotation matrix (right, up, forward)
	outMatrix[0] = right;
	outMatrix[1] = up;
	outMatrix[2] = -fwd; // Negative because the object should face the camera
}

void LoadRotateMatrixRad(float rad, glm::vec3& pnormal, glm::mat3& pmat)
{
	float s = std::sin(rad);
	float c = std::cos(rad);
	float oneMinusC = 1.0f - c;

	glm::vec3 n = glm::normalize(pnormal);

	// Outer product of n with itself
	glm::mat3 outer = glm::outerProduct(n, n);

	// Skew-symmetric cross-product matrix of n
	glm::mat3 crossMat = {
		0.0f,     -n.z,  n.y,
		n.z,   0.0f, -n.x,
		-n.y,   n.x, 0.0f
	};

	// Rodrigues' rotation formula
	pmat = oneMinusC * outer + c * glm::mat3(1.0f) + s * crossMat;
}

glm::vec3 g_normalX = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 g_normalY = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 g_normalZ = glm::vec3(0.0f, 0.0f, 1.0f);