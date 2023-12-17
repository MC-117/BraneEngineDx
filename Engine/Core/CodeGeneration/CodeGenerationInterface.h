#pragma once
#include <vector>
#include "../Utility/Name.h"
#include "../Utility/Decimal.h"

namespace Code
{
    ENGINE_API extern const Name float_t;
    ENGINE_API extern const Name int_t;
    ENGINE_API extern const Name bool_t;
    ENGINE_API extern const Name string_t;
    ENGINE_API extern const Name char_t;
    ENGINE_API extern const Name true_t;
    ENGINE_API extern const Name false_t;
    ENGINE_API extern const Name Vector2f_t;
    ENGINE_API extern const Name Vector3f_t;
    ENGINE_API extern const Name Vector4f_t;
    ENGINE_API extern const Name Quaternionf_t;
    ENGINE_API extern const Name Color_t;

    ENGINE_API extern const Name assign_op;
    ENGINE_API extern const Name add_op;
    ENGINE_API extern const Name sub_op;
    ENGINE_API extern const Name mul_op;
    ENGINE_API extern const Name div_op;
    ENGINE_API extern const Name mod_op;
    ENGINE_API extern const Name minus_op;
    ENGINE_API extern const Name lShift_op;
    ENGINE_API extern const Name rShift_op;
    ENGINE_API extern const Name gre_op;
    ENGINE_API extern const Name les_op;
    ENGINE_API extern const Name eq_op;
    ENGINE_API extern const Name notEq_op;
    ENGINE_API extern const Name greEq_op;
    ENGINE_API extern const Name lesEq_op;
    ENGINE_API extern const Name not_op;
    ENGINE_API extern const Name and_op;
    ENGINE_API extern const Name or_op;
    ENGINE_API extern const Name xor_op;
    ENGINE_API extern const Name cond_op;

    ENGINE_API int getCodeOperatorNum(const Name& operatorType);
};

struct ENGINE_API CodeSymbolDefinition
{
    Name type;
    Name name;

    CodeSymbolDefinition() = default;
    CodeSymbolDefinition(const Name& type, const Name& name);
};

struct ENGINE_API CodeFunctionSignature
{
    Name name;
    std::vector<CodeSymbolDefinition> parameters;
    std::vector<CodeSymbolDefinition> outputs;

    static CodeFunctionSignature none;

    CodeFunctionSignature(const Name& name = Name::none);

    bool isValid() const;

    template<class ... Params>
    inline CodeFunctionSignature& param(Params ... params)
    {
        parameters = { params... };
        return *this;
    }

    template<class ... Outputs>
    inline CodeFunctionSignature& out(Outputs ... outs)
    {
        outputs = { outs... };
        return *this;
    }
};

struct ENGINE_API CodeBool
{
    bool value;

    explicit CodeBool(bool value);
};

struct ENGINE_API CodeChar
{
    char value;

    explicit CodeChar(char value);
};

struct ENGINE_API CodeFunctionInvocation;
class ICodeScopeBackend;

class ENGINE_API CodeParameter
{
public:

    static const CodeParameter none;

    CodeParameter(Name symbol);
    CodeParameter(Decimal number);
    CodeParameter(CodeBool boolean);
    CodeParameter(CodeChar character);
    CodeParameter(const char* string);
    CodeParameter(const std::string& string);
    CodeParameter(const CodeFunctionInvocation& expression);
    CodeParameter(const CodeParameter& param);
    CodeParameter(CodeParameter&& param);
    ~CodeParameter();

    bool isValid() const;

    Name& symbol();
    const Name& symbol() const;

    Decimal& number();
    const Decimal& number() const;

    bool& boolean();
    const bool& boolean() const;

    char& character();
    const char& character() const;

    char*& string();
    char* const& string() const;

    CodeFunctionInvocation& expression();
    const CodeFunctionInvocation& expression() const;

    CodeParameter& assign(const CodeParameter& param);
    CodeParameter& assign(CodeParameter&& param);

    CodeParameter& operator=(const CodeParameter& param);
    CodeParameter& operator=(CodeParameter&& param);

    std::string toString(ICodeScopeBackend& backend) const;

    std::ostream& write(std::ostream& os, ICodeScopeBackend& backend) const;
protected:
    enum Type
    {
        Symbol_t,
        Number_t,
        Bool_t,
        Char_t,
        String_t,
        Expression_t,
    } type;
    
    union
    {
        Name _symbol;
        Decimal _number;
        bool _boolean;
        char _character;
        char* _string;
        CodeFunctionInvocation* _expression;
    };
};

struct ENGINE_API CodeFunctionInvocation
{
    Name name;
    bool isExpression;
    std::vector<CodeParameter> parameters;
    std::vector<Name> outputs;

    CodeFunctionInvocation(const Name& name = Name::none, bool isExpression = false);

    template<class ... Params>
    inline CodeFunctionInvocation& param(Params ... params)
    {
        parameters = std::vector<CodeParameter>({ params... });
        return *this;
    }

    template<class ... Outputs>
    inline CodeFunctionInvocation& out(Outputs ... outs)
    {
        outputs = std::vector<Name>({ outs... });
        return *this;
    }
};

class ICodeScopeBackend
{
public:
    virtual ~ICodeScopeBackend() = default;

    virtual Name newTempVariableName() = 0;
    virtual Name newTempFunctionName() = 0;

    virtual Name convertKeyword(const Name& keyword) = 0;
    virtual std::string convertString(const char* str) = 0;
    virtual std::string convertCharacter(char character) = 0;
    virtual std::string convertExpression(const CodeFunctionInvocation& invocation) = 0;

    virtual ICodeScopeBackend* subscope() = 0;
    virtual bool declareVariable(const CodeSymbolDefinition& definition, const CodeParameter& defaultValue = CodeParameter::none) = 0;
    virtual ICodeScopeBackend* declareFunction(const CodeFunctionSignature& signature) = 0;
    virtual bool invoke(const CodeFunctionInvocation& invocation) = 0;
    virtual bool branch(const std::vector<CodeParameter>& conditionParams, std::vector<ICodeScopeBackend*>& scopes) = 0;
    virtual ICodeScopeBackend* loop(const CodeParameter& conditionParam) = 0;
    virtual bool jumpOut() = 0;
    virtual bool output(const std::vector<CodeParameter>& returnValues) = 0;
};

#include "../Serialization.h"

class ENGINE_API CodeFunctionNameAttribute : public Attribute
{
public:
    Name funcName;
    
    CodeFunctionNameAttribute(Name funcName);
    
    virtual void resolve(const Attribute& conflict);
};
