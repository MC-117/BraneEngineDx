#pragma once

#include "Spine2DActor.h"
#include "../Core/Editor/ActorEditor.h"

class Spine2DActorEditor : public ActorEditor
{
public:
	Spine2DActorEditor() = default;
	virtual ~Spine2DActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual Texture* getPreviewTexture(const Vector2f& desiredSize);

	virtual void onActorGUI(EditorInfo& info);
	virtual void onRenderersGUI(EditorInfo& info);
protected:
	Spine2DActor* spine2DActor = NULL;
	int activeTrackIndex = 0;
	bool loop = true;
};