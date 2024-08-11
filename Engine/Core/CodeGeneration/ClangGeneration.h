#pragma once
#include <sstream>
#include <stack>
#include "CodeGenerationInterface.h"
#include "ICodeWriter.h"

class ENGINE_API ClangWriter : public ICodeWriter
{
public:
    ClangWriter();
    virtual ~ClangWriter();

    virtual Name convertKeyword(const Name& keyword);
    virtual int getOperatorParamNum(const Name& op);
    virtual const char* getOperatorFormatter(const Name& op);

    virtual void write(const char* fmt_str, ...);
    virtual void write(const char* fmt_str, va_list ap);
    virtual ICodeWriter* subscope(ScopeType type);
    virtual void beginExpression(const char* fmt_str = NULL, ...);
    virtual void endExpression(const char* ender = NULL);
    virtual void writeSymbolDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier = CQF_None);
    virtual void writeParameterDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier = CQF_None);
    virtual void writeParameter(const CodeParameter& param, ICodeScopeBackend& backend);

    virtual void getString(std::string& str);
    virtual void writeTo(std::ostream& os);
protected:
    std::stringstream firstbuffer;
    struct Scope
    {
        ClangWriter* scopeWriter = NULL;
        std::stringstream buffer;
    };
    std::list<Scope> scopes;
    
    int indent = 0;
    int expressionCount = 0;
    std::stack<int> expressionCountStack;

    std::stringstream& output();

    virtual void writeIndent();
    virtual ClangWriter* newWriter();
};

class ENGINE_API ClangScopeBackend : public ICodeScopeBackend
{
public:
    ClangScopeBackend(ICodeWriter& writer, const CodeFunctionSignature& signature = CodeFunctionSignature::none);
    virtual ~ClangScopeBackend();

    virtual Name newTempVariableName();
    virtual Name newTempFunctionName();

    virtual Name convertKeyword(const Name& keyword);
    virtual std::string convertString(const char* str);
    virtual std::string convertCharacter(char character);
    virtual std::string convertExpression(const CodeFunctionInvocation& invocation);

    virtual void write(const char* fmt_str, ...);
    virtual ICodeScopeBackend* subscope();
    virtual bool declareVariable(const CodeSymbolDefinition& definition, const CodeParameter& defaultValue = CodeParameter::none);
    virtual ICodeScopeBackend* declareFunction(const CodeFunctionSignature& signature);
    virtual bool invoke(const CodeFunctionInvocation& invocation);
    virtual bool branch(const std::vector<CodeParameter>& conditionParams, std::vector<ICodeScopeBackend*>& scopes);
    virtual ICodeScopeBackend* loop(const CodeParameter& conditionParam);
    virtual bool jumpOut();
    virtual bool output(const std::vector<CodeParameter>& returnValues, bool doCheck = true);
protected:
    static Name _builtin_function_type_;
    ICodeWriter& writer;
    CodeFunctionSignature scopeSignature;
    std::unordered_map<Name, CodeSymbolDefinition> symbolTable;
    std::unordered_map<Name, CodeFunctionSignature> funcitonTable;
    std::vector<ClangScopeBackend*> subscopes;

    Name newSymbolName(const char* name);

    bool registerSymbol(const CodeSymbolDefinition& symbol);
    bool registerFunction(const CodeFunctionSignature& signature);
    bool checkSymbol(const Name& name);
    bool checkParameter(const CodeParameter& param);

    virtual ICodeScopeBackend* subscope(const CodeFunctionSignature& signature);
};
