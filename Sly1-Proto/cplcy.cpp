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
	// Calculate reciprocal distances
	float recipNear = 1.0f / pcm->sNearFog;
	float recipFar = 1.0f / pcm->sFarFog;

	// Compute target output range (similar to what PS2 did with 4080.0)
	float fogStart = 0.0f;
	float fogEnd = (1.0f - pcm->uFogMax * uFog);

	// Solve linear fit: fog = recipZ * ruFog + duFogBias
	pfgfn->ruFog = (fogEnd - fogStart) / (recipFar - recipNear);
	pfgfn->duFogBias = fogStart - pfgfn->ruFog * recipNear;

	pfgfn->sNearFog = pcm->sNearFog;
}

void BuildFrustrum(const glm::mat3& pmatLookAt, float rx, float ry, glm::vec3 *anormalFrustrum)
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

void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt)
{
	if (g_fDisableInput != true)
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

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			pcpman->pcm->yaw += MOUSE::GetDX();
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