#include "ShaderGraph.h"

#include "ShaderCodeGeneration.h"
#include "../../Attributes/TagAttribute.h"
#include "../../IVendor.h"
#include "../../Graph/GraphCodeGeneration.h"
#include "../../CodeGeneration/ClangGeneration.h"
#include "../../Utility/EngineUtility.h"

SerializeInstance(ShaderGraph, DEF_ATTR(Tag, "Shader"));

ShaderGraph::ShaderGraph()
{
    core.name = name;
    flag = Flag::Expression;
    static const Name BxDFName = "BxDF";
    bxdfPin = castTo<ShaderStructPin>(GraphPinFactory::get().construct(BxDFName, BxDFName.c_str()));
    if (bxdfPin == NULL)
        throw runtime_error("BxDF ShaderStructPin not found");
    addReturn(bxdfPin);
    addReturnNode();
}

void ShaderGraph::setPath(const Name& path)
{
    this->path = path;
}

const Name& ShaderGraph::getPath() const
{
    return path;
}

void ShaderGraph::setLocalSize(const Vector3u& size)
{
    localSize = size;
}

const Vector3u& ShaderGraph::getLocalSize() const
{
    return localSize;
}

bool ShaderGraph::generateShaderParameters(GraphCodeGenerationContext& context)
{
    ICodeScopeBackend& backend = context.getBackend();
    backend.write(
        "#include \"/lib/GBuffer_Def.hmat\"\n"
        "DEF_MAT_BEGIN\n");
    vector<GraphVariable*> textureVariables;
    for (auto variable : variables) {
        const Name type = variable->getVariableType();
        if (type == ShaderCode::Texture2D_t || type == ShaderCode::TextureCube_t) {
            textureVariables.emplace_back(variable);
        }
        variable->generate(context);
    }
    auto declareTextures = [&] ()
    {
        for (int index = 0; index < textureVariables.size(); index++) {
            backend.invoke(CodeFunctionInvocation("DEF_TEX2D"_N).param(Name(variables[index]->getName()), CodeParameter(index)));
        }
    };
    backend.write("DEF_MAT_END\n");
    backend.write("#if defined(DEFERRED_SHADER_FEATURE)\n");
    declareTextures();
    backend.write("#elif defined(LIGHTING_SHADER_FEATURE)\n");
    backend.write("DEF_GBUFFER\n");
    backend.invoke(CodeFunctionInvocation("DEF_TEX2D"_N).param("depthMap"_N, CodeParameter(6)));
    backend.write("#else\n");
    declareTextures();
    backend.invoke(CodeFunctionInvocation("DEF_TEX2D"_N).param("depthMap"_N, CodeParameter(textureVariables.size())));
    backend.write("#endif\n");
    return true;
}

bool ShaderGraph::generateGlobalVariables(GraphCodeGenerationContext& context)
{
    return true;
}

bool ShaderGraph::addNode(GraphNode* node)
{
    if (dynamic_cast<EntryNode*>(node))
        return false;
    if (dynamic_cast<ReturnNode*>(node))
        return false;
    return Graph::addNode(node);
}

bool ShaderGraph::removeNode(GraphNode* node)
{
    if (node == returnNodes.front())
        return false;
    return Graph::removeNode(node);
}

void ShaderGraph::setName(const Name& name)
{
    Graph::setName(name);
    core.name = name;
}

Name ShaderGraph::getName() const
{
    return core.name;
}

void ShaderGraph::setBaseMaterial(Material* baseMaterial)
{
    core.baseMaterial = baseMaterial;
}

Material* ShaderGraph::getBaseMaterial() const
{
    return core.baseMaterial;
}

void ShaderGraph::setRenderOrder(int renderOrder)
{
    core.renderOrder = renderOrder;
}

int ShaderGraph::getRenderOrder() const
{
    return core.renderOrder;
}

bool ShaderGraph::isNull() const
{
    return core.isNull();
}

bool ShaderGraph::isValid() const
{
    return core.isValid();
}

bool ShaderGraph::isComputable() const
{
    return core.isComputable();
}

bool ShaderGraph::setMeshStageAdapter(ShaderAdapter & adapter)
{
    return core.setMeshStageAdapter(adapter);
}

ShaderAdapter * ShaderGraph::getMeshStageAdapter()
{
    return core.getMeshStageAdapter();
}

bool ShaderGraph::addShaderAdapter(ShaderAdapter & adapter)
{
    return core.addShaderAdapter(adapter);
}

ShaderAdapter * ShaderGraph::getShaderAdapter(ShaderStageType stageType)
{
    return core.getShaderAdapter(stageType);
}

ShaderProgram * ShaderGraph::getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule)
{
    return core.getProgram(feature, rule);
}

Serializable* ShaderGraph::instantiate(const SerializationInfo& from)
{
    return new ShaderGraph();
}

void ShaderGraph::generateCode(ICodeWriter& writer, ShaderGraph& graph)
{
    writer.write(
        "#vertex use default\n"
        "#fragment\n"
        "#condition depth\n"
        "#condition depth.vsm\n"
        "#condition deferred\n"
        "#condition lighting\n"
        "#condition lighting.vsm\n"
        "#define SHADER_GRAPH_GENERATION\n");
    
    ClangScopeBackend backend(writer);
    GraphCodeGenerationContext context;
    context.pushSubscopeBackend(&backend);
    graph.generateShaderParameters(context);
    ICodeWriter* headWriter = writer.subscope(ICodeWriter::BlankScope);
    graph.generateStatement(context);

    for (const auto& file : context.includeFiles) {
        string path = getGoodRelativePath(file.c_str());
        static const string shaderRootPath = "Engine/Shaders";
        if (path.find(shaderRootPath) == 0) {
            path = path.substr(shaderRootPath.size());
            if (path.front() != '/') {
                path = '/' + path;
            }
        }
        else {
            throw runtime_error("Invalid shader include path");
        }
        headWriter->write("#include \"%s\"\n", path.c_str());
    }

    backend.write(
        "#include \"/lib/Surface_Def.hmat\"\n"
        "FS_DEF_OUT_BEGIN\n"
        "FS_DEF_OUT_SURFACE\n"
        "FS_DEF_OUT_END\n"
        "FragmentOut SURFACE_MAIN\n"
        );

    ICodeScopeBackend& mainScope = *backend.subscope();
    const Name finName = "fin";
    const Name foutName = "fout";
    const Name contextName = "context";
    const Name surfaceName = "surf";
    const Name bxdfName = "bxdf";
    mainScope.declareVariable(CodeSymbolDefinition("FragmentOut", foutName));
    mainScope.declareVariable(CodeSymbolDefinition("BxDFContext", contextName));
    mainScope.declareVariable(CodeSymbolDefinition("BxDF", bxdfName));
    mainScope.invoke(CodeFunctionInvocation("initBxDF"_N).param(bxdfName));
    
    mainScope.invoke(CodeFunctionInvocation(Code::assign_op).param(
        CodeFunctionInvocation(Code::access_op).param(contextName, surfaceName),
        CodeFunctionInvocation("getSurface"_N).param(finName)));
    mainScope.invoke(CodeFunctionInvocation(Code::assign_op).param(bxdfName,
        CodeFunctionInvocation(graph.getName()).param(contextName)));
    mainScope.write("#ifdef DEPTH_SHADER_FEATURE\n");
    mainScope.invoke(CodeFunctionInvocation("outputDepth"_N).param(finName,
        CodeFunctionInvocation(Code::access_op).param(bxdfName, "opacity"_N), CodeParameter(0.5f)));
    mainScope.write("#else\n");
    mainScope.write("#ifdef DEFERRED_SHADER_FEATURE\n");
    mainScope.invoke(CodeFunctionInvocation("outputSurface"_N).param(
        CodeFunctionInvocation(Code::access_op).param(contextName, surfaceName),
        CodeFunctionInvocation("GBUFFER_OUT_PARAMS"_N).param(foutName)));
    mainScope.write("#else\n");
    mainScope.invoke(CodeFunctionInvocation(Code::assign_op).param(
        CodeFunctionInvocation(Code::access_op).param(foutName, "FragColor"_N),
        CodeFunctionInvocation("resolveBxDF"_N).param(bxdfName)));
    mainScope.write("#endif\n");
    
    mainScope.write("#endif\n");

    mainScope.output({ foutName }, false);
}

void ShaderGraph::generateSignature(GraphCodeGenerationContext& context, CodeFunctionSignature& signature)
{
    signature.param(CodeSymbolDefinition("BxDFContext", "context"_N, CQF_Ref));
}
