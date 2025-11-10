#include "cplcy.h"

void InitCplcy(CPLCY* pcplcy, CM* pcm)
{
	pcplcy->pcm = pcm;
}

void InitCplook(CPLOOK* pcplook, CM* pcm)
{
	InitCplcy(pcplook, pcm);
	pcplook->clookk = 0;
	pcplook->rZoomMax = 10.0;
	//PushCplookLookk(pcplook, LOOKK_User);
	pcplook->rScreenSniper = 0.5;
	pcplook->sRadiusSniper = -1.0;
	pcplook->paloFocusSniper = nullptr;
}

void InitCpalign(CPALIGN* pcpalign, CM* pcm)
{
	InitCplcy(pcpalign, pcm);
	pcpalign->matLocal = glm::identity<glm::mat3>();
}

void BuildCmFgfn(CM* pcm, float uFog, FGFN* pfgfn)
{
	float recipNear = 1.0f / pcm->sNearFog;
	float recipFar  = 1.0f / pcm->sFarFog;

	// Base remapping range
	float ruFog = 1.0f / (recipNear - recipFar);

	// Apply fogMax * uFog to control the final intensity
	if (uFog != 0.0)
		ruFog *= pcm->uFogMax * uFog;
	else
		ruFog *= pcm->uFogMax;

	pfgfn->duFogBias = recipNear;
	pfgfn->ruFog = ruFog;
	pfgfn->sNearFog = pcm->sFarFog;
}

void BuildFrustrum(const glm::mat3& pmatLookAt, float rx, float ry, glm::vec3* anormalFrustrum)
{
	const glm::vec3& X = pmatLookAt[0];
	const glm::vec3& Y = pmatLookAt[1];
	const glm::vec3& Z = pmatLookAt[2];

	// +X (right side)
	{
		glm::vec3 axis = glm::normalize(X + Z * rx);
		anormalFrustrum[0] = glm::normalize(glm::cross(Z, axis));
	}

	// -X (left side)
	{
		glm::vec3 axis = glm::normalize(X - Z * rx);
		anormalFrustrum[1] = glm::normalize(glm::cross(axis, Z));
	}

	// +Y (top side)
	{
		glm::vec3 axis = glm::normalize(Y + Z * ry);
		anormalFrustrum[2] = glm::normalize(glm::cross(axis, X));
	}

	// -Y (bottom side)
	{
		glm::vec3 axis = glm::normalize(Y - Z * ry);
		anormalFrustrum[3] = glm::normalize(glm::cross(X, axis));
	}
}

void ExtractFrustumPlanes(const glm::mat4& viewProj, FRUSTUM* pfrustum)
{
	// Transpose the matrix once to access rows
	glm::mat4 m = glm::transpose(viewProj);

	// Left
	pfrustum->planes[0] = m[3] + m[0];
	// Right
	pfrustum->planes[1] = m[3] - m[0];
	// Bottom
	pfrustum->planes[2] = m[3] + m[1];
	// Top
	pfrustum->planes[3] = m[3] - m[1];
	// Near
	pfrustum->planes[4] = m[3] + m[2];
	// Far
	pfrustum->planes[5] = m[3] - m[2];

	// Normalize the planes
	for (int i = 0; i < 6; i++)
	{
		float length = glm::length(glm::vec3(pfrustum->planes[i]));

		if (length != 0.0f)
			pfrustum->planes[i] /= length;
	}
}

bool SphereInFrustum(const FRUSTUM& frustum, const glm::vec3& center, float radius)
{
	for (int i = 0; i < 6; i++)
	{
		float distance = glm::dot(glm::vec3(frustum.planes[i]), center) + frustum.planes[i].w;

		if (distance < -radius)
			return false;
	}

	return true;
}

void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt)
{
	CM* pcm = pcpman->pcm;

	// -------- movement (camera space) --------
	glm::vec3 dcam(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dcam.z -= 1.0f; // +F
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dcam.z += 1.0f; // -F
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dcam.x += 1.0f; // +R
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dcam.x -= 1.0f; // -R
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) dcam.y += 1.0f; // +U
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) dcam.y -= 1.0f; // -U
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) pcm->pos = glm::vec3(0.0);

	float speed = 5000;

	if (glm::length2(dcam) > 0.0f) {
		dcam = glm::normalize(dcam) * (speed * dt);

		// columns: [0]=R, [1]=U, [2]=F
		const glm::vec3 R = pcm->mat[0];
		const glm::vec3 U = pcm->mat[1];
		const glm::vec3 F = pcm->mat[2];

		pcm->pos += R * dcam.x + U * dcam.y + F * dcam.z;
	}

	if (g_fDisableInput != true)
	{
		// -------- mouse -> yaw/pitch (Z-up: yaw about +Z, pitch about +X) --------
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			// --- mouse deltas (pixels) ---
			float dx = (float)MOUSE::GetDX();
			float dy = (float)MOUSE::GetDY();

			// sensitivity in *degrees* per pixel (since yaw/pitch are degrees)
			const float sensDeg = 0.15f;

			// intuitive controls (Z-up, RH):
			//  - move right -> yaw right
			//  - move up    -> look up
			pcpman->pcm->yaw   -= dx;
			pcpman->pcm->pitch += dy;

			// clamp pitch (avoid flip)
			pcpman->pcm->pitch = glm::clamp(pcpman->pcm->pitch, -89.0f, 89.0f);

			// radians once
			float yaw   = glm::radians(pcpman->pcm->yaw);
			float pitch = glm::radians(pcpman->pcm->pitch);

			// precompute trig
			float cy = cosf(yaw);
			float sy = sinf(yaw);

			float cp = cosf(pitch);
			float sp = sinf(pitch);

			// --- build orthonormal basis (Z-up, zero roll) ---
			// Right in XY plane from yaw
			glm::vec3 R = glm::vec3(cy, sy, 0.0f);

			// Forward (direction)
			glm::vec3 F = glm::vec3(-sy * cp, cy * cp, sp);

			// Up from cross (keeps it orthonormal and RH)
			glm::vec3 U = glm::normalize(glm::cross(R, F));

			// normalize R/F just in case
			R = glm::normalize(R);
			F = -glm::normalize(F);

			// --- store into pcm->mat (columns = [R, U, F]) ---
			pcpman->pcm->mat[0] = glm::vec4(R,  0.0f);
			pcpman->pcm->mat[1] = glm::vec4(U,  0.0f);
			pcpman->pcm->mat[2] = glm::vec4(F,  0.0f);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	UpdateCmMat4(pcm);
}