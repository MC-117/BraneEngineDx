#include "GraphBaseEditor.h"

void GraphBaseEditor::onInspectGUI(EditorInfo& info)
{
}

void GraphBaseEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}

void GraphBaseEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}

void GraphBaseEditor::onGUI(EditorInfo& info)
{
	if (base == NULL)
		return;
	BaseEditor::onGUI(info);
	if (ImGui::CollapsingHeader("Graph"))
		onInspectGUI(info);
}

GraphInfo::~GraphInfo()
{
	if (context) {
		ax::NodeEditor::DestroyEditor(context);
		context = NULL;
	}
	if (builder) {
		delete builder;
		builder = NULL;
	}
}

bool GraphInfo::isTargetGraph(Graph* graph) const
{
	return graphList.empty() ? false : (graphList.back() == graph);
}

void GraphInfo::tickClear()
{
	links.clear();
}

void GraphInfo::resetContext()
{
	if (context) {
		ax::NodeEditor::DestroyEditor(context);
	}
	context = ax::NodeEditor::CreateEditor();
	if (builder) {
		delete builder;
	}
	builder = new ax::NodeEditor::Utilities::BlueprintNodeBuilder;
}

void GraphInfo::setGraph(Graph* graph)
{
	graphList.clear();
	if (graph)
		graphList.push_back(graph);
	resetContext();
}

void GraphInfo::openGraph(Graph* graph)
{
	for (auto b = graphList.begin(), e = graphList.end(); b != e; b++) {
		if ((*b) == graph) {
			graphList.erase(b, graphList.end());
		}
	}
	graphList.push_back(graph);
}
