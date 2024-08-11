#include "CodeGenerationInterface.h"
#include <cassert>

namespace Code
{
    int getCodeOperatorNum(const Name& operatorType)
    {
        static const unordered_map<Name, int> formatterMap = {
            { Code::assign_op, 2 },
            { Code::access_op, 2 },
            { Code::add_op, 2 },
            { Code::sub_op, 2 },
            { Code::mul_op, 2 },
            { Code::div_op, 2 },
            { Code::mod_op, 2 },
            { Code::lShift_op, 2 },
            { Code::rShift_op, 2 },
            { Code::gre_op, 2 },
            { Code::les_op, 2 },
            { Code::eq_op, 2 },
            { Code::notEq_op, 2 },
            { Code::greEq_op, 2 },
            { Code::lesEq_op, 2 },
            { Code::and_op, 2 },
            { Code::or_op, 2 },
            { Code::xor_op, 2 },
            { Code::minus_op, 1 },
            { Code::not_op, 1 },
            { Code::cond_op, 3 },
        };

        auto iter = formatterMap.find(operatorType);
        if (iter != formatterMap.end())
            return iter->second;
        return 0;
    }
}

CodeSymbolDefinition::CodeSymbolDefinition(const Name& type, const Name& name, Enum<CodeQualifierFlags> qualifiers)
    : type(type), name(name), qualifiers(qualifiers)
{
}

CodeFunctionSignature::CodeFunctionSignature(const Name& name) : name(name)
{
}

CodeFunctionSignature CodeFunctionSignature::none;

bool CodeFunctionSignature::isValid() const
{
    return !name.isNone();
}

CodeBool::CodeBool(bool value) : value(value) {}

CodeChar::CodeChar(char value) : value(value) {}

const CodeParameter CodeParameter::none = Name();

CodeParameter::CodeParameter(Name symbol)
    : type(Symbol_t), _symbol(symbol)
{
}

CodeParameter::CodeParameter(Decimal number)
    : type(Number_t), _number(number)
{
}

CodeParameter::CodeParameter(CodeBool boolean)
    : type(Bool_t), _boolean(boolean.value)
{
}

CodeParameter::CodeParameter(CodeChar character)
    : type(Char_t), _character(character.value)
{
}

CodeParameter::CodeParameter(const char* string)
    : type(String_t)
{
    const size_t len = strlen(string) + 1;
    _string = new char[len];
    strcpy_s(_string, len, string);
}

CodeParameter::CodeParameter(const std::string& string)
    : type(String_t)
{
    const size_t len = string.size() + 1;
    _string = new char[len];
    strcpy_s(_string, len, string.c_str());
}

CodeParameter::CodeParameter(const CodeFunctionInvocation& invocation)
    : type(Expression_t), _expression(new CodeFunctionInvocation(invocation))
{
    this->_expression->isExpression = true;
}

CodeParameter::CodeParameter(const CodeParameter& param) : type(Symbol_t)
{
    assign(param);
}

CodeParameter::CodeParameter(CodeParameter&& param)
{
    assign(param);
}

CodeParameter& CodeParameter::assign(const CodeParameter& param)
{
    switch (type) {
    case String_t: delete _string; _string = NULL; break;
    case Expression_t: delete _expression; _expression = NULL; break;
    }
    
    type = param.type;
    switch (type) {
    case Symbol_t: _symbol = param._symbol; break;
    case Number_t: _number = param._number; break;
    case Bool_t: _boolean = param._boolean; break;
    case Char_t: _character = param._character; break;
    case String_t:
        {
            const size_t len = strlen(param._string) + 1;
            _string = new char[len];
            strcpy_s(_string, len, param._string);
            break;
        }
    case Expression_t:
        _expression = new CodeFunctionInvocation(*param._expression);
    }
    return *this;
}

CodeParameter& CodeParameter::assign(CodeParameter&& param)
{
    switch (type) {
    case String_t: delete _string; _string = NULL; break;
    case Expression_t: delete _expression; _expression = NULL; break;
    }
    
    type = param.type;
    switch (type) {
    case Symbol_t: _symbol = param._symbol; break;
    case Number_t: _number = param._number; break;
    case Bool_t: _boolean = param._boolean; break;
    case Char_t: _character = param._character; break;
    case String_t: _string = param._string; param._string = NULL; break;
    case Expression_t: _expression = param._expression; param._expression = NULL; break;
    }
    return *this;
}

CodeParameter& CodeParameter::operator=(const CodeParameter& param)
{
    return assign(param);
}

CodeParameter& CodeParameter::operator=(CodeParameter&& param)
{
    return assign(param);
}

CodeParameter::~CodeParameter()
{
    if (type == String_t)
        delete _string;
}

bool CodeParameter::isType(Type type) const
{
    return this->type == type;
}

bool CodeParameter::isValid() const
{
    return !(type == Symbol_t && _symbol.isNone());
}

Name& CodeParameter::symbol()
{
    assert(type == Symbol_t);
    return _symbol;
}

const Name& CodeParameter::symbol() const
{
    assert(type == Symbol_t);
    return _symbol;
}

Decimal& CodeParameter::number()
{
    assert(type == Number_t);
    return _number;
}

const Decimal& CodeParameter::number() const
{
    assert(type == Number_t);
    return _number;
}

bool& CodeParameter::boolean()
{
    assert(type == Bool_t);
    return _boolean;
}

const bool& CodeParameter::boolean() const
{
    assert(type == Bool_t);
    return _boolean;
}

char& CodeParameter::character()
{
    assert(type == Char_t);
    return _character;
}

const char& CodeParameter::character() const
{
    assert(type == Char_t);
    return _character;
}

char*& CodeParameter::string()
{
    assert(type == String_t);
    return _string;
}

char* const& CodeParameter::string() const
{
    assert(type == String_t);
    return _string;
}

CodeFunctionInvocation& CodeParameter::expression()
{
    assert(type == Expression_t);
    return *_expression;
}

const CodeFunctionInvocation& CodeParameter::expression() const
{
    assert(type == Expression_t);
    return *_expression;
}

std::string CodeParameter::toString(ICodeScopeBackend& backend) const
{
    switch (type) {
    case Symbol_t: return backend.convertKeyword(_symbol).c_str();
    case Number_t: return _number.toString();
    case Bool_t: return backend.convertKeyword(_boolean ? Code::true_t : Code::false_t).c_str();
    case Char_t: return backend.convertCharacter(_character);
    case String_t: return backend.convertString(_string);
    case Expression_t: return backend.convertExpression(*_expression);
    }
    return std::string();
}

std::ostream& CodeParameter::write(std::ostream& os, ICodeScopeBackend& backend) const
{
    switch (type) {
    case Symbol_t: os << backend.convertKeyword(_symbol).c_str(); break;
    case Number_t: os << _number; break;
    case Bool_t: os << backend.convertKeyword(_boolean ? Code::true_t : Code::false_t).c_str(); break;
    case Char_t: os << backend.convertCharacter(_character); break;
    case String_t: os << backend.convertString(_string); break;
    case Expression_t: os << toString(backend);
    }
    return os;
}

CodeFunctionInvocation::CodeFunctionInvocation(const CodeParameter& operation, bool isExpression)
    : operation(operation), isExpression(isExpression)
{
}

CodeFunctionNameAttribute::CodeFunctionNameAttribute(Name funcName)
    : Attribute("CodeFunctionName", false)
    , funcName(funcName)
{
}

void CodeFunctionNameAttribute::resolve(const Attribute& conflict)
{
    throw runtime_error("CodeFunctionNameAttribute cannot resolve");
}
