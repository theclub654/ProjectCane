#pragma once

#include <fstream>
#include <string>

typedef unsigned char byte;

class CBinaryInputStream
{
	public:

		// File Object
		std::ifstream file;

		CBinaryInputStream(std::string fileName);

		void Read(int numBytes, void *pv);
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
		// Reads a string from file
		void ReadStringSw(std::string *pachz);
		// Close and destroy binary stream
		void Close();

		~CBinaryInputStream();
};