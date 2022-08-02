#include "Guid.h"
#include <objbase.h>
#include <string>

Guid Guid::newGuid()
{
    Guid guid = { 0 };
    CoCreateGuid((GUID*)&guid);
    return guid;
}

Guid Guid::fromString(const std::string& str)
{
    Guid guid = { 0 };
    const int ret = sscanf_s(str.c_str(), "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
        &guid.Data1, &guid.Data2, &guid.Data3, &guid.Data4[0], &guid.Data4[1], &guid.Data4[2],
        &guid.Data4[3], &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);
    if (ret != 11)
        return guid;
    return guid;
}

std::string Guid::toString() const
{
    std::string str = std::string(36, '0');
    snprintf(str.data(), 37, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
        Data1, Data2, Data3, Data4[0], Data4[1], Data4[2], Data4[3], Data4[4],
        Data4[5], Data4[6], Data4[7]);
    return str;
}

bool Guid::isDefault() const
{
    unsigned long long* num0 = (unsigned long long*) & Data1;
    unsigned long long* num1 = (unsigned long long*)Data4;

    return *num0 == 0 && *num1 == 0;
}

Guid& Guid::operator=(const Guid& guid)
{
    unsigned long long* num0 = (unsigned long long*) & Data1;
    unsigned long long* num1 = (unsigned long long*)Data4;
    unsigned long long* _num0 = (unsigned long long*) & guid.Data1;
    unsigned long long* _num1 = (unsigned long long*)guid.Data4;
    *num0 = *_num0;
    *num1 = *_num1;
    return *this;
}

bool Guid::operator==(const Guid& guid) const
{
    unsigned long long* num0 = (unsigned long long*) & Data1;
    unsigned long long* num1 = (unsigned long long*)Data4;
    unsigned long long* _num0 = (unsigned long long*) & guid.Data1;
    unsigned long long* _num1 = (unsigned long long*)guid.Data4;
    return *num0 == *_num0 && *num1 == *_num1;
}

bool Guid::operator!=(const Guid& guid) const
{
    unsigned long long* num0 = (unsigned long long*) & Data1;
    unsigned long long* num1 = (unsigned long long*)Data4;
    unsigned long long* _num0 = (unsigned long long*) & guid.Data1;
    unsigned long long* _num1 = (unsigned long long*)guid.Data4;
    return *num0 != *_num0 || *num1 != *_num1;
}

bool Guid::operator<(const Guid& guid) const
{
    unsigned long long* num0 = (unsigned long long*)&Data1;
    unsigned long long* num1 = (unsigned long long*)Data4;
    unsigned long long* _num0 = (unsigned long long*)&guid.Data1;
    unsigned long long* _num1 = (unsigned long long*)guid.Data4;
    if (*num0 < *_num0)
        return true;
    else if (*num0 == *_num0)
        return *num1 < *_num1;
    return false;
}
