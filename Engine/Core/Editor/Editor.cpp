#include "Editor.h"

SerializeInstance(Editor);

void Editor::onGUI(EditorInfo& info)
{
}

void Editor::onGizmo(GizmoInfo& info)
{
}

Texture* Editor::getPreviewTexture(const Vector2f& desiredSize)
{
	return false;
}

Serializable* Editor::instantiate(const SerializationInfo& from)
{
	return nullptr;
}

Delegate<void(Base*)> EditorManager::onSelectionChanged;
StaticVar<unordered_map<string, const EditorType*>> EditorManager::editorFactories;
StaticVar<unordered_map<void*, Ref<Editor>>> EditorManager::editors;
Ref<Base> EditorManager::selectedObject;
Asset* EditorManager::selectedAsset;

void EditorManager::regist(const EditorType& info)
{
	(*editorFactories)[info.getName()] = &info;
}

Editor* EditorManager::findEditor(void* object)
{
	auto editorIter = editors->find(object);
	if (editorIter != editors->end()) {
		return editorIter->second;
	}
	return NULL;
}

Editor* EditorManager::getEditor(const string& className, void* object)
{
	auto editorIter = editors->find(object);
	if (editorIter != editors->end()) {
		return editorIter->second;
	}
	auto factoryIter = editorFactories->find(className);
	if (factoryIter != editorFactories->end()) {
		Editor* editor = factoryIter->second->createEditor();
		editor->setInspectedObject(object);
		Ref<Editor> editorRef;
		editorRef = editor;
		editors->insert(make_pair(object, editorRef));
		return editor;
	}
	return NULL;
}

Editor* EditorManager::getEditor(Serializable& serialzable)
{
	if (&serialzable == NULL)
		return NULL;
	const Serialization* serialization = &serialzable.getSerialization();
	Editor* editor = NULL;
	while (editor == NULL && serialization != NULL) {
		editor = getEditor(serialization->type, &serialzable);
		serialization = serialization->getBaseSerialization();
	}
	return editor;
}

bool EditorManager::releaseEditor(void* object)
{
	auto editorIter = editors->find(object);
	if (editorIter != editors->end()) {
		editors->erase(editorIter);
		delete editorIter->second;
		return true;
	}
	return false;
}

void EditorManager::selectBase(Base* base)
{
	if (base != NULL && !Base::CheckValid(base))
		return;
	if (base != selectedObject) {
		selectedObject = base;
		onSelectionChanged(base);
	}
}

Base* EditorManager::getSelectedBase()
{
	return selectedObject;
}

void EditorManager::selectObject(Object* object)
{
	selectBase(object);
}

Object* EditorManager::getSelectedObject()
{
	return dynamic_cast<Object*>((Base*)selectedObject);
}

void EditorManager::selectAsset(Asset* asset)
{
	selectedAsset = asset;
}

Asset* EditorManager::getSelectedAsset()
{
	return selectedAsset;
}