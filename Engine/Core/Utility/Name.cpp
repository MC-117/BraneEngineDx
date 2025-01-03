﻿#include "Name.h"
#include "hash.h"

NamePool& NamePool::get()
{
    static NamePool namePool;
    return namePool;
}

NameHash NamePool::calHash(const char* str)
{
    const char* ptr = str;
    NameHash hash = 0;
    while (true) {
        NameHash data = 0;
        int len = 0;
        for (; len < sizeof(NameHash); len++) {
            if (ptr[len] == '\0')
                break;
        }
        memcpy(&data, ptr, len);
        hash_combine(hash, data);
        if (len < sizeof(NameHash))
            break;
        ptr += sizeof(NameHash);
    }
    return hash;
}

std::pair<NameHash, const char*> NamePool::registerString(const char* str)
{
    if (str == NULL)
        return { 0, NULL };
    const NameHash hash = calHash(str);
    if (hash == 0)
        return { 0, "" };
    std::string& name = nameMap[hash];
    if (name.empty())
        name = str;
    return { hash, name.c_str() };
}

const Name Name::none;

Name::Name()
    : hash(0)
    , ptr(NULL)
{
}

Name::Name(const char* str)
{
    if (strlen(str) == 0) {
        hash = 0;
        ptr = NULL;
    }
    else {
        const std::pair<NameHash, const char*> pair = NamePool::get().registerString(str);
        hash = pair.first;
        ptr = pair.second;
    }
}

Name::Name(const std::string& str)
{
    if (str.empty()) {
        hash = 0;
        ptr = NULL;
    }
    else {
        const std::pair<NameHash, const char*> pair = NamePool::get().registerString(str.c_str());
        hash = pair.first;
        ptr = pair.second;
    }
}

bool Name::isNone() const
{
    return hash == 0;
}

bool Name::empty() const
{
    return isNone();
}

bool Name::operator==(const Name& name) const
{
    return hash == name.hash;
}

bool Name::operator<(const Name& name) const
{
    return hash < name.hash;
}

NameHash Name::getHash() const
{
    return hash;
}

const char* Name::c_str() const
{
    return ptr == NULL ? "" : ptr;
}

inline std::string operator+(const Name& name, const char* c_str)
{
    return std::string(name.c_str()) + c_str;
}

inline std::string operator+(const char* c_str, const Name& name)
{
    return c_str + std::string(name.c_str());
}

inline std::string operator+(const Name& name, const std::string& str)
{
    return name.c_str() + str;
}

inline std::string operator+(const std::string& str, const Name& name)
{
    return str + name.c_str();
}

Name operator ""_N(const char* c_str, size_t)
{
    return Name(c_str);
}
