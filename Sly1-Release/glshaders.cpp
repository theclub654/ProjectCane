#include "glshaders.h"
#include "resource1.h"

#include <Windows.h>
#include <cstring>
#include <stdexcept>

namespace
{
	int ShaderResourceId(const char* filename)
	{
		struct ShaderResource
		{
			const char* filename;
			int resourceId;
		};

		static constexpr ShaderResource resources[] =
		{
			{ "screen.vert", IDR_SHADER_SCREEN_VERT },
			{ "screen.frag", IDR_SHADER_SCREEN_FRAG },
			{ "dysh.vert", IDR_SHADER_DYSH_VERT },
			{ "dysh.frag", IDR_SHADER_DYSH_FRAG },
			{ "glob.vert", IDR_SHADER_GLOB_VERT },
			{ "glob.frag", IDR_SHADER_GLOB_FRAG },
			{ "celborder.vert", IDR_SHADER_CELBORDER_VERT },
			{ "celborder.frag", IDR_SHADER_CELBORDER_FRAG },
			{ "geom.vert", IDR_SHADER_GEOM_VERT },
			{ "geom.frag", IDR_SHADER_GEOM_FRAG },
			{ "blot.vert", IDR_SHADER_BLOT_VERT },
			{ "blot.frag", IDR_SHADER_BLOT_FRAG },
			{ "blip.vert", IDR_SHADER_BLIP_VERT },
			{ "blip.frag", IDR_SHADER_BLIP_FRAG },
		};

		for (const ShaderResource& resource : resources)
		{
			if (std::strcmp(filename, resource.filename) == 0)
				return resource.resourceId;
		}

		return 0;
	}

	std::string GetEmbeddedShaderSource(const char* filename)
	{
		const int resourceId = ShaderResourceId(filename);
		if (resourceId == 0)
			throw std::runtime_error(std::string("Unknown embedded shader: ") + filename);

		HRSRC resource = FindResourceW(nullptr, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
		if (resource == nullptr)
			throw std::runtime_error(std::string("Embedded shader resource was not found: ") + filename);

		HGLOBAL loadedResource = LoadResource(nullptr, resource);
		const DWORD resourceSize = SizeofResource(nullptr, resource);
		const void* resourceData = LockResource(loadedResource);
		if (loadedResource == nullptr || resourceData == nullptr || resourceSize == 0)
			throw std::runtime_error(std::string("Embedded shader resource could not be loaded: ") + filename);

		return std::string(static_cast<const char*>(resourceData), resourceSize);
	}
}

void GLSHADER::Init(const char* vertexFile, const char* geometryFile, const char* fragmentFile)
{
	std::string vertexCode{};
	std::string geometryCode{};
	std::string fragmentCode{};

	if (vertexFile != NULL)
		vertexCode = GetEmbeddedShaderSource(vertexFile);

	if (geometryFile != NULL)
		geometryCode = GetEmbeddedShaderSource(geometryFile);

	if (fragmentFile != NULL)
		fragmentCode = GetEmbeddedShaderSource(fragmentFile);

	const char* vertexSource   = NULL;
	const char* geometrySource = NULL;
	const char* fragmentSource = NULL;

	if (vertexFile != NULL)
		vertexSource = vertexCode.c_str();

	if (geometryFile != NULL)
		geometrySource = geometryCode.c_str();

	if (fragmentFile != NULL)
		fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = NULL;

	if (vertexFile != NULL)
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);
		compileErrors(vertexShader, "VERTEX");
	}

	GLuint geometryShader = NULL;

	if (geometryFile != NULL)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		glCompileShader(geometryShader);
		compileErrors(geometryShader, "GEOMETRY");
	}

	GLuint fragmentShader = NULL;

	if (fragmentFile != NULL)
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);
		compileErrors(fragmentShader, "FRAGMENT");
	}

	ID = glCreateProgram();

	if (vertexFile != NULL)
		glAttachShader(ID, vertexShader);

	if (geometryFile != NULL)
		glAttachShader(ID, geometryShader);

	if (fragmentFile != NULL)
		glAttachShader(ID, fragmentShader);

	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	if (vertexFile != NULL)
		glDeleteShader(vertexShader);

	if (geometryFile != NULL)
		glDeleteShader(geometryShader);

	if (fragmentFile != NULL)
		glDeleteShader(fragmentShader);

}

void GLSHADER::Use()
{
	glUseProgram(ID);
}

void GLSHADER::Delete()
{
	glDeleteProgram(ID);
}

void GLSHADER::compileErrors(unsigned int shader, const char* type)
{
	GLint hasCompiled;
	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}

	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}

GLSHADER glScreenShader;
GLSHADER glDyshadow;
GLSHADER glGlobShader;
GLSHADER glCelBorderShader;
GLSHADER glGeomShader;
GLSHADER glBlotShader;
GLSHADER glBlipShader;
