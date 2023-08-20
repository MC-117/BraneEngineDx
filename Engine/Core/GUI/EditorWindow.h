#pragma once
#ifndef _EDITORWINDOW_H_
#define _EDITORWINDOW_H_

#include "InspectorWindow.h"

class Mesh;

class EditorWindow : public UIWindow
{
public:
	Material& baseMat;
	bool alwaysShow = false;
	Object* parentObj = NULL;

	EditorWindow(Object& object, Material& baseMat, string name = "Editor", bool defaultShow = false);
	virtual void onRenderWindow(GUIRenderInfo& info);
	virtual void onPostAction(GUIPostInfo& info);
protected:
	void traverse(Object& obj, GUI& gui, Object*& dragObj, Object*& targetObj);
	void select(Object* obj, GUI& gui);

	void objectContextMenu(Object* obj);
	void meshCombo();
	void materialCombo();

	Object* lastSelectedObject = NULL;
	bool nodeAutoExpand = false;

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

	// GrassActor
	float grassDensity = 20;
	Vector2f grassBound = { 100, 100 };

	// TerrainActor
	float terrainUnit = 500;
	float terrainHeight = 500;
	Vector2u terrainGrid = { 100, 100 };

	// ReflectionProbe
	float reflectionProbeRadius = 10;
};

#endif // !_EDITORWINDOW_H_
