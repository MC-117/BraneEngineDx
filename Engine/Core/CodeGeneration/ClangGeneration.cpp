﻿#include "ClangGeneration.h"
#include "Core/Console.h"

ClangWriter::ClangWriter()
{
}

ClangWriter::~ClangWriter()
{
    for (auto& scope : scopes)
        delete scope.scopeWriter;
}

Name ClangWriter::convertKeyword(const Name& keyword)
{
    return keyword;
}

const Name x_op = ".x";
const Name y_op = ".y";
const Name z_op = ".z";
const Name w_op = ".w";

int ClangWriter::getOperatorParamNum(const Name& op)
{
    if (op == x_op || op == y_op || op == z_op || op == w_op)
        return 1;
    return Code::getCodeOperatorNum(op);
}

const char* ClangWriter::getOperatorFormatter(const Name& op)
{
    static const unordered_map<Name, const char*> formatterMap = {
        { Code::assign_op, "%s = %s" },
        { Code::access_op, "%s.%s" },
        { Code::add_op, "%s + %s" },
        { Code::sub_op, "%s - %s" },
        { Code::mul_op, "%s * %s" },
        { Code::div_op, "%s / %s" },
        { Code::mod_op, "%s % %s" },
        { Code::lShift_op, "%s >> %s" },
        { Code::rShift_op, "%s << %s" },
        { Code::gre_op, "%s > %s" },
        { Code::les_op, "%s < %s" },
        { Code::eq_op, "%s == %s" },
        { Code::notEq_op, "%s != %s" },
        { Code::greEq_op, "%s >= %s" },
        { Code::lesEq_op, "%s <= %s" },
        { Code::and_op, "%s & %s" },
        { Code::or_op, "%s | %s" },
        { Code::xor_op, "%s ^ %s" },
        { Code::minus_op, "-%s" },
        { Code::not_op, "!%s" },
        { x_op, "%s.x()" },
        { y_op, "%s.y()" },
        { z_op, "%s.z()" },
        { w_op, "%s.w()" },
        { Code::cond_op, "%s ? %s : %s" },
    };

    auto iter = formatterMap.find(op);
    if (iter != formatterMap.end())
        return iter->second;
    return NULL;
}

void ClangWriter::write(const char* fmt_str, ...)
{
    va_list ap;
    va_start(ap, fmt_str);
    write(fmt_str, ap);
    va_end(ap);
}

void ClangWriter::write(const char* fmt_str, va_list ap)
{
    writeIndent();
    int len = strlen(fmt_str);
    int final_n, n = len * 2; /* Reserve two times as much as the length of the fmt_str */
    std::unique_ptr<char[]> formatted;
    while (1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        final_n = vsnprintf(&formatted[0], n, fmt_str, ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    //formatted[final_n + 1] = '\0';
    output() << formatted.get();
}

ICodeWriter* ClangWriter::subscope(ScopeType type)
{
    ClangWriter* scopeWriter = newWriter();
    if (type == BlankScope) {
        scopes.emplace_back().scopeWriter = scopeWriter;
    }
    else {
        writeIndent();
        output() << "{\n";
        scopes.emplace_back().scopeWriter = scopeWriter;
        scopeWriter->indent = indent + 1;
        writeIndent();
        output() << "}\n";
    }
    return scopeWriter;
}

void ClangWriter::beginExpression(const char* fmt_str, ...)
{
    if (fmt_str) {
        va_list ap;
        va_start(ap, fmt_str);
        write(fmt_str, ap);
        va_end(ap);
    }
    if (expressionCount > 0) {
        output() << ", ";
        expressionCountStack.push(expressionCount);
        expressionCount = 0;
    }
    output() << '(';
    indent++;
}

void ClangWriter::endExpression(const char* ender)
{
    if (expressionCountStack.empty()) {
        expressionCount = 0;
    }
    else {
        expressionCount = expressionCountStack.top();
        expressionCountStack.pop();
    }
    indent--;
    writeIndent();
    output() << ")";
    if (ender)
        output() << ender;
}

void ClangWriter::writeSymbolDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier)
{
    writeIndent();
    Enum<CodeQualifierFlags> qualifier = definition.qualifiers | extraQualifier;
    if (qualifier.has(CQF_Static)) {
        output() << "static ";
    }
    if (qualifier.has(CQF_Const)) {
        output() << "const ";
    }
    output() << convertKeyword(definition.type).c_str();
    if (qualifier.enumValue & CQF_Out) {
        output() << '&';
    }
    output() << ' ' << definition.name.c_str();
}

void ClangWriter::writeParameterDefinition(const CodeSymbolDefinition& definition, Enum<CodeQualifierFlags> extraQualifier)
{
    writeIndent();
    if (expressionCount > 0)
        output() << ", ";
    writeSymbolDefinition(definition, extraQualifier);
    expressionCount++;
}

void ClangWriter::writeParameter(const CodeParameter& param, ICodeScopeBackend& backend)
{
    if (expressionCount > 0)
        output() << ", ";
    param.write(output(), backend);
    expressionCount++;
}

void ClangWriter::getString(std::string& str)
{
    if (scopes.empty())
        str = firstbuffer.str();
    else {
        std::stringstream buffer;
        writeTo(buffer);
        str = buffer.str();
    }
}

void ClangWriter::writeTo(std::ostream& os)
{
    os << firstbuffer.str();
    for (auto& scope : scopes) {
        assert(scope.scopeWriter);
        scope.scopeWriter->writeTo(os);
        os << scope.buffer.str();
    }
}

std::stringstream& ClangWriter::output()
{
    if (scopes.empty())
        return firstbuffer;
    return scopes.back().buffer;
}

void ClangWriter::writeIndent()
{
    char lastChar;
    std::stringstream& stream = output();
    const size_t pos = stream.tellp();
    if (pos > 0) {
        stream.seekg(-1, std::ios::end);
        stream.read(&lastChar, 1);
        if (lastChar != '\n')
            return;
    }
    for (int i = 0; i < indent; i++)
        stream << "    ";
}

ClangWriter* ClangWriter::newWriter()
{
    return new ClangWriter();
}

Name ClangScopeBackend::_builtin_function_type_ = "_builtin_function_type_";

ClangScopeBackend::ClangScopeBackend(ICodeWriter& writer, const CodeFunctionSignature& signature)
    : writer(writer), scopeSignature(signature)
{
}

ClangScopeBackend::~ClangScopeBackend()
{
    for (auto& scope : subscopes)
        delete scope;
}

Name ClangScopeBackend::newTempVariableName()
{
    return newSymbolName("tmpVar");
}

Name ClangScopeBackend::newTempFunctionName()
{
    return newSymbolName("tmpFunc");
}

Name ClangScopeBackend::convertKeyword(const Name& keyword)
{
    return writer.convertKeyword(keyword);
}

std::string ClangScopeBackend::convertString(const char* str)
{
    return std::string("\"") + str + "\"";
}

std::string ClangScopeBackend::convertCharacter(char character)
{
    return std::string("'") + character + "'";
}

std::string ClangScopeBackend::convertExpression(const CodeFunctionInvocation& invocation)
{
    ClangWriter writer;
    ClangScopeBackend backend(writer);
    backend.invoke(invocation);
    std::string result;
    writer.getString(result);
    return result;
}

void ClangScopeBackend::write(const char* fmt_str, ...)
{
    va_list ap;
    va_start(ap, fmt_str);
    writer.write(fmt_str, ap);
    va_end(ap);
}

ICodeScopeBackend* ClangScopeBackend::subscope()
{
    return subscope(scopeSignature);
}

bool ClangScopeBackend::declareVariable(const CodeSymbolDefinition& definition, const CodeParameter& defaultValue)
{
    if (!registerSymbol(definition))
        return false;
    writer.writeSymbolDefinition(definition);
    if (defaultValue.isValid())
        writer.write(" = %s;\n", defaultValue.toString(*this).c_str());
    else
        writer.write(";\n");
    return true;
}

ICodeScopeBackend* ClangScopeBackend::declareFunction(const CodeFunctionSignature& signature)
{
    if (!registerFunction(signature))
        return NULL;

    const bool inFunction = scopeSignature.isValid();
    const bool onlyOneOutput = signature.outputs.size() == 1;

    if (inFunction) {
        writer.beginExpression("auto %s = [&] ", signature.name.c_str());
    }
    else if (onlyOneOutput) {
        writer.beginExpression("%s %s", signature.outputs[0].type.c_str(), signature.name.c_str());
    }
    else {
        writer.beginExpression("void %s", signature.name.c_str());
    }
    
    for (auto& symbol : signature.parameters) {
        writer.writeParameterDefinition(symbol, CQF_In);
    }

    if (signature.outputs.size() > 1) {
        for (auto& symbol : signature.outputs) {
            writer.writeParameterDefinition(symbol, CQF_Out);
        }
    }
    
    writer.endExpression();

    if (inFunction && onlyOneOutput) {
        writer.write(" -> %s\n", signature.outputs[0].type.c_str());
    }
    else {
        writer.write("\n");
    }

    ClangScopeBackend* funcScope = (ClangScopeBackend*)subscope(signature);

    if (funcScope) {
        for (auto& symbol : signature.parameters) {
            funcScope->registerSymbol(symbol);
        }
        
        if (signature.outputs.size() > 1) {
            for (auto& symbol : signature.outputs) {
                funcScope->registerSymbol(symbol);
            }
        }
    }
    
    return funcScope;
}

bool ClangScopeBackend::invoke(const CodeFunctionInvocation& invocation)
{
    bool ok = true;
    int opNum = 0;
    if (invocation.operation.isType(CodeParameter::Symbol_t))
        opNum = writer.getOperatorParamNum(invocation.operation.symbol());
    if (opNum == 0) {
        if (invocation.outputs.size() == 1) {
            ok &= checkSymbol(invocation.outputs[0]);
            writer.write("%s = ", invocation.outputs[0].c_str());
        }
        writer.beginExpression(invocation.operation.toString(*this).c_str());
        for (auto& param : invocation.parameters) {
            ok &= checkParameter(param);
            writer.writeParameter(param, *this);
        }
        if (invocation.outputs.size() > 1) {
            for (auto& param : invocation.outputs) {
                ok &= checkSymbol(param);
                writer.writeParameter(param, *this);
            }
        }
        writer.endExpression(invocation.isExpression ? NULL : ";\n");
    }
    else {
        if (opNum != invocation.parameters.size()) {
            Console::error("Inconsistent parameter count");
            return false;
        }
        for (auto& output : invocation.outputs) {
            writer.write("%s = ", output.c_str());
        }
        const char* formatter = writer.getOperatorFormatter(invocation.operation.symbol().c_str());
        switch (opNum) {
        case 1:
            writer.write(formatter,
                invocation.parameters[0].toString(*this).c_str());
            break;
        case 2:
            writer.write(formatter,
                invocation.parameters[0].toString(*this).c_str(),
                invocation.parameters[1].toString(*this).c_str());
            break;
        case 3:
            writer.write(formatter,
                invocation.parameters[0].toString(*this).c_str(),
                invocation.parameters[1].toString(*this).c_str(),
                invocation.parameters[2].toString(*this).c_str());
            break;
        default:
            assert(0);
        }
        if (!invocation.isExpression)
            writer.write(";\n");
    }
    return ok;
}

bool ClangScopeBackend::branch(const std::vector<CodeParameter>& conditionParams, std::vector<ICodeScopeBackend*>& scopes)
{
    if (conditionParams.empty()) {
        Console::error("Branch not has condition variable");
        return false;
    }
    bool ok = true;
    writer.beginExpression("if ");
    ok &= checkParameter(conditionParams[0]);
    writer.writeParameter(conditionParams[0], *this);
    writer.endExpression("\n");
    scopes.push_back(subscope());
    for (int i = 1; i < conditionParams.size(); i++) {
        writer.beginExpression("else if ");
        ok &= checkParameter(conditionParams[i]);
        writer.writeParameter(conditionParams[i], *this);
        writer.endExpression("\n");
        scopes.push_back(subscope());
    }
    writer.write("else\n");
    scopes.push_back(subscope());
    return ok;
}

ICodeScopeBackend* ClangScopeBackend::loop(const CodeParameter& conditionParam)
{
    writer.beginExpression("while ");
    checkParameter(conditionParam);
    writer.writeParameter(conditionParam, *this);
    writer.endExpression("\n");
    return subscope();
}

bool ClangScopeBackend::jumpOut()
{
    writer.write("break;\n");
    return true;
}

bool ClangScopeBackend::output(const std::vector<CodeParameter>& returnValues, bool doCheck)
{
    if (doCheck) {
        if (!scopeSignature.isValid())
            return false;
        if (scopeSignature.outputs.size() != returnValues.size())
            return false;
    }
    const int outCount = returnValues.size();
    bool ok = true;
    switch (outCount) {
    case 0:
        writer.write("return;\n");
        break;
    case 1:
        writer.write("return %s;\n", returnValues[0].toString(*this).c_str());
        break;
    default:
        for (int i = 0; i < outCount; i++) {
            ok &= invoke(CodeFunctionInvocation(Code::assign_op)
                .param(scopeSignature.outputs[i].name, returnValues[i]));
        }
        break;
    }
    return ok;
}

Name ClangScopeBackend::newSymbolName(const char* name)
{
    int num = 0;
    while (true) {
        Name tmpName = std::string(name) + "_" + std::to_string(num);
        if (symbolTable.find(tmpName) == symbolTable.end()) {
            return tmpName;
        }
        num++;
    }
}

bool ClangScopeBackend::registerSymbol(const CodeSymbolDefinition& symbol)
{
    if (symbol.name.isNone()) {
        Console::error("Symbol is not valid");
        return false;
    }
    if (symbolTable.find(symbol.name) == symbolTable.end()) {
        symbolTable.insert({ symbol.name, symbol });
        return true;
    }
    Console::error("Symbol redefine: %s %s", symbol.type.c_str(), symbol.name.c_str());
    return false;
}

bool ClangScopeBackend::registerFunction(const CodeFunctionSignature& signature)
{
    if (!registerSymbol({ _builtin_function_type_, signature.name, }))
        return false;
    funcitonTable.insert({ signature.name, signature });
    return true;
}

bool ClangScopeBackend::checkSymbol(const Name& name)
{
    if (name.isNone()) {
        Console::error("Symbol is not valid");
        return false;
    }
    if (symbolTable.find(name) != symbolTable.end())
        return true;
    Console::error("Symbol not found: %s", name.c_str());
    return false;
}

bool ClangScopeBackend::checkParameter(const CodeParameter& param)
{
    return param.isValid();
}

ICodeScopeBackend* ClangScopeBackend::subscope(const CodeFunctionSignature& signature)
{
    ClangScopeBackend* subscope = new ClangScopeBackend(*writer.subscope(ICodeWriter::FunctionScope), signature);
    subscope->symbolTable = symbolTable;
    subscope->funcitonTable = funcitonTable;
    subscopes.push_back(subscope);
    return subscope;
}
