#pragma once

#include "Name.h"
#include "../Serialization.h"

template<>
inline bool SerializationInfo::get<Name>(const Path& path, Name& object) const
{
    if (path.empty())
        return false;
    if (path.size() == 1) {
        auto iter = stringFeild.find(path[0]);
        if (iter == stringFeild.end())
            return false;
        object = stringList[iter->second];
        return true;
    }
    else {
        auto _iter = subfeilds.find(path[0]);
        if (_iter == subfeilds.end())
            return false;
        return sublists[_iter->second].get<Name>(path.popTop(), object);
    }
}

template<>
inline bool SerializationInfo::get<Name>(const size_t i, Name& object) const
{
    if (i < stringList.size()) {
        object = stringList[i];
        return true;
    }
    return false;
}
