#pragma once

#include "ActorEditor.h"
#include "../Bone.h"

class BoneEditor : public ActorEditor
{
public:
	enum ShowBoneFlag
	{
		Hide = 0, Selected = 1, Always = 2
	};

	BoneEditor() = default;
	virtual ~BoneEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onActorGUI(EditorInfo& info);
	virtual void onBoneGUI(EditorInfo& info);

	virtual void onPersistentGizmo(GizmoInfo& info);
protected:
	Bone* bone = NULL;
	static float boneRadius;
	static ShowBoneFlag showBoneFlag;
};