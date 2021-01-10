#pragma once
#ifndef _EDITORWINDOW_H_
#define _EDITORWINDOW_H_

#include "InspectorWindow.h"

class EditorWindow : public UIWindow
{
public:
	Material& baseMat;
	Object* selectedObj = NULL;
	bool alwaysShow = false;
	Object* parentObj = NULL;
	SerializationInfo copyInfo;

	EditorWindow(Object& object, Material& baseMat, string name = "Editor", bool defaultShow = false);
	virtual void onRenderWindow(GUIRenderInfo& info);
	virtual void onPostAction(GUIPostInfo& info);
protected:
	void traverse(Object& obj, GUI& gui, Object*& dragObj, Object*& targetObj);
	void select(Object* obj, GUI& gui);

	void objectContextMenu(Object* obj);
	void meshCombo();
	void materialCombo();

	string newObjectName;
	Mesh* selectedMesh = NULL;
	int selectedMeshID = 0;
	Material* selectedMaterial = NULL;
	int selectedMaterialID = 0;

	// Character
	float springArmRadius = 2;
	float springArmLength = 50;

	// Character
	float capsuleRadius = 5;
	float capsuleHalfLength = 20;

	// GrasActor
	float grassDensity = 20;
	Vector2f grassBound = { 100, 100 };
};

#endif // !_EDITORWINDOW_H_
