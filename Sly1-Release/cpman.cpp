#include "cpman.h"
#include "cm.h"
#include "clock.h"

void SetCpmanCpmt(CPMAN* pcpman, CPMT cpmt)
{
	pcpman->cpmt = cpmt;
	/*if ((cpmt == CPMT_Orbit) && (pcpman->paloOrbit != (ALO*)0)) {
		SetCmLookAt(pcpman->pcm, &pcpman->pcm->pos, &(pcpman->paloOrbit->xf).posWorld);
	}*/
}

void UpdateCpman(CPMAN* pcpman, CPDEFI* pcpdefi, JOY* pjoy, float dt)
{
    (void)pcpdefi;

    CM* pcm = pcpman->pcm;
    GLFWwindow* window = g_gl.window;

    if (window == nullptr)
        return;

    // The game clock can be stopped while the debug UI or pause state is active.
    // A manual camera is a real-time debugging tool, so keep it responsive there.
    const float dtReal = (std::isfinite(g_clock.dtReal) && g_clock.dtReal > 0.0f)
        ? g_clock.dtReal
        : (1.0f / 60.0f);
    const float dtCamera = (std::isfinite(dt) && dt > 0.0f) ? dt : dtReal;

    if (!g_fDisableInput && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        const float dx = static_cast<float>(MOUSE::GetDX());
        const float dy = static_cast<float>(MOUSE::GetDY());
        const float sensitivity = 1.0f;

        pcm->yaw -= dx * sensitivity;
        pcm->pitch += dy * sensitivity;
        pcm->pitch = glm::clamp(pcm->pitch, -89.0f, 89.0f);

        const float yaw = glm::radians(pcm->yaw);
        const float pitch = glm::radians(pcm->pitch);

        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);
        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);

        const glm::vec3 right = glm::normalize(glm::vec3(cy, sy, 0.0f));
        const glm::vec3 forward = glm::normalize(glm::vec3(-sy * cp, cy * cp, sp));
        const glm::vec3 up = glm::normalize(glm::cross(right, forward));
        const glm::vec3 backward = -forward;

        glm::mat3 glBasis(1.0f);
        glBasis[0] = right;
        glBasis[1] = up;
        glBasis[2] = backward;

        pcm->mat = ConvertGlCameraBasisToGame(glBasis);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

	// Keyboard and controller input belong exclusively to the free camera while
	// this policy is active. Do not suppress them with the ImGui hover flag.
	if (pjoy != nullptr && pjoy->uDeflect2 > 0.0f)
	{
		constexpr float lookSpeed = 120.0f;
		pcm->yaw -= pjoy->x2 * lookSpeed * dtCamera;
		pcm->pitch += pjoy->y2 * lookSpeed * dtCamera;
		pcm->pitch = glm::clamp(pcm->pitch, -89.0f, 89.0f);

		const float yaw = glm::radians(pcm->yaw);
		const float pitch = glm::radians(pcm->pitch);
		const float cy = std::cos(yaw);
		const float sy = std::sin(yaw);
		const float cp = std::cos(pitch);
		const float sp = std::sin(pitch);
		const glm::vec3 right = glm::normalize(glm::vec3(cy, sy, 0.0f));
		const glm::vec3 forward = glm::normalize(glm::vec3(-sy * cp, cy * cp, sp));
		const glm::vec3 up = glm::normalize(glm::cross(right, forward));
		glm::mat3 glBasis(1.0f);
		glBasis[0] = right;
		glBasis[1] = up;
		glBasis[2] = -forward;
		pcm->mat = ConvertGlCameraBasisToGame(glBasis);
	}

	glm::vec3 dcam(0.0f);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dcam.z -= 1.0f;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dcam.z += 1.0f;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dcam.x += 1.0f;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dcam.x -= 1.0f;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) dcam.y += 1.0f;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) dcam.y -= 1.0f;

		if (pjoy != nullptr)
		{
			dcam.x += pjoy->x;
			dcam.z -= pjoy->y;
			if (pjoy->IsHeld(BTN_CROSS)) dcam.y += 1.0f;
			if (pjoy->IsHeld(BTN_CIRCLE)) dcam.y -= 1.0f;
		}

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			pcm->pos = glm::vec3(0.0f);

		if (glm::length2(dcam) > 0.0f)
		{
			const float speed = 5000.0f;
			dcam = glm::normalize(dcam) * (speed * dtCamera);
			const glm::mat3 glBasis = ConvertGameCameraBasisToGl(pcm->mat);
			pcm->pos += glBasis[0] * dcam.x + glBasis[1] * dcam.y + glBasis[2] * dcam.z;
	}

    UpdateCmMat4(pcm);
}
