#include "SkeletonMeshRenderEditor.h"

RegistEditor(SkeletonMeshRender);

void SkeletonMeshRenderEditor::setInspectedObject(void* object)
{
	skeletonMeshRender = dynamic_cast<SkeletonMeshRender*>((Render*)object);
	MeshRenderEditor::setInspectedObject(skeletonMeshRender);
}

void SkeletonMeshRenderEditor::onRendererGUI(EditorInfo& info)
{
	MeshRenderEditor::onRendererGUI(info);
}
