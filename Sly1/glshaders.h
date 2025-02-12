#pragma once
#include <iostream>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>
#include "shd.h"

class GLSHADER
{
    public:

    GLuint ID;

    void Init(const char* vertexFile, const char* geometryFile, const char* fragmentFile);

    void Use();
    void Delete();

    private:
    void compileErrors(unsigned int shader, const char* type);
};

std::string get_file_contents(const char* filename);

extern GLSHADER glScreenShader;
extern GLSHADER glGlobShader;
extern GLSHADER glCollisionShader;

extern GLSHADER glShadow;
extern GLSHADER glDiffuse;
extern GLSHADER glSaturate;

