#pragma once
#include <iostream>
#include <glad/glad.h>
#include <string>
#include "shd.h"

class GLSHADER
{
    public:

    GLuint ID;

    void Init(const char* vertexFile, const char* geometryFile, const char* fragmentFile);

    void Use();
    void Delete();

    private:
    bool compileErrors(unsigned int shader, const char* type,
        const char* vertexFile = nullptr,
        const char* geometryFile = nullptr,
        const char* fragmentFile = nullptr);
};

void InvalidateShaderProgramCache();

extern GLSHADER glScreenShader;
extern GLSHADER glDyshadow;
extern GLSHADER glGlobShader;
extern GLSHADER glCelBorderShader;
extern GLSHADER glGeomShader;
extern GLSHADER glBlotShader;
extern GLSHADER glBlipShader;
