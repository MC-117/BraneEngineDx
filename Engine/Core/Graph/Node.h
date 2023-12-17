#pragma once

#include "../Base.h"
#include <stack>
#include "../CodeGeneration/CodeGenerationInterface.h"

class GraphNode;
class GraphCodeGenerationContext;

class ENGINE_API GraphContext
{
public:
	float deltaTime = 0;
	stack<GraphNode*> nodeStack;
	int loopCount = 0;

	virtual ~GraphContext() = default;

	virtual void execute();
	virtual void executeNode(GraphNode* node);
};

enum struct GraphPinShape : unsigned char
{
	Flow, Circle, Square, Grid, RoundSquare, Diamond
};

class ENGINE_API GraphPin : public Base
{
	friend class GraphNode;
public:
	Serialize(GraphPin, Base);

	string getName() const;
	string getDisplayName() const;
	virtual Color getPinColor() const;
	virtual GraphPinShape getPinShape() const;

	GraphNode* getNode() const;

	bool isOutputPin() const;

	void setName(const string& name);
	void setDisplayName(const string& name);

	virtual bool isConnectable(GraphPin* pin) const;
	virtual GraphPin* getConnectedPin();
	virtual Ref<GraphPin>& getConnectedPinRef();
	virtual bool connect(GraphPin* pin);
	virtual bool disconnect(GraphPin* pin);

	virtual bool process(GraphContext& context);
	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	GraphPin(const string& name);
	string name;
	string displayName;
	GraphNode* node = NULL;
	bool isOutput = false;
	bool autoDelete = false;
	Ref<GraphPin> connectedPin;
};

class ENGINE_API FlowPin : public GraphPin
{
	friend class GraphNode;
public:
	Serialize(FlowPin, GraphPin);

	FlowPin(const string& name);

	virtual Color getPinColor() const;
	virtual GraphPinShape getPinShape() const;

	virtual bool connect(GraphPin* pin);

	virtual bool process(GraphContext& context);
	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
};

class ENGINE_API GraphNode : public Base
{
	friend class GraphProxy;
	friend class Graph;
public:
	Serialize(GraphNode, Base);

	enum struct Flag : unsigned char
	{
		Statment, Expression
	};

	GraphNode();
	virtual ~GraphNode();

	virtual void setName(const string& name);
	string getName() const;
	virtual string getDisplayName() const;
	virtual Color getNodeColor() const;

	int getInputCount() const;
	GraphPin* getInput(int index) const;
	GraphPin* getInput(const string& name) const;

	int getOutputCount() const;
	GraphPin* getOutput(int index) const;
	GraphPin* getOutput(const string& name) const;

	void getRootStates(unordered_set<GraphNode*>& states);

	virtual bool solveInput(GraphContext& context);
	virtual bool flowControl(GraphContext& context);
	virtual bool solveState(GraphContext& context);
	virtual bool process(GraphContext& context);

	virtual Name getFunctionName() const;
	virtual bool generateParameter(GraphCodeGenerationContext& context);
	virtual bool solveAndGenerateOutput(GraphCodeGenerationContext& context);
	virtual bool generateStatement(GraphCodeGenerationContext& context);
	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	string name;
	string displayName;
	Flag flag = Flag::Statment;
	Color nodeColor = { 128, 128, 128 };
	vector<GraphPin*> inputs;
	vector<GraphPin*> outputs;

	void addInternalInput(GraphPin& input);
	void addInternalOutput(GraphPin& output);

	virtual void addInput(GraphPin& input);
	virtual void addOutput(GraphPin& output);

	virtual void clearInputs();
	virtual void clearOutputs();

	void clearPins();
};