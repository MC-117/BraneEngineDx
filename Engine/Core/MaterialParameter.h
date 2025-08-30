#pragma once

#include "Unit.h"
#include "Utility/Name.h"

template<class T>
class MaterialParameter
{
public:
    MaterialParameter();

    void setValue(const T& value)
    {
        
        value_GameThread = value;
    }
protected:
    Name name;
    T value_GameThread;
    T value_RenderThread;
};
