#pragma once
#include "Node.h"
#include "Variable.h"

class ENGINE_API GraphCodeGenerationContext
{
public:
    struct BackendInfo
    {
        ICodeScopeBackend* backend;
        GraphNode* popNode;
    };
    unordered_map<const GraphPin*, CodeParameter> pinToParameterMap;
    unordered_map<const GraphVariable*, CodeParameter> variableToParameterMap;
    stack<BackendInfo> backendStack;
    stack<GraphNode*> nodeStack;
    unordered_set<GraphNode*> generatedNodes;
    CodeFunctionInvocation invocation;
    CodeFunctionSignature signature;

    GraphCodeGenerationContext();
    virtual ~GraphCodeGenerationContext() = default;

    virtual ICodeScopeBackend& getBackend();
    virtual CodeFunctionInvocation& getInvocation();
    virtual CodeFunctionSignature& getSignature();
    virtual void assignParameter(const GraphPin* pin, const CodeParameter& param);
    virtual void assignParameter(const GraphVariable* variable, const CodeParameter& param);
    virtual const CodeParameter& getParameter(const GraphPin* pin);
    virtual const CodeParameter& getParameter(const GraphVariable* variable);
    virtual void pushSubscopeBackend(ICodeScopeBackend* backend, GraphNode* popNode);
    virtual void pushSubscopeBackend(ICodeScopeBackend* backend);
    virtual void popSubscopeBackend(ICodeScopeBackend* checkBackend = NULL);
    virtual void generateCode();
    virtual void generateCodeFromNode(GraphNode* node);
};
