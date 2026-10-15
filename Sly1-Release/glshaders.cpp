#include "glshaders.h"
#include <limits>

namespace
{
GLuint s_activeShaderProgram = (std::numeric_limits<GLuint>::max)();
}
#include "resource1.h"

#ifdef CANE_PORTABLE_SHADERS
#include "embedded_shaders.h"
#endif
#ifdef _WIN32
#include <Windows.h>
#endif
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace
{
	void ShowShaderErrorConsole()
	{
		#ifdef _WIN32
		static bool initialized = false;
		if (initialized)
			return;

		initialized = true;
		if (GetConsoleWindow() == nullptr && AllocConsole() == FALSE)
			return;

		FILE* stream = nullptr;
		freopen_s(&stream, "CONOUT$", "w", stdout);
		freopen_s(&stream, "CONOUT$", "w", stderr);
		std::cout.clear();
		std::cerr.clear();
		SetConsoleTitleW(L"ProjectCane - Shader Error");

		if (HWND consoleWindow = GetConsoleWindow())
		{
			ShowWindow(consoleWindow, SW_SHOW);
			SetForegroundWindow(consoleWindow);
		}
		#endif
	}

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
		#ifdef CANE_PORTABLE_SHADERS
		return CaneEmbeddedShader(filename);
		#else
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

		const char* source = static_cast<const char*>(resourceData);
		size_t sourceSize = resourceSize;

		// Some editors save GLSL as UTF-8 with a BOM. NVIDIA accepts the BOM
		// before #version, but AMD's GLSL compiler treats it as an unexpected
		// token. Strip it after loading so the embedded resource is portable.
		if (sourceSize >= 3 &&
			static_cast<unsigned char>(source[0]) == 0xef &&
			static_cast<unsigned char>(source[1]) == 0xbb &&
			static_cast<unsigned char>(source[2]) == 0xbf)
		{
			source += 3;
			sourceSize -= 3;
		}

		return std::string(source, sourceSize);
		#endif
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
		if (vertexShader == 0)
			throw std::runtime_error(std::string("Could not create vertex shader: ") + vertexFile);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);
		if (!compileErrors(vertexShader, "VERTEX", vertexFile))
		{
			glDeleteShader(vertexShader);
			throw std::runtime_error(std::string("Vertex shader compilation failed: ") + vertexFile);
		}
	}

	GLuint geometryShader = NULL;

	if (geometryFile != NULL)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		if (geometryShader == 0)
			throw std::runtime_error(std::string("Could not create geometry shader: ") + geometryFile);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		glCompileShader(geometryShader);
		if (!compileErrors(geometryShader, "GEOMETRY", vertexFile, geometryFile))
		{
			if (vertexShader != 0) glDeleteShader(vertexShader);
			glDeleteShader(geometryShader);
			throw std::runtime_error(std::string("Geometry shader compilation failed: ") + geometryFile);
		}
	}

	GLuint fragmentShader = NULL;

	if (fragmentFile != NULL)
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		if (fragmentShader == 0)
			throw std::runtime_error(std::string("Could not create fragment shader: ") + fragmentFile);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);
		if (!compileErrors(fragmentShader, "FRAGMENT", vertexFile, geometryFile, fragmentFile))
		{
			if (vertexShader != 0) glDeleteShader(vertexShader);
			if (geometryShader != 0) glDeleteShader(geometryShader);
			glDeleteShader(fragmentShader);
			throw std::runtime_error(std::string("Fragment shader compilation failed: ") + fragmentFile);
		}
	}

	ID = glCreateProgram();
	if (ID == 0)
	{
		if (vertexShader != 0) glDeleteShader(vertexShader);
		if (geometryShader != 0) glDeleteShader(geometryShader);
		if (fragmentShader != 0) glDeleteShader(fragmentShader);
		throw std::runtime_error("Could not create shader program");
	}

	if (vertexFile != NULL)
		glAttachShader(ID, vertexShader);

	if (geometryFile != NULL)
		glAttachShader(ID, geometryShader);

	if (fragmentFile != NULL)
		glAttachShader(ID, fragmentShader);

	glLinkProgram(ID);
	if (!compileErrors(ID, "PROGRAM", vertexFile, geometryFile, fragmentFile))
	{
		if (vertexShader != 0) glDeleteShader(vertexShader);
		if (geometryShader != 0) glDeleteShader(geometryShader);
		if (fragmentShader != 0) glDeleteShader(fragmentShader);
		glDeleteProgram(ID);
		ID = 0;
		throw std::runtime_error("Shader program linking failed");
	}

	if (vertexFile != NULL)
		glDeleteShader(vertexShader);

	if (geometryFile != NULL)
		glDeleteShader(geometryShader);

	if (fragmentFile != NULL)
		glDeleteShader(fragmentShader);

}

void GLSHADER::Use()
{
	if (s_activeShaderProgram != ID)
	{
		glUseProgram(ID);
		s_activeShaderProgram = ID;
	}
}

void GLSHADER::Delete()
{
	if (s_activeShaderProgram == ID)
		s_activeShaderProgram = (std::numeric_limits<GLuint>::max)();
	glDeleteProgram(ID);
}

void InvalidateShaderProgramCache()
{
	s_activeShaderProgram = (std::numeric_limits<GLuint>::max)();
}

bool GLSHADER::compileErrors(unsigned int shader, const char* type,
	const char* vertexFile, const char* geometryFile, const char* fragmentFile)
{
	GLint hasCompiled;

	if (std::strcmp(type, "PROGRAM") != 0)
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			const char* shaderFile = vertexFile;
			if (std::strcmp(type, "GEOMETRY") == 0)
				shaderFile = geometryFile;
			else if (std::strcmp(type, "FRAGMENT") == 0)
				shaderFile = fragmentFile;

			GLint logLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			std::string infoLog(static_cast<size_t>((std::max)(logLength, 1)), '\0');
			glGetShaderInfoLog(shader, logLength, NULL, infoLog.data());
			ShowShaderErrorConsole();
			std::cout << "SHADER_COMPILATION_ERROR for " << type << ": "
				<< (shaderFile != nullptr ? shaderFile : "<unknown>") << "\n"
				<< infoLog << std::endl;
			return false;
		}
	}

	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			GLint logLength = 0;
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			std::string infoLog(static_cast<size_t>((std::max)(logLength, 1)), '\0');
			glGetProgramInfoLog(shader, logLength, NULL, infoLog.data());
			ShowShaderErrorConsole();
			std::cout << "SHADER_LINKING_ERROR for: "
				<< (vertexFile != nullptr ? vertexFile : "<none>") << " + ";
			if (geometryFile != nullptr)
				std::cout << geometryFile << " + ";
			std::cout << (fragmentFile != nullptr ? fragmentFile : "<none>") << '\n';

			const GLubyte* vendor = glGetString(GL_VENDOR);
			const GLubyte* renderer = glGetString(GL_RENDERER);
			const GLubyte* version = glGetString(GL_VERSION);
			std::cout << "OpenGL vendor: " << (vendor != nullptr ? reinterpret_cast<const char*>(vendor) : "<unknown>") << '\n'
				<< "OpenGL renderer: " << (renderer != nullptr ? reinterpret_cast<const char*>(renderer) : "<unknown>") << '\n'
				<< "OpenGL version: " << (version != nullptr ? reinterpret_cast<const char*>(version) : "<unknown>") << '\n';

			GLint vertexSsboBlocks = 0;
			GLint combinedSsboBlocks = 0;
			GLint vertexOutputComponents = 0;
			glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &vertexSsboBlocks);
			glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &combinedSsboBlocks);
			glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &vertexOutputComponents);
			std::cout << "GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS: " << vertexSsboBlocks << '\n'
				<< "GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS: " << combinedSsboBlocks << '\n'
				<< "GL_MAX_VERTEX_OUTPUT_COMPONENTS: " << vertexOutputComponents << '\n'
				<< infoLog << std::endl;
			return false;
		}
	}

	return true;
}

GLSHADER glScreenShader;
GLSHADER glDyshadow;
GLSHADER glGlobShader;
GLSHADER glCelBorderShader;
GLSHADER glGeomShader;
GLSHADER glBlotShader;
GLSHADER glBlipShader;
