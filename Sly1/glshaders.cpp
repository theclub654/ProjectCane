#include "glshaders.h"

void GLSHADER::Init(const char* vertexFile, const char* geometryFile, const char* fragmentFile)
{
	std::string geometryCode{};

	std::string vertexCode = get_file_contents(vertexFile);

	if (geometryFile != NULL)
		geometryCode = get_file_contents(fragmentFile);

	std::string fragmentCode = get_file_contents(fragmentFile);

	const char* geometrySource{};
	const char* vertexSource = vertexCode.c_str();

	if (geometryFile != NULL)
		geometrySource = geometryCode.c_str();

	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	GLuint geometryShader{};

	if (geometryFile != NULL)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		glCompileShader(geometryShader);
		compileErrors(geometryShader, "GEOMETRY");
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);

	if (geometryFile != NULL)
		glAttachShader(ID, geometryShader);

	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);

	if (geometryFile != NULL)
		glDeleteShader(geometryShader);

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

std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}