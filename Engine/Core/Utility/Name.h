#pragma once
#include <iostream>
#include <unordered_map>

typedef size_t NameHash;

class ENGINE_API NamePool
{
public:
    NamePool() = default;

    static NamePool& get();

    static NameHash calHash(const char* str);

    std::pair<NameHash, const char*> registerString(const char* str);
protected:
    std::unordered_map<NameHash, std::string> nameMap;
};

struct ENGINE_API Name
{
    static const Name none;
    
    Name();
    Name(const char* str);
    Name(const std::string& str);

    bool isNone() const;
    bool empty() const;

    bool operator==(const Name& name) const;
    bool operator<(const Name& name) const;

    NameHash getHash() const;
    const char* c_str() const;
protected:
    NameHash hash;
    const char* ptr;
};

template<>
struct ENGINE_API std::hash<Name>
{
    size_t operator()(const Name& name) const noexcept
    {
        return name.getHash();
    }
};

ENGINE_API std::string operator+(const Name& name, const char* c_str);
ENGINE_API std::string operator+(const char* c_str, const Name& name);
ENGINE_API std::string operator+(const Name& name, const std::string& str);
ENGINE_API std::string operator+(const std::string& str, const Name& name);

ENGINE_API Name operator ""_N(const char* c_str, size_t);
