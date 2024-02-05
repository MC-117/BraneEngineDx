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

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	Graph* graph = NULL;
	Graph* openingSubGraph = NULL;
	string createNodeFilter;
	ImVec2 popupCurserPos;
	Serialization* createPinType = NULL;
	Serialization* createVariableType = NULL;
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
};
