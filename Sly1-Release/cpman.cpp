#include "cpman.h"
#include "cm.h"

void SetCpmanCpmt(CPMAN* pcpman, CPMT cpmt)
{
	pcpman->cpmt = cpmt;
	/*if ((cpmt == CPMT_Orbit) && (pcpman->paloOrbit != (ALO*)0)) {
		SetCmLookAt(pcpman->pcm, &pcpman->pcm->pos, &(pcpman->paloOrbit->xf).posWorld);
	}*/
}

void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt)
{
    CM* pcm = pcpman->pcm;

    if (g_fDisableInput != true)
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
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
    }

    glm::vec3 dcam(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        dcam.z -= 1.0f;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        dcam.z += 1.0f;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        dcam.x += 1.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        dcam.x -= 1.0f;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        dcam.y += 1.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        dcam.y -= 1.0f;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        pcm->pos = glm::vec3(0.0f);

    if (glm::length2(dcam) > 0.0f)
    {
        const float speed = 5000.0f;
        dcam = glm::normalize(dcam) * (speed * dt);

        const glm::mat3 glBasis = ConvertGameCameraBasisToGl(pcm->mat);
        const glm::vec3 right = glBasis[0];
        const glm::vec3 up = glBasis[1];
        const glm::vec3 backward = glBasis[2];

        pcm->pos += right * dcam.x + up * dcam.y + backward * dcam.z;
    }

    UpdateCmMat4(pcm);
}