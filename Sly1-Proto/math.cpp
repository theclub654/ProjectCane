#include "math.h"

void LoadMatrixFromPosRot(glm::vec3 &ppos, glm::mat3 &pmat, glm::mat4 &pmatDst)
{
	pmatDst = glm::mat4(1.0f);

	pmatDst[0] = glm::vec4(pmat[0], 0.0f); // Right (X axis)
	pmatDst[1] = glm::vec4(pmat[1], 0.0f); // Up (Y axis)
	pmatDst[2] = glm::vec4(pmat[2], 0.0f); // Forward (Z axis)
	pmatDst[3] = glm::vec4(ppos, 1.0f);    // Position (translation)
}

void LoadMatrixFromPosRotScale(glm::vec3 &vecPos, glm::mat3 &matRot, glm::vec3 &vecScale, glm::mat4 &pmat)
{
	// Start with identity
	pmat = glm::mat4(1.0f);

	// Apply rotation and scaling to each axis
	pmat[0] = glm::vec4(matRot[0] * vecScale.x, 0.0f); // Right (X)
	pmat[1] = glm::vec4(matRot[1] * vecScale.y, 0.0f); // Up (Y)
	pmat[2] = glm::vec4(matRot[2] * vecScale.z, 0.0f); // Forward (Z)

	// Apply translation
	pmat[3] = glm::vec4(vecPos, 1.0f);
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
	glm::vec3 forward = glm::normalize(vec1);
	glm::vec3 upRef = glm::normalize(vec2);

	// If forward and upRef are nearly aligned, fix upRef to avoid swing
	if (std::abs(glm::dot(forward, upRef)) > 0.999f)
	{
		// Choose a stable fallback up vector
		upRef = glm::abs(forward.z) < 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
	}

	// Compute a stable right vector
	glm::vec3 right = glm::normalize(glm::cross(upRef, forward));

	// Ensure right is not flipped due to cross direction instability
	if (right.x < 0.0f) right = -right;

	// Recompute up to ensure orthogonality
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	// Z-up basis: right (X), forward (Y), up (Z)
	outMatrix[0] = right;
	outMatrix[1] = forward;
	outMatrix[2] = up;
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

void DecomposeRotateMatrixEuler(const glm::mat3& mat, glm::vec3& eulOut)
{
	float pitch = asinf(-mat[0][2]);
	eulOut.y = pitch;

	if (pitch < glm::radians(89.96f)) {
		if (pitch > glm::radians(-89.96f)) {
			// Normal case
			eulOut.x = atan2f(mat[2][0], mat[2][2]);  // roll (x)
			eulOut.z = atan2f(mat[0][1], mat[0][0]);  // yaw (z)
			return;
		}

		// Gimbal lock (pitch ? -90°)
		eulOut.x = 0.0f;
		eulOut.z = -atan2f(-mat[1][0], mat[1][1]);
	}
	else {
		// Gimbal lock (pitch ? +90°)
		eulOut.x = 0.0f;
		eulOut.z = atan2f(-mat[1][0], mat[1][1]);
	}
}

void LoadRotateMatrixEuler(const glm::vec3& eul, glm::mat3* outMat)
{
	if (!outMat) return;

	glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), eul.y, glm::vec3(0, 1, 0));
	glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), eul.x, glm::vec3(1, 0, 0));
	glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), eul.z, glm::vec3(0, 0, 1));

	glm::mat4 result = rotY * rotX * rotZ;  // Y * X * Z (YXZ order)
	*outMat = glm::mat3(result);
}

glm::vec3 g_normalX = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 g_normalY = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 g_normalZ = glm::vec3(0.0f, 0.0f, 1.0f);