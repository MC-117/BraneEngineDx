#include "GraphWindow.h"
#include "GraphAssetInfo.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"
#include "../Utility/EngineUtility.h"

GraphWindow::GraphWindow(string name, bool defaultShow)
	: UIWindow(*(Object*)NULL, name, defaultShow)
	, previewer(name)
{
	previewer.init(NULL);
}

GraphWindow::~GraphWindow()
{
}

void GraphWindow::setGraph(Graph* graph)
{
	this->graph = graph;
	graphInfo.setGraph(graph);
}

void GraphWindow::onWindowGUI(GUIRenderInfo& info)
{
	float splitterSize = 3;
	ImVec2 contentPos = ImGui::GetWindowContentRegionMin();
	float height = ImGui::GetWindowContentRegionMax().y -
		contentPos.y - ImGui::GetStyle().WindowPadding.y;
	float width = ImGui::GetWindowContentRegionMax().x - contentPos.x;
	float viewWidth = width - splitterSize;

	float inspectViewWidth = viewWidth * inspectViewWidthRadio;
	float graphViewWidth = viewWidth - inspectViewWidth;

	if (ImGui::Splitter(true, splitterSize, &inspectViewWidth, &graphViewWidth, 20, 20)) {
		inspectViewWidthRadio = inspectViewWidth / viewWidth;
	}

	EditorInfo editorInfo;
	editorInfo.gui = &info.gui;
	editorInfo.gizmo = info.gizmo;
	editorInfo.camera = info.camera;
	editorInfo.world = Engine::getCurrentWorld();
	editorInfo.previewer = &previewer;

	Graph* pGraph = graph;

	if (targetGraph != NULL && pGraph == NULL) {
		graphInfo.setGraph(pGraph);
		targetGraph = NULL;
		editor = NULL;
	}

	if (!graphInfo.graphList.empty()) {
		if (targetGraph != graphInfo.graphList.back()) {
			targetGraph = graphInfo.graphList.back();
			graphInfo.resetContext();
			editor = dynamic_cast<GraphEditor*>(EditorManager::getEditor(*targetGraph));
		}
	}

	ImGui::BeginChild("ToolBarTop", ImVec2(inspectViewWidth, -1));

	float buttonHeight = 25;
	ImVec2 buttonSize = { buttonHeight, buttonHeight };
	ImGui::BeginHorizontal("ToolBarHorizontal");

	ImGui::ButtonEx(ICON_FA_FILE, buttonSize);
	if (ImGui::BeginPopupContextItem("CreateGraphContextMenu", ImGuiPopupFlags_MouseButtonLeft)) {
		vector<Serialization*> types = { &Graph::GraphNodeSerialization::serialization };
		Graph::GraphSerialization::serialization.getChildren(types);
		Serialization* selectedType = NULL;
		for (Serialization* type : types) {
			if (ImGui::Selectable(type->type.c_str())) {
				selectedType = type;
			}
		}
		if (selectedType) {
			SerializationInfo graphInfo;
			Serializable* serialiable = selectedType->instantiate(graphInfo);
			if (serialiable) {
				Graph* tempGraph = dynamic_cast<Graph*>(serialiable);
				if (tempGraph) {
					pGraph = tempGraph;
					graph = pGraph;
					setGraph(pGraph);
				}
			}
		}
		ImGui::EndPopup();
	}
	ImGui::BeginDisabled(pGraph == NULL);
	if (ImGui::ButtonEx(ICON_FA_XMARK, buttonSize)) {
		thread td = thread([](Graph* tar) {
			FileDlgDesc desc;
			desc.title = "Save Graph";
			desc.filter = "graph(*.graph)|*.graph";
			desc.initDir = "./Content";
			desc.defFileExt = "graph";
			desc.save = true;
			if (openFileDlg(desc)) {
				if (AssetManager::saveAsset(*tar, desc.filePath)) {
					MessageBox(NULL, "Complete", "Info", MB_OK);
				}
				else {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
					return;
				}
			}
		}, pGraph);
		td.detach();
	}
	ImGui::Spring();
	if (ImGui::ButtonEx(ICON_FA_PLAY, buttonSize)) {
		GraphContext context;
		pGraph->solveState(context);
	}
	ImGui::EndDisabled();
	ImGui::EndHorizontal();
	if (editor)
		editor->onInspectGUI(editorInfo);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("GraphView", ImVec2(graphViewWidth, -1));
	ImGui::BeginHorizontal("GraphBar");
	int i = 0;
	for each (auto g in graphInfo.graphList)
	{
		ImGui::PushID(i);
		string name = i == 0 ? "Self" : g->getDisplayName().c_str();
		if (ImGui::Button(name.c_str())) {
			graphInfo.openGraph(g);
			ImGui::PopID();
			break;
		}
		ImGui::TextUnformatted(">");
		ImGui::PopID();
		i++;
	}
	ImGui::EndHorizontal();
	if (editor) {
		ax::NodeEditor::SetCurrentEditor(graphInfo.context);
		editor->onGraphGUI(editorInfo, graphInfo);
		graphInfo.tickClear();
	}
	ImGui::EndChild();
}

void GraphWindow::onRender(RenderInfo& info)
{
	UIWindow::onRender(info);
	previewer.onRender(info);
}

void GraphWindow::showGraph(GUI& gui, Graph* graph)
{
	GraphWindow* win = dynamic_cast<GraphWindow*>(gui.getUIControl("Graph"));
	if (win == NULL) {
		win = new GraphWindow();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setGraph(graph);
}
