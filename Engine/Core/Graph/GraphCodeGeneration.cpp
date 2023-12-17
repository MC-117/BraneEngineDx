#include "GraphCodeGeneration.h"


GraphCodeGenerationContext::GraphCodeGenerationContext()
{
}

ICodeScopeBackend& GraphCodeGenerationContext::getBackend()
{
    return *backendStack.top().backend;
}

CodeFunctionInvocation& GraphCodeGenerationContext::getInvocation()
{
    return invocation;
}

CodeFunctionSignature& GraphCodeGenerationContext::getSignature()
{
    return signature;
}

void GraphCodeGenerationContext::assignParameter(const GraphPin* pin, const CodeParameter& param)
{
    pinToParameterMap.insert_or_assign(pin, param);
}

void GraphCodeGenerationContext::assignParameter(const GraphVariable* variable, const CodeParameter& param)
{
    variableToParameterMap.insert_or_assign(variable, param);
}

const CodeParameter& GraphCodeGenerationContext::getParameter(const GraphPin* pin)
{
    auto iter = pinToParameterMap.find(pin);
    if (iter != pinToParameterMap.end())
        return iter->second;
    return CodeParameter::none;
}

const CodeParameter& GraphCodeGenerationContext::getParameter(const GraphVariable* variable)
{
    auto iter = variableToParameterMap.find(variable);
    if (iter != variableToParameterMap.end())
        return iter->second;
    return CodeParameter::none;
}

void GraphCodeGenerationContext::pushSubscopeBackend(ICodeScopeBackend* backend, GraphNode* popNode)
{
    backendStack.push({ backend, popNode });
}

void GraphCodeGenerationContext::pushSubscopeBackend(ICodeScopeBackend* backend)
{
    pushSubscopeBackend(backend, nodeStack.empty() ? NULL : nodeStack.top());
}

void GraphCodeGenerationContext::popSubscopeBackend(ICodeScopeBackend* checkBackend)
{
    assert(checkBackend == backendStack.top().backend);
    backendStack.pop();
}

void GraphCodeGenerationContext::generateCode()
{
    while (!nodeStack.empty()) {
        GraphNode* curNode = nodeStack.top();
        nodeStack.pop();
        while (backendStack.top().popNode == curNode)
            backendStack.pop();
        curNode->generateStatement(*this);
    }
}

void GraphCodeGenerationContext::generateCodeFromNode(GraphNode* node)
{
    GraphNode* endNode = nodeStack.empty() ? NULL : nodeStack.top();
    nodeStack.push(node);
    while (!nodeStack.empty()) {
        GraphNode* curNode = nodeStack.top();
        if (endNode == curNode)
            break;
        nodeStack.pop();
        while (backendStack.top().popNode == curNode)
            backendStack.pop();
        curNode->generateStatement(*this);
    }
}
