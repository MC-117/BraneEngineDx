#pragma once
#include <iostream>

ENGINE_API void alignStream(std::istream& is, uint8_t size);
ENGINE_API void alignStream(std::ostream& os, uint8_t size);
ENGINE_API void align16(std::istream& is);
ENGINE_API void align16(std::ostream& os);
ENGINE_API void align16(std::ostream& os, uint64_t size);
ENGINE_API uint64_t calAlign(uint64_t size, uint32_t alignSize);
ENGINE_API uint64_t calAlign16(uint64_t size);
ENGINE_API uint64_t calAlignString(std::string str);

ENGINE_API void swapBytes(char* bytes, unsigned int size);

ENGINE_API void readStream(std::istream& is, char* src, std::streamsize size);
ENGINE_API void peekStream(std::istream& is, char* src, std::streamsize size);
ENGINE_API void writeStream(std::ostream& os, const char* src, std::streamsize size);

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

ENGINE_API void readString(std::string& str, std::istream& is);
ENGINE_API void writeString(const std::string& str, std::ostream& os);
ENGINE_API void readAlignedString(std::istream& is, std::string& str);
ENGINE_API void writeAlignedString(std::ostream& os, const std::string& str);