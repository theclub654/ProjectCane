#pragma once

#include <glm/glm.hpp>

struct alignas(16) ROCEL
{
	glm::mat4 model;
	glm::vec4 celRgba;
	float     uAlphaCelBorder;
};