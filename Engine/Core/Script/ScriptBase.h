#pragma once

#include "../Base.h"

class ScriptBase : public Base
{
public:
    Serialize(ScriptBase, Base);
    
    virtual ~ScriptBase() = default;
    
    virtual bool isValid() const = 0;
    virtual bool load(const std::string& file) = 0;
    virtual const std::string& getName() const = 0;
    virtual const std::string& getCodePath() const = 0;
    virtual const std::string& getSourceCode() const = 0;
    virtual bool setSourceCode(const std::string& code) = 0;
    virtual bool refresh() = 0;
    virtual bool saveSourceCode() = 0;

    static Serializable* instantiate(const SerializationInfo& from);
};
