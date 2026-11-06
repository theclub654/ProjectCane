#pragma once
#include <glad/glad.h>

namespace CaneDeko
{
bool Initialize(unsigned width, unsigned height);
void ResizeOutput(unsigned width, unsigned height);
void Shutdown();
void InstallEntryPoints();
// Override the next presentation's telemetry with its scene render size.
// Frames without an override (installer, movies) use the output size.
void SetFrameResolution(unsigned width, unsigned height);
void Present();
void SetSwapInterval(int interval);
GLuint CreateProgram(const char* vertex, const char* geometry, const char* fragment);
}
