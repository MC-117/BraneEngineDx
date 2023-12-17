#pragma once
#include "CodeGenerationInterface.h"

class ICodeWriter
{
public:
    virtual ~ICodeWriter() = default;

    virtual void write(const char* fmt_str, ...) = 0;
    virtual ICodeWriter* subscope() = 0;
    virtual void beginExpression(const char* fmt_str = NULL, ...) = 0;
    virtual void endExpression(const char* ender = NULL) = 0;
    virtual void writeInParameter(const CodeSymbolDefinition& definition) = 0;
    virtual void writeOutParameter(const CodeSymbolDefinition& definition) = 0;
    virtual void writeParameter(const CodeParameter& param, ICodeScopeBackend& backend) = 0;

    virtual void getString(std::string& str) = 0;
    virtual void writeTo(std::ostream& os) = 0;
};
