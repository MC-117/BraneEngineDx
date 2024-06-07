#pragma once
#include <vector>
#include "../Utility/Name.h"
#include "../Utility/Decimal.h"

namespace Code
{
    static const Name float_t = "float";
    static const Name int_t = "int";
    static const Name bool_t = "bool";
    static const Name string_t = "string";
    static const Name char_t = "char";
    static const Name KeyCode_t = "KeyCode";
    static const Name true_t = "true";
    static const Name false_t = "false";
    static const Name Vector2f_t = "Vector2f";
    static const Name Vector3f_t = "Vector3f";
    static const Name Vector4f_t = "Vector4f";
    static const Name Quaternionf_t = "Quaternionf";
    static const Name Color_t = "Color";
    
    const Name assign_op = "_assign_op_";
    const Name access_op = "_access_op_";
    const Name add_op = "_add_op_";
    const Name sub_op= "_sub_op_";
    const Name mul_op = "_mul_op_";
    const Name div_op = "_div_op_";
    const Name mod_op = "_mod_op_";
    const Name minus_op = "_minus_op_";
    const Name lShift_op = "_l_shift_op_";
    const Name rShift_op = "_r_shift_op_";
    const Name gre_op = "_gre_op_";
    const Name les_op = "_les_op_";
    const Name eq_op = "_eq_op_";
    const Name notEq_op = "_not_eq_op_";
    const Name greEq_op = "_gre_eq_op_";
    const Name lesEq_op = "_les_eq_op_";
    const Name not_op = "_not_op_";
    const Name and_op = "_and_op_";
    const Name or_op = "_or_op_";
    const Name xor_op = "_xor_op_";
    const Name cond_op = "_cond_op_";

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
    enum Type
    {
        Symbol_t,
        Number_t,
        Bool_t,
        Char_t,
        String_t,
        Expression_t,
    };

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

    bool isType(Type type) const;

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
    Type type;
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
    CodeParameter operation;
    bool isExpression;
    std::vector<CodeParameter> parameters;
    std::vector<Name> outputs;

    CodeFunctionInvocation(const CodeParameter& operation = Name::none, bool isExpression = false);

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
