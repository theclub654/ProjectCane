#pragma once
#include <fstream>
#include "glm/glm.hpp"
typedef unsigned char byte;

class CBinaryInputStream
{
	public:

		// File Object
		std::ifstream file;

		CBinaryInputStream(std::string filePath);

		// Align bytes to n
		void Align(int n);
		// Reads 8 unsigned bytes from file
		byte U8Read();
		// Reads 16 unsigned bytes from file
		uint16_t U16Read();
		// Reads 32 unsigned bytes from file
		uint32_t U32Read();
		// Reads 8 signed bytes from file
		int8_t S8Read();
		// Reads 16 signed bytes from file
		int16_t S16Read();
		// Reads 32 signed bytes from file
		int32_t S32Read();
		// Reads float bytes from file
		float F32Read();
		// Reads Vector 2 from file
		glm::vec2 ReadVector2();
		// Reads vector from file
		glm::vec3 ReadVector();
		// Reads vector 4 from file
		glm::vec4 ReadVector4();
		// Reads matrix from file
		glm::mat3 ReadMatrix();
		// Reads matrix 4 from file
		glm::mat4 ReadMatrix4();
		// Reads a string from file
		void ReadStringSw();
		// Close and destroy binary stream
		void Close();

		~CBinaryInputStream();
};