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
	FitRecipFunction(1.0, 4080.0, pcm->sFarFog / pcm->sNearFog, (1.0 - pcm->uFogMax * uFog) * 4080.0, &pfgfn->duFogBias, &pfgfn->ruFog);

	pfgfn->duFogBias = pfgfn->duFogBias + 8388608.0;
	pfgfn->duFogPlusClipBias = (pfgfn->duFogBias + 8388608.0) + 36864.0;
	pfgfn->sNearFog = pcm->sNearFog;
}

void BuildFrustrum(glm::mat4 &projViewMatrix, FRUSTUM &frustum)
{
	frustum.planes[0] = glm::vec4(projViewMatrix[0][3] + projViewMatrix[0][0],
	projViewMatrix[1][3] + projViewMatrix[1][0],
	projViewMatrix[2][3] + projViewMatrix[2][0],
	projViewMatrix[3][3] + projViewMatrix[3][0]);
	
	frustum.planes[1] = glm::vec4(projViewMatrix[0][3] - projViewMatrix[0][0],
	projViewMatrix[1][3] - projViewMatrix[1][0],
	projViewMatrix[2][3] - projViewMatrix[2][0],
	projViewMatrix[3][3] - projViewMatrix[3][0]);

	frustum.planes[2] = glm::vec4(projViewMatrix[0][3] + projViewMatrix[0][1],
	projViewMatrix[1][3] + projViewMatrix[1][1],
	projViewMatrix[2][3] + projViewMatrix[2][1],
	projViewMatrix[3][3] + projViewMatrix[3][1]);

	frustum.planes[3] = glm::vec4(projViewMatrix[0][3] - projViewMatrix[0][1],
	projViewMatrix[1][3] - projViewMatrix[1][1],
	projViewMatrix[2][3] - projViewMatrix[2][1],
	projViewMatrix[3][3] - projViewMatrix[3][1]);

	frustum.planes[4] = glm::vec4(projViewMatrix[0][3] + projViewMatrix[0][2],
	projViewMatrix[1][3] + projViewMatrix[1][2],
	projViewMatrix[2][3] + projViewMatrix[2][2],
	projViewMatrix[3][3] + projViewMatrix[3][2]);

	frustum.planes[5] = glm::vec4(projViewMatrix[0][3] - projViewMatrix[0][2],
	projViewMatrix[1][3] - projViewMatrix[1][2],
	projViewMatrix[2][3] - projViewMatrix[2][2],
	projViewMatrix[3][3] - projViewMatrix[3][2]);

	// Normalize the planes
	for (int i = 0; i < 6; ++i)
		frustum.planes[i] /= glm::length(glm::vec3(frustum.planes[i]));
}

void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt)
{
	float speed = 10000.0;
	float velocity = (float)dt * speed;
	
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
		pcpman->pcm->up    = glm::normalize(glm::cross(pcpman->pcm->right, pcpman->pcm->direction));
	}

	UpdateCmMat4(pcpman->pcm);
}