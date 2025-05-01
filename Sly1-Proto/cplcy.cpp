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
	float recipFar = 1.0f / pcm->sFarFog;

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

void ExtractFrustumPlanes(const glm::mat4 &viewProj, FRUSTUM *pfrustum) 
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

bool SphereInFrustum(const FRUSTUM &frustum, const glm::vec3 &position, float radius)
{
	for (int i = 0; i < 6; i++)
	{
		float distance = glm::dot(glm::vec3(frustum.planes[i]), position) + frustum.planes[i].w;

		if (distance < -radius )
			return false;
	}
	return true;
}

void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt)
{
	float speed = 8000.0;
	float velocity = dt * speed;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		pcpman->pcm->pos += pcpman->pcm->direction * velocity;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		pcpman->pcm->pos -= pcpman->pcm->direction * velocity;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		pcpman->pcm->pos += pcpman->pcm->right * velocity;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		pcpman->pcm->pos -= pcpman->pcm->right * velocity;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		pcpman->pcm->pos += pcpman->pcm->up * velocity;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		pcpman->pcm->pos -= pcpman->pcm->up * velocity;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		pcpman->pcm->pos = glm::vec3{ 0.0 };

	if (g_fDisableInput != true)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			pcpman->pcm->yaw   += MOUSE::GetDX();
			pcpman->pcm->pitch += MOUSE::GetDY();

			if (pcpman->pcm->pitch > 89.0f)
				pcpman->pcm->pitch = 89.0f;

			else if (pcpman->pcm->pitch < -89.0f)
				pcpman->pcm->pitch = -89.0f;

			pcpman->pcm->direction.x = cos(glm::radians(-pcpman->pcm->yaw)) * cos(glm::radians(pcpman->pcm->pitch));
			pcpman->pcm->direction.y = sin(glm::radians(-pcpman->pcm->yaw)) * cos(glm::radians(pcpman->pcm->pitch));
			pcpman->pcm->direction.z = sin(glm::radians(pcpman->pcm->pitch));

			pcpman->pcm->direction = glm::normalize(pcpman->pcm->direction);

			pcpman->pcm->right = glm::normalize(glm::cross(pcpman->pcm->direction, pcpman->pcm->worldUp));
			pcpman->pcm->up = glm::normalize(glm::cross(pcpman->pcm->right, pcpman->pcm->direction));
		}
	}
	
	UpdateCmMat4(pcpman->pcm);
}