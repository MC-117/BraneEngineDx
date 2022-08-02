#include "RenderEditor.h"

RegistEditor(Render);

void RenderEditor::setInspectedObject(void* object)
{
	render = (Render*)object;
}

void RenderEditor::onRendererGUI(EditorInfo& info)
{
	bool can = render->canCastShadow;
	if (ImGui::Checkbox("CastShadow", &can)) {
		render->canCastShadow = can;
	}
}

void RenderEditor::onShapeGUI(EditorInfo& info)
{
	Shape* shp = render->getShape();
	if (shp == NULL)
		return;
	ImGui::Text("Bound: (%.4f, %.4f, %.4f)-(%.4f, %.4f, %.4f)",
		shp->bound.minVal[0],
		shp->bound.minVal[1],
		shp->bound.minVal[2],
		shp->bound.maxVal[0],
		shp->bound.maxVal[1],
		shp->bound.maxVal[2]);
	Vector3f center = shp->getCenter();
	ImGui::Text("Center: (%.4f, %.4f, %.4f)",
		center.x(),
		center.y(),
		center.z());
	ImGui::Text("Width: %.4f, Height: %.4f, Depth: %.4f",
		shp->getWidth(),
		shp->getHeight(),
		shp->getDepth());
}

void RenderEditor::onOutlineGUI(EditorInfo& info)
{
}

void RenderEditor::onMaterialsGUI(EditorInfo& info)
{

}

void RenderEditor::onGUI(EditorInfo& info)
{
	if (render == NULL)
		return;
	onRendererGUI(info);
	if (ImGui::CollapsingHeader("Shape"))
		onShapeGUI(info);
	if (ImGui::CollapsingHeader("Outline"))
		onOutlineGUI(info);
	if (ImGui::CollapsingHeader("Materials"))
		onMaterialsGUI(info);
}
