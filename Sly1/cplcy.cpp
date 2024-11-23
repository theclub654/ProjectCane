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

void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt)
{
	float speed = 10000.0;
	float velocity = dt * speed;
	
	glm::vec3 *cameraRight = nullptr;
	glm::vec3 *cameraUp = nullptr;
	glm::vec3 *cameraDirection = nullptr;
	glm::vec3 *cameraPos = nullptr;
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cameraDirection = &pcpman->pcm->mat[2];
		cameraPos = &pcpman->pcm->pos;
		*cameraPos += *cameraDirection * velocity;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraDirection = &pcpman->pcm->mat[2];
		cameraPos = &pcpman->pcm->pos;
		*cameraPos -= *cameraDirection * velocity;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cameraRight = &pcpman->pcm->mat[0];
		cameraPos = &pcpman->pcm->pos;
		*cameraPos += *cameraRight * velocity;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraRight = &pcpman->pcm->mat[0];
		cameraPos = &pcpman->pcm->pos;
		*cameraPos -= *cameraRight * velocity;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		cameraUp = &pcpman->pcm->mat[1];
		cameraPos = &pcpman->pcm->pos;
		*cameraPos += *cameraUp * velocity;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		cameraUp = &pcpman->pcm->mat[1];
		cameraPos = &pcpman->pcm->pos;
		*cameraPos -= *cameraUp * velocity;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		*cameraPos = glm::vec3{ 0.0 };

	SetCmPosMat(pcpman->pcm, cameraPos, nullptr);
}