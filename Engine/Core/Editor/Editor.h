#pragma once

#include "../Asset.h"
#include "../GUI/GUI.h"
#include "../GUI/Gizmo.h"
#include "../IWorld.h"

struct EditorInfo
{
	GUI* gui;
	Gizmo* gizmo;
	Camera* camera;
	IWorld* world;
	bool destroy;
};

class EditorType;
class Editor : public Base
{
public:
	Serialize(Editor, Base);
	Editor() = default;
	virtual ~Editor() = default;

	virtual void setInspectedObject(void* object) = 0;
	virtual void onGUI(EditorInfo& info);
	virtual void onGizmo(GizmoInfo& info);
	virtual Texture* getPreviewTexture(const Vector2f& desiredSize);

	virtual EditorType& getEditorInfo() const = 0;

	static Serializable* instantiate(const SerializationInfo& from);
};

class EditorType
{
public:
	virtual string getName() const = 0;
	virtual Editor* createEditor() const = 0;
};

class EditorManager
{
public:
	static Delegate<void(Base*)> onSelectionChanged;
	static StaticVar<unordered_map<string, const EditorType*>> editorFactories;
	static StaticVar<unordered_map<void*, Ref<Editor>>> editors;

	static void regist(const EditorType& info);
	static Editor* findEditor(void* object);
	static Editor* getEditor(const string& className, void* object);
	static Editor* getEditor(Serializable& serialzable);
	static bool releaseEditor(void* object);

	static void selectBase(Base* base);
	static Base* getSelectedBase();
	static void selectObject(Object* object);
	static Object* getSelectedObject();
	static void selectAsset(Asset* asset);
	static Asset* getSelectedAsset();
protected:
	static Ref<Base> selectedObject;
	static Asset* selectedAsset;
};

#define EditorInfoMethod() virtual EditorType& getEditorInfo() const;

#define RegistEditor(Class) \
class Class##EditorType : public EditorType \
{ \
	friend class Class##Editor; \
public: \
	virtual string getName() const \
	{ \
		return #Class;\
	} \
	virtual Editor* createEditor() const \
	{ \
		return new Class##Editor();\
	} \
protected: \
	static Class##EditorType instance; \
	Class##EditorType() \
	{ \
		EditorManager::regist(*this); \
	} \
}; \
Class##EditorType Class##EditorType::instance; \
EditorType& Class##Editor::getEditorInfo() const \
{ \
	return Class##EditorType::instance; \
}