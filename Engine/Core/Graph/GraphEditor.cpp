#include "GraphEditor.h"

#include "GraphCodeGeneration.h"
#include "../GUI/GUIUtility.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../Engine.h"
#include "../Utility/EngineUtility.h"
#include "../CodeGeneration/ClangGeneration.h"
#include "../ShaderCode/ShaderGraph/HLSLGeneration.h"
#include "../ShaderCode/ShaderGraph/HLSLGeneration.h"
#include "../Attributes/TagAttribute.h"

RegistEditor(Graph);

GraphEditor::~GraphEditor()
{
	delete tempScript;
}

void GraphEditor::setInspectedObject(void* object)
{
	graph = dynamic_cast<Graph*>((Base*)object);
	GraphNodeEditor::setInspectedObject(graph);
}

void GraphEditor::onInspectGUI(EditorInfo& info)
{
	if (graph == NULL)
		return;
	if (showFlags.has(Show_Details)) {
		onDetailsGUI(info);
	}
	if (showFlags.has(Show_CreateParameters)) {
		onCreateParametersGUI(info);
	}
	if (showFlags.has(Show_CreateReturns)) {
		onCreateReturnsGUI(info);
	}
	if (showFlags.has(Show_CreateVariables)) {
		onCreateVariablesGUI(info);
	}
	if (showFlags.has(Show_CreateSubgraphs)) {
		onCreateSubgraphGUI(info);
	}
	if (showFlags.has(Show_Execution)) {
		onExecutionGUI(info);
	}
	if (showFlags.has(Show_CreateSubgraphs)) {
		onGenerationGUI(info);
	}
	onCustomGUI(info);
}

void GraphEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (graphInfo.isTargetGraph(graph)) {
		onGraphAction(info, graphInfo);
		onGraphCanvasGUI(info, graphInfo);
		if (openingSubGraph) {
			graphInfo.openGraph(openingSubGraph);
			openingSubGraph = NULL;
		}
	}
	else {
		onGraphNodeGUI(info, graphInfo);
	}
}

void GraphEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (graphInfo.isTargetGraph(graph)) {
		onCanvasContextMenuGUI(info, graphInfo);
	}
	else {
		onNodeContextMenuGUI(info, graphInfo);
	}
}

void GraphEditor::copyNodes(const vector<ax::NodeEditor::NodeId>& nodeIDs, SerializationInfo& info)
{
	info.type = "CopyGraphNode";
	int i = 0;
	for each (auto nodeID in nodeIDs)
	{
		GraphNode* node = getGraphNode(nodeID);
		ReturnNode* returnNode = dynamic_cast<ReturnNode*>(node);
		EntryNode* entryNode = dynamic_cast<EntryNode*>(node);
		if (node == NULL || returnNode || entryNode)
			continue;
		SerializationInfo* nodeInfo = info.add(to_string(i).c_str());
		if (nodeInfo)
			node->serialize(*nodeInfo);
		i++;
	}
}

void GraphEditor::pasteNodes(SerializationInfo& info)
{
	namespace ne = ax::NodeEditor;
	if (graph == NULL || info.type != "CopyGraphNode")
		return;
	map<Guid, Guid> guidMap;
	newSerializationInfoGuid(info, guidMap);
	replaceSerializationInfoGuid(info, guidMap);
	set<Guid> guidSet;
	for each (const auto & item in guidMap)
	{
		guidSet.insert(item.second);
	}
	ne::ClearSelection();
	for (auto b = info.sublists.begin(), e = info.sublists.end(); b != e; b++) {
		if (b->serialization &&
			(b->serialization == &GraphNode::GraphNodeSerialization::serialization ||
				b->serialization->isChildOf(GraphNode::GraphNodeSerialization::serialization))) {
			Serializable* serializable = b->serialization->instantiate(*b);
			GraphNode* node = dynamic_cast<GraphNode*>(serializable);
			if (node) {
				node->deserialize(*b);
				for (int i = 0; i < node->getInputCount(); i++) {
					GraphPin* pin = node->getInput(i);
					Ref<GraphPin>& ref = pin->getConnectedPinRef();
					if (!ref.guid.isDefault() && guidSet.find(ref.guid) == guidSet.end()) {
						ref = NULL;
					}
				}
				for (int i = 0; i < node->getOutputCount(); i++) {
					GraphPin* pin = node->getOutput(i);
					Ref<GraphPin>& ref = pin->getConnectedPinRef();
					if (!ref.guid.isDefault() && guidSet.find(ref.guid) == guidSet.end()) {
						ref = NULL;
					}
				}
				graph->addNode(node);
				ne::SelectNode(node->getInstanceID(), true);
			}
			else
				delete serializable;
		}
	}
}

void GraphEditor::getPinMenuTags(vector<Name>& tags)
{
}

void GraphEditor::getVariableMenuTags(vector<Name>& tags)
{
}

void GraphEditor::getNodeMenuTags(vector<Name>& tags)
{
	TagManager::get().fuzzyFindTag("Graph", tags);
}

GraphPin* GraphEditor::onPinCreateGUI(const char* buttonName, string& valueName, bool nameValid)
{
	ImGui::PushID(buttonName);
	ImGui::SetNextItemWidth(100);
	vector<Name> tags;
	getPinMenuTags(tags);
	ImGui::TypeCombo("##PinTypeCombo", createPinType, GraphPin::GraphPinSerialization::serialization, tags);
	ImGui::SameLine();
	ImGui::InputText("##ValueNodeName", &valueName);
	ImGui::BeginDisabled(!nameValid);
	if (ImGui::ButtonEx(buttonName, { -1, 36 })) {
		if (createPinType != NULL && !valueName.empty()) {
			SerializationInfo info;
			info.name = valueName;
			Serializable* serialiable = createPinType->instantiate(info);
			if (serialiable == NULL)
				return NULL;
			GraphPin* pin = dynamic_cast<GraphPin*>(serialiable);
			if (pin == NULL)
				delete serialiable;
			else
				pin->setName(valueName);
			return pin;
		}
	}
	ImGui::EndDisabled();
	ImGui::PopID();
	return NULL;
}

GraphVariable* GraphEditor::onVariableCreateGUI(const char* buttonName)
{
	ImGui::PushID(buttonName);
	ImGui::SetNextItemWidth(100);
	vector<Name> tags;
	getVariableMenuTags(tags);
	ImGui::TypeCombo("##VariableTypeCombo", createVariableType, GraphVariable::GraphVariableSerialization::serialization, tags);
	ImGui::SameLine();
	ImGui::InputText("##VariableName", &createVariableName);
	bool nameValid = graph->checkVariableName(createVariableName);
	ImGui::BeginDisabled(!nameValid);
	if (ImGui::ButtonEx(buttonName, { -1, 36 })) {
		if (createVariableType != NULL && !createVariableName.empty()) {
			SerializationInfo info;
			info.name = createVariableName;
			Serializable* serialiable = createVariableType->instantiate(info);
			if (serialiable == NULL)
				return NULL;
			GraphVariable* variable = dynamic_cast<GraphVariable*>(serialiable);
			if (variable == NULL)
				delete serialiable;
			return variable;
		}
	}
	ImGui::EndDisabled();
	ImGui::PopID();
	return NULL;
}

Graph* GraphEditor::onSubGraphCreateGUI(const char* buttonName)
{
	ImGui::PushID(buttonName);
	ImGui::InputText("SubGraphName", &createSubGraphName);
	bool nameValid = graph->checkSubGraphName(createSubGraphName);
	ImGui::BeginDisabled(!nameValid);
	if (ImGui::ButtonEx(buttonName, { -1, 36 })) {
		Graph* subgraph = new Graph();
		subgraph->setName(createSubGraphName);
		return subgraph;
	}
	ImGui::EndDisabled();
	ImGui::PopID();
	return nullptr;
}

void GraphEditor::onCreateLink(GraphPin* fromPin, GraphPin* toPin)
{
	fromPin->connect(toPin);
}

void GraphEditor::onExtraContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}

GraphNode* GraphEditor::onGraphMemberGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	GraphNode* node = NULL;

	ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

	if (ImGui::TreeNode("Variable")) {
		for (int i = 0; i < graph->getVariableCount(); i++)
		{
			GraphVariable* variable = graph->getVariable(i);
			if (!createNodeFilter.empty() && variable->getName().find(createNodeFilter) == string::npos)
				continue;
			if (ImGui::MenuItem(("Get " + variable->getName()).c_str())) {
				VariableNode* getNode = new VariableNode();
				getNode->init(variable);
				node = getNode;
			}
			if (ImGui::MenuItem(("Set " + variable->getName()).c_str())) {
				SetVariableNode* setNode = new SetVariableNode();
				setNode->init(variable);
				node = setNode;
			}
		}
		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

	if (ImGui::TreeNode("SubGraph")) {
		for (int i = 0; i < graph->getSubGraphCount(); i++)
		{
			Graph* subgraph = graph->getSubGraph(i);
			string subgraphName = subgraph->getName().c_str();
			if (!createNodeFilter.empty() && subgraphName.find(createNodeFilter) == string::npos)
				continue;
			if (ImGui::MenuItem(subgraph->getName().c_str())) {
				GraphProxy* proxy = new GraphProxy();
				proxy->setName(subgraph->getDisplayName());
				proxy->setEntryNode(subgraph);
				proxy->setReturnNode(subgraph);
				node = proxy;
			}
		}
		ImGui::TreePop();
	}
	return node;
}

void GraphEditor::onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo)
{
	graphInfo.openGraph(graph);
}

void GraphEditor::onGraphCanvasGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	if (graph != NULL)
		for (int i = 0; i < graph->getNodeCount(); i++) {
			GraphNode* node = graph->getNode(i);
			if (node == NULL)
				continue;
			GraphNodeEditor* nodeEditor = dynamic_cast<GraphNodeEditor*>(EditorManager::getEditor(*node));
			if (nodeEditor) {
				ne::SetNodePosition(node->getInstanceID(), (ImVec2&)nodeEditor->getNodePosition());
			}
		}

	ne::Begin("Canvas");

	if (graph != NULL) {

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 3 });

		auto cursorTopLeft = ImGui::GetCursorScreenPos();

		for (int i = 0; i < graph->getNodeCount(); i++) {
			GraphNode* node = graph->getNode(i);
			if (node == NULL)
				continue;
			GraphNodeEditor* nodeEditor = dynamic_cast<GraphNodeEditor*>(EditorManager::getEditor(*node));
			if (nodeEditor)
				nodeEditor->onGraphGUI(info, graphInfo);
		}

		for each (const auto & link in graphInfo.links)
		{
			ne::LinkId linkID = getLinkID(link.first, link.second);
			ne::Link(linkID, link.first, link.second);
			GraphPin* fromPin = getGraphPin(link.first);
			GraphPin* toPin = getGraphPin(link.second);
			if (fromPin && toPin) {
				GraphPinEditor* fromEditor = dynamic_cast<GraphPinEditor*>(EditorManager::getEditor(*fromPin));
				GraphPinEditor* toEditor = dynamic_cast<GraphPinEditor*>(EditorManager::getEditor(*toPin));
				if (fromEditor && toEditor) {
					LinkInfo linkInfo = { linkID, ne::GetLinkColor(linkID), fromEditor, toEditor, fromPin, toPin };
					fromEditor->onLinkGUI(info, graphInfo, linkInfo);
				}
			}
		}
		graphInfo.links.clear();

		if (!createNewNode) {
			if (ne::BeginCreate(ImColor(255, 255, 255))) {
				ne::PinId fromId = 0, toId = 0;
				if (ne::QueryNewLink(&fromId, &toId)) {
					graphInfo.newLinkPinID = fromId ? fromId : toId;
					GraphPin* fromPin = getGraphPin(fromId);
					GraphPin* toPin = getGraphPin(toId);
					if (fromPin && toPin) {
						if (!fromPin->isOutputPin())
							swap(fromPin, toPin);
						if (fromPin->isConnectable(toPin)) {
							if (ne::AcceptNewItem(ImColor(128, 255, 128), 3.0f))
							{
								onCreateLink(fromPin, toPin);
							}
						}
						else
							ne::RejectNewItem(ImColor(255, 0, 0));
					}
				}

				ne::PinId pinId = 0;
				if (ne::QueryNewNode(&pinId))
				{
					graphInfo.newLinkPinID = pinId;
					if (ne::AcceptNewItem())
					{
						createNewNode = true;
						graphInfo.newLinkPinID = 0;
						ne::Suspend();
						ImGui::OpenPopup("CreateNodeMenu");
						ne::Resume();
					}
				}

				ne::EndCreate();
			}
			else
				graphInfo.newLinkPinID = 0;

			if (ne::BeginDelete())
			{
				ne::LinkId linkId = 0;
				while (ne::QueryDeletedLink(&linkId))
				{
					if (ne::AcceptDeletedItem())
					{
						ne::PinId fromId = 0, toId = 0;
						getLinkedPinID(linkId, fromId, toId);
						GraphPin* fromPin = getGraphPin(fromId);
						GraphPin* toPin = getGraphPin(toId);
						if (fromPin && toPin) {
							toPin->disconnect(fromPin);
						}
					}
				}

				ne::NodeId nodeId = 0;
				while (ne::QueryDeletedNode(&nodeId))
				{
					if (ne::AcceptDeletedItem())
					{
						GraphNode* node = getGraphNode(nodeId);
						graph->removeNode(node);
					}
				}
				ne::EndDelete();
			}

			ImGui::SetCursorScreenPos(cursorTopLeft);
		}

		ImGui::PopStyleVar();

		ne::Suspend();

		popupCurserPos = ne::ScreenToCanvas(ImGui::GetMousePos());

		if (ne::ShowNodeContextMenu(&graphInfo.contextNodeID))
			ImGui::OpenPopup("NodeContextMenu");
		else if (ne::ShowPinContextMenu(&graphInfo.contextPinID))
			ImGui::OpenPopup("PinContextMenu");
		else if (ne::ShowBackgroundContextMenu())
		{
			ImGui::OpenPopup("CreateNodeMenu");
		}

		GraphNode* node = getGraphNode(graphInfo.contextNodeID);
		GraphPin* pin = getGraphPin(graphInfo.contextPinID);
		if (node) {
			GraphNodeEditor* editor = dynamic_cast<GraphNodeEditor*>(EditorManager::getEditor(*node));
			if (editor && ImGui::BeginPopup("NodeContextMenu"))
			{
				editor->onContextMenuGUI(info, graphInfo);
				ImGui::EndPopup();
			}
		}

		if (pin) {
			GraphPinEditor* editor = dynamic_cast<GraphPinEditor*>(EditorManager::getEditor(*pin));
			if (editor && ImGui::BeginPopup("PinContextMenu"))
			{
				editor->onContextMenuGUI(info, graphInfo);
				ImGui::EndPopup();
			}
		}

		if (ImGui::BeginPopup("CreateNodeMenu")) {
			onContextMenuGUI(info, graphInfo);
			ImGui::EndPopup();
		}
		else
			createNewNode = false;

		onExtraContextMenuGUI(info, graphInfo);

		ne::Resume();
	}
	else {
		ne::Suspend();
		if (ne::ShowBackgroundContextMenu())
		{
			ImGui::OpenPopup("CreateGraphMenu");
		}
		if (ImGui::BeginPopup("CreateGraphMenu")) {
			if (ImGui::MenuItem("Create Graph")) {
				graph = new Graph();
			}
			ImGui::EndPopup();
		}
		ne::Resume();
	}

	ne::End();

	if (graph != NULL)
		for (int i = 0; i < graph->getNodeCount(); i++) {
			GraphNode* node = graph->getNode(i);
			if (node == NULL)
				continue;
			GraphNodeEditor* nodeEditor = dynamic_cast<GraphNodeEditor*>(EditorManager::getEditor(*node));
			if (nodeEditor) {
				nodeEditor->setNodePosition((Vector2f&)ne::GetNodePosition(node->getInstanceID()));
			}
		}
}

void GraphEditor::onGraphNodeGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	GraphNodeEditor::onGraphGUI(info, graphInfo);
}

void GraphEditor::onCanvasContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (graph == NULL)
		return;
	ImGui::InputText("Filter", &createNodeFilter);

	ImGui::BeginChild("NodeTypes", { -1, 300 });

	vector<Serialization*> nodeTypes;
	GraphNode::GraphNodeSerialization::serialization.getChildren(nodeTypes);

	GraphNode* node = onGraphMemberGUI(info, graphInfo);

	if (ImGui::TreeNode("Scope")) {
		int i = 0;
		for each (auto pGraph in graphInfo.graphList)
		{
			if (pGraph == graph)
				continue;
			GraphEditor* graphEditor = dynamic_cast<GraphEditor*>(EditorManager::getEditor(*pGraph));
			if (graphEditor) {
				ImGui::PushID(i);
				ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
				if (ImGui::TreeNode((pGraph->getDisplayName() + " Members").c_str())) {
					GraphNode* newNode = graphEditor->onGraphMemberGUI(info, graphInfo);
					if (newNode)
						node = newNode;
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			i++;
		}
		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

	auto graphNodeItems = [&](const char* tag)
	{
		TagManager::Iter iter = TagManager::get().findByTag(tag, &GraphNode::GraphNodeSerialization::serialization);
		while (iter.next()) {
			Serialization& type = iter.get();
			if (!createNodeFilter.empty() && type.type.find(createNodeFilter) == string::npos)
				continue;
			if (ImGui::MenuItem(type.type.c_str())) {
				if (&type != &EntryNode::EntryNodeSerialization::serialization) {
					SerializationInfo serInfo;
					Serializable* serializable = type.instantiate(serInfo);
					if (serializable) {
						node = dynamic_cast<GraphNode*>(serializable);
						if (!node)
							delete serializable;
					}
				}
			}
		}
	};

	vector<Name> tags;
	getNodeMenuTags(tags);
	vector<string> lastTagItems;
	stack<pair<Name, bool>> tagStack;
	for (auto& tag : tags) {
		vector<string> curTagItems = split(tag.c_str(), '.');
		const int lastLength = lastTagItems.size();
		const int curLength = curTagItems.size();
		int index = 0;
		while (index < lastLength && index < curLength
			&& lastTagItems[index] == curTagItems[index]) {
			index++;
		}
		for (int i = lastLength; i > index; i--) {
			const pair<Name, bool>& topTagItem = tagStack.top();
			if (topTagItem.second) {
				graphNodeItems(topTagItem.first.c_str());
				ImGui::TreePop();
			}
			tagStack.pop();
		}
		for (; index < curLength; index++) {
			const char* curTagCStr = curTagItems[index].c_str();
			bool treeOpen = true;
			if (!tagStack.empty()) {
				treeOpen = tagStack.top().second;
			}
			if (treeOpen) {
				treeOpen = ImGui::TreeNode(curTagCStr);
			}
			tagStack.emplace(tag, treeOpen);
		}
		lastTagItems = std::move(curTagItems);
	}
	for (int i = 0; i < lastTagItems.size(); i++) {
		const pair<Name, bool>& tagTreeItem = tagStack.top();
		if (tagTreeItem.second) {
			graphNodeItems(tagTreeItem.first.c_str());
			ImGui::TreePop();
		}
		tagStack.pop();
	}

	if (node) {
		if (graph->addNode(node)) {
			createNewNode = false;
			ax::NodeEditor::SetNodePosition(node->getInstanceID(), popupCurserPos);
		}
		else {
			delete node;
			node = NULL;
		}
	}

	ImGui::EndChild();
}

void GraphEditor::onNodeContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	GraphNodeEditor::onContextMenuGUI(info, graphInfo);
	if (ImGui::Button("Open")) {
		graphInfo.openGraph(graph);
	}
}

void GraphEditor::onGraphAction(EditorInfo& info, GraphInfo& graphInfo)
{
	namespace ne = ax::NodeEditor;
	if (Engine::getInput().getKeyDown(VK_CONTROL)) {
		if (Engine::getInput().getKeyPress('C')) {
			int count = ne::GetSelectedObjectCount();
			vector<ax::NodeEditor::NodeId> nodeIDs;
			nodeIDs.resize(count);
			count = ne::GetSelectedNodes(nodeIDs.data(), count);
			nodeIDs.resize(count);
			if (count > 0) {
				SerializationInfo copyInfo;
				copyNodes(nodeIDs, copyInfo);
				stringstream stream;
				SerializationInfoWriter writer(stream);
				writer.write(copyInfo);
				ImGui::SetClipboardText(stream.str().c_str());
			}
		}
		else if (Engine::getInput().getKeyPress('V')) {
			stringstream stream(ImGui::GetClipboardText());
			SerializationInfoParser parser(stream);
			if (parser.parse()) {
				pasteNodes(parser.infos[0]);
			}
		}
	}
}

void GraphEditor::onGenerateCode(ScriptBase& script)
{
	string code;
	switch (genCodeType) {
	case Clang:
		{
			ClangWriter writer;
			ClangScopeBackend backend(writer);
			GraphCodeGenerationContext context;
			context.pushSubscopeBackend(&backend);
			graph->generateStatement(context);
			writer.getString(code);
			break;
		}
	case HLSL:
		{
			HLSLWriter writer;
			ClangScopeBackend backend(writer);
			GraphCodeGenerationContext context;
			context.pushSubscopeBackend(&backend);
			graph->generateStatement(context);
			writer.getString(code);
			break;
		}
	}

	tempScript->setSourceCode(code);
}

void GraphEditor::onDetailsGUI(EditorInfo& info)
{
	string graphName = graph->getName().c_str();
	if (ImGui::InputText("Name", &graphName)) {
		graph->setName(graphName);
	}
	const char* flagStr[2] = { "Statment", "Expression" };
	int flag = (unsigned char)graph->getFlag();
	if (ImGui::BeginCombo("GraphType", flagStr[flag])) {
		for (int i = 0; i < 2; i++) {
			if (ImGui::Selectable(flagStr[i], flag == i))
				graph->setFlag((Graph::Flag)i);
		}
		ImGui::EndCombo();
	}
}

void GraphEditor::onCreateParametersGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Parameters")) {
		GraphPin* pin = onPinCreateGUI("AddParameter", createParameterName,
			graph->checkParameterName(createParameterName));
		if (pin) {
			graph->setEntryNode();
			graph->addParameter(pin);
			ax::NodeEditor::CenterNodeOnScreen(graph->getEntryNode()->getInstanceID());
		}

		for (int i = 0; i < graph->getInputCount(); i++)
		{
			ImGui::PushID(i);
			GraphPin* pin = graph->getInput(i);
			GraphPinEditor* editor = dynamic_cast<GraphPinEditor*>(EditorManager::getEditor(*pin));
			if (editor) {
				if (ImGui::BeginHeaderBox(pin->getDisplayName().c_str())) {
					editor->onInspectGUI(info);
				}
				else {
					graph->removeParameter(pin);
				}
				ImGui::EndHeaderBox();
			}
			ImGui::PopID();
		}
	}
}

void GraphEditor::onCreateReturnsGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Returns")) {
		GraphPin* pin = onPinCreateGUI("AddReturn", createReturnName,
			graph->checkReturnName(createReturnName));
		if (pin) {
			graph->addReturn(pin);
		}

		for (int i = 0; i < graph->getOutputCount(); i++)
		{
			ImGui::PushID(i);
			GraphPin* pin = graph->getOutput(i);
			GraphPinEditor* editor = dynamic_cast<GraphPinEditor*>(EditorManager::getEditor(*pin));
			if (editor) {
				if (ImGui::BeginHeaderBox(pin->getDisplayName().c_str())) {
					editor->onInspectGUI(info);
				}
				else {
					graph->removeParameter(pin);
				}
				ImGui::EndHeaderBox();
			}
			ImGui::PopID();
		}
	}
}

void GraphEditor::onCreateVariablesGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Variables")) {
		GraphVariable* variable = onVariableCreateGUI("AddVariable");
		if (variable) {
			graph->addVariable(variable);
		}

		for (int i = 0; i < graph->getVariableCount(); i++)
		{
			ImGui::PushID(i);
			GraphVariable* variable = graph->getVariable(i);
			GraphBaseEditor* editor = dynamic_cast<GraphBaseEditor*>(EditorManager::getEditor(*variable));
			if (editor) {
				if (ImGui::BeginHeaderBox(variable->getName().c_str())) {
					editor->onInspectGUI(info);
				}
				else {
					graph->removeVariable(variable);
				}
				ImGui::EndHeaderBox();
			}
			ImGui::PopID();
		}
	}
}

void GraphEditor::onCreateSubgraphGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("SubGraphes")) {
		Graph* subgraph = onSubGraphCreateGUI("AddSubGraph");
		if (subgraph) {
			graph->addSubGraph(subgraph);
		}

		for (int i = 0; i < graph->getSubGraphCount(); i++)
		{
			ImGui::PushID(i);
			Graph* subgraph = graph->getSubGraph(i);
			if (ImGui::Button(("Open " + subgraph->getName()).c_str(), { -52, 36 })) {
				openingSubGraph = subgraph;
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete", { 50, 36 })) {
				graph->removeSubGraph(subgraph);
			}
			ImGui::PopID();
		}
	}
}

void GraphEditor::onExecutionGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Execution")) {
		if (ImGui::Button("Run")) {
			GraphContext context;
			graph->solveState(context);
		}
	}
}

void GraphEditor::onGenerationGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Generation")) {
		static const char* genCodeTypeNames[] = {
			"C", "HLSL"
		};
		if (ImGui::BeginCombo("CodeType", genCodeTypeNames[genCodeType])) {
			for (int i = 0; i < CodeTypeNum; i++) {
				if (ImGui::Selectable(genCodeTypeNames[i], genCodeType == i)) {
					genCodeType = (GenCodeType)i;
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("GenerateCode")) {
			static const char* genCodeExts[] = {
				"cpp", "mat"
			};
			
			if (tempScript == NULL)
				tempScript = new TempScript(graph->getName() + "_GenCode." + genCodeExts[genCodeType]);
			
			onGenerateCode(*tempScript);
			
			ScriptWindow::OpenScript(*tempScript);
		}
	}
}

void GraphEditor::onCustomGUI(EditorInfo& info)
{
}
