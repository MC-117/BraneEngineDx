#pragma once
#include "../../CodeGeneration/ClangGeneration.h"
#include "ShaderCodeGeneration.h"

class ENGINE_API HLSLWriter : public ClangWriter
{
public:
    virtual Name convertKeyword(const Name& keyword);
    virtual int getOperatorParamNum(const Name& op);
    virtual const char* getOperatorFormatter(const Name& op);

    virtual void writeSymbolDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier = CQF_None);
protected:
    virtual ClangWriter* newWriter();
};
