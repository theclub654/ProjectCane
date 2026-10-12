#pragma once
#include <glad/glad.h>

namespace CaneDeko
{
bool Initialize(unsigned width, unsigned height);
void ResizeOutput(unsigned width, unsigned height);
void Shutdown();
void InstallEntryPoints();
void Present();
void SetSwapInterval(int interval);
GLuint CreateProgram(const char* vertex, const char* geometry, const char* fragment);
}
