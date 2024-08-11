#pragma once
#include "CodeGenerationInterface.h"

class ICodeWriter
{
public:
    enum ScopeType
    {
        BlankScope, FunctionScope, StructScope
    };
    
    virtual ~ICodeWriter() = default;

    virtual Name convertKeyword(const Name& keyword) = 0;
    virtual int getOperatorParamNum(const Name& op) = 0;
    virtual const char* getOperatorFormatter(const Name& op) = 0;

    virtual void write(const char* fmt_str, ...) = 0;
    virtual void write(const char* fmt_str, va_list ap) = 0;
    virtual ICodeWriter* subscope(ScopeType type) = 0;
    virtual void beginExpression(const char* fmt_str = NULL, ...) = 0;
    virtual void endExpression(const char* ender = NULL) = 0;
    virtual void writeSymbolDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier = CQF_None) = 0;
    virtual void writeParameterDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier = CQF_None) = 0;
    virtual void writeParameter(const CodeParameter& param, ICodeScopeBackend& backend) = 0;

    virtual void getString(std::string& str) = 0;
    virtual void writeTo(std::ostream& os) = 0;
};
