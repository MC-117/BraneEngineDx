#pragma once

#include "Graph.h"
#include "GraphNodeEditor.h"
#include "../Script/ScriptWindow.h"

class ENGINE_API GraphEditor : public GraphNodeEditor
{
public:
	GraphEditor() = default;
	virtual ~GraphEditor();

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	enum ShowFlags
	{
		Show_None = 0,
		Show_Details = 1 << 0,
		Show_CreateParameters = 1 << 1,
		Show_CreateReturns = 1 << 2,
		Show_CreateVariables = 1 << 3,
		Show_CreateSubgraphs = 1 << 4,
		Show_Execution = 1 << 5,
		Show_Generation = 1 << 6,
		Show_All = (1 << 7) - 1
	};
	Enum<ShowFlags> showFlags = Show_All;
	Graph* graph = NULL;
	Graph* openingSubGraph = NULL;
	string createNodeFilter;
	ImVec2 popupCurserPos;
	const Serialization* createPinType = NULL;
	const Serialization* createVariableType = NULL;
	string createParameterName;
	string createReturnName;
	string createVariableName;
	string createSubGraphName;

	bool createNewNode = true;

	TempScript* tempScript = NULL;
	enum GenCodeType
	{
		Clang, HLSL, CodeTypeNum
	} genCodeType = Clang;

	void copyNodes(const vector<ax::NodeEditor::NodeId>& nodeIDs, SerializationInfo& info);
	void pasteNodes(SerializationInfo& info);

	virtual void getPinMenuTags(vector<Name>& tags);
	virtual void getVariableMenuTags(vector<Name>& tags);
	virtual void getNodeMenuTags(vector<Name>& tags);

	virtual GraphPin* onPinCreateGUI(const char* buttonName, string& valueName, bool nameValid);
	virtual GraphVariable* onVariableCreateGUI(const char* buttonName);
	virtual Graph* onSubGraphCreateGUI(const char* buttonName);
	
	virtual void onCreateLink(GraphPin* fromPin, GraphPin* toPin);
	virtual void onExtraContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual GraphNode* onGraphMemberGUI(EditorInfo& info, GraphInfo& graphInfo);
	
	virtual void onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo);

	virtual void onGraphCanvasGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onGraphNodeGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onCanvasContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onNodeContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);

	virtual void onGraphAction(EditorInfo& info, GraphInfo& graphInfo);

	virtual void onGenerateCode(ScriptBase& script);

	virtual void onDetailsGUI(EditorInfo& info);
	virtual void onCreateParametersGUI(EditorInfo& info);
	virtual void onCreateReturnsGUI(EditorInfo& info);
	virtual void onCreateVariablesGUI(EditorInfo& info);
	virtual void onCreateSubgraphGUI(EditorInfo& info);
	virtual void onExecutionGUI(EditorInfo& info);
	virtual void onGenerationGUI(EditorInfo& info);
	virtual void onCustomGUI(EditorInfo& info);
};
