#pragma once
#include <iostream>

void alignStream(std::istream& is, uint8_t size);
void alignStream(std::ostream& os, uint8_t size);
void align16(std::istream& is);
void align16(std::ostream& os);
void align16(std::ostream& os, uint64_t size);
uint64_t calAlign(uint64_t size, uint32_t alignSize);
uint64_t calAlign16(uint64_t size);
uint64_t calAlignString(std::string str);

void swapBytes(char* bytes, unsigned int size);

void readStream(std::istream& is, char* src, std::streamsize size);
void peekStream(std::istream& is, char* src, std::streamsize size);
void writeStream(std::ostream& os, const char* src, std::streamsize size);

template<class T>
void readStream(std::istream& is, T& value)
{
	readStream(is, (char*)&value, sizeof(T));
}
template<class T>
void peekStream(std::istream& is, T& value)
{
	peekStream(is, (char*)&value, sizeof(T));
}
template<class T>
void writeStream(std::ostream& os, const T& value)
{
	writeStream(os, (const char*)&value, sizeof(T));
}

template<class T>
void readArrayStream(std::istream& is, T* value, uint32_t count)
{
	readStream(is, (char*)value, sizeof(T) * count);
}
template<class T>
void writeArrayStream(std::ostream& os, const T* value, uint32_t count)
{
	writeStream(os, (const char*)value, sizeof(T) * count);
}

void readString(std::string& str, std::istream& is);
void writeString(const std::string& str, std::ostream& os);
void readAlignedString(std::istream& is, std::string& str);
void writeAlignedString(std::ostream& os, const std::string& str);