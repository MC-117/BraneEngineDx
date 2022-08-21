#pragma once
#include <iostream>

struct Guid
{
    union {
        struct {
            unsigned long  Data1;
            unsigned short Data2;
            unsigned short Data3;
            unsigned char  Data4[8];
        };
        struct {
            unsigned long long Word0;
            unsigned long long Word1;
        };
    };

    static Guid newGuid();
    static Guid fromString(const std::string& str);
    std::string toString() const;

    bool isDefault() const;

    Guid& operator=(const Guid& guid);
    bool operator==(const Guid& guid) const;
    bool operator!=(const Guid& guid) const;
    bool operator<(const Guid& guid) const;
};