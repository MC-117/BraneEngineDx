#include "IOUtility.h"

const char pad[15] = { 0 };

void alignStream(std::istream& is, uint8_t size)
{
    uint8_t mod = is.tellg() % size;
    if (mod > 0) {
        is.seekg(size - mod, std::ios::cur);
    }
}

void alignStream(std::ostream& os, uint8_t size)
{
    uint8_t mod = os.tellp() % size;
    if (mod > 0) {
        os.write(pad, size - mod);
    }
}

void align16(std::istream& is)
{
    alignStream(is, 16);
}

void align16(std::ostream& os)
{
    alignStream(os, 16);
}

void align16(std::ostream& os, uint64_t size)
{
    uint8_t mod = size % 16;
    if (mod > 0) {
        os.write(pad, 16 - mod);
    }
}

uint64_t calAlign(uint64_t size, uint32_t alignSize)
{
    uint32_t mod = size % alignSize;
    return mod == 0 ? size : (size + alignSize - mod);
}

uint64_t calAlign16(uint64_t size)
{
    uint8_t mod = size % 16;
    return mod == 0 ? size : (size + 16 - mod);
}

uint64_t calAlignString(std::string str)
{
    uint64_t size = sizeof(uint32_t) + str.length();
    return calAlign(size, 4);
}

void readStream(std::istream& is, char* src, std::streamsize size)
{
    is.read(src, size);
}

void peekStream(std::istream& is, char* src, std::streamsize size)
{
    size_t pos = is.tellg();
    readStream(is, src, size);
    is.seekg(pos);
}

void writeStream(std::ostream& os, const char* src, std::streamsize size)
{
    os.write(src, size);
}

void readString(std::string& str, std::istream& is)
{
    str.clear();
    while (true) {
        char c;
        is.read(&c, sizeof(char));
        if (c == '\0')
            break;
        str += c;
    }
}

void writeString(const std::string& str, std::ostream& os)
{
    const char end = '\0';
    os.write(str.c_str(), str.size() * sizeof(char));
    os.write(&end, sizeof(char));
}

void swapBytes(char* bytes, unsigned int size)
{
    unsigned int n = size / 2;
    for (int i = 0; i < n; ++i) {
        char b = bytes[i];
        bytes[i] = bytes[size - i - 1];
        bytes[size - i - 1] = b;
    }
}

void readAlignedString(std::istream& is, std::string& str)
{
    uint32_t length = 0;
    readStream(is, length);
    if (length > 0) {
        str.resize(length);
        is.read((char*)str.data(), sizeof(uint8_t) * length);
        alignStream(is, 4);
    }
}
void writeAlignedString(std::ostream& os, const std::string& str)
{
    uint32_t length = str.length();
    writeStream(os, length);
    if (length > 0) {
        os.write(str.data(), sizeof(uint8_t) * length);
        alignStream(os, 4);
    }
}
