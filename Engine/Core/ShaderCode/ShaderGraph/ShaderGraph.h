#pragma once
#include "ShaderNode.h"
#include "../../Graph/Graph.h"
#include "../../Shader.h"

class ICodeWriter;

class ShaderGraph : public Graph, public Shader
{
	friend class Material;
public:
	Serialize(ShaderGraph, Graph);

	ShaderGraph();
	virtual ~ShaderGraph() = default;

	void setPath(const Name& path);
	const Name& getPath()const;

	void setLocalSize(const Vector3u& size);
	const Vector3u& getLocalSize()const;

	virtual bool generateShaderParameters(GraphCodeGenerationContext& context);
	
	virtual bool generateGlobalVariables(GraphCodeGenerationContext& context);

	virtual bool addNode(GraphNode* node);

	virtual bool removeNode(GraphNode* node);

	virtual void setName(const Name& name);
	virtual Name getName() const;

	virtual void setBaseMaterial(Material* baseMaterial);
	virtual Material* getBaseMaterial() const;
	
	virtual void setRenderOrder(int renderOrder);
	virtual int getRenderOrder() const;

	virtual bool isNull() const;
	virtual bool isValid() const;
	virtual bool isComputable() const;

	bool setMeshStageAdapter(ShaderAdapter& adapter);
	ShaderAdapter* getMeshStageAdapter();
	virtual bool addShaderAdapter(ShaderAdapter& adapter);
	virtual ShaderAdapter* getShaderAdapter(ShaderStageType stageType);
	virtual ShaderProgram* getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule = ShaderMatchRule());

	static Serializable* instantiate(const SerializationInfo& from);

	static void generateCode(ICodeWriter& writer, ShaderGraph& graph);
protected:
	Name path;
	Vector3u localSize;
	ShaderStructPin* bxdfPin = NULL;
	ShaderCore core;

	virtual void generateSignature(GraphCodeGenerationContext& context, CodeFunctionSignature& signature);
};
