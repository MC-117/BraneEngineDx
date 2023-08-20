#pragma once

#include "Live2DActor.h"
#include "../Core/Editor/ActorEditor.h"

class Live2DActorEditor : public ActorEditor
{
public:
	Live2DActorEditor() = default;
	virtual ~Live2DActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual Texture* getPreviewTexture(const Vector2f& desiredSize);

	virtual void onActorGUI(EditorInfo& info);
	virtual void onRenderersGUI(EditorInfo & info);
protected:
	Live2DActor* live2DActor = NULL;
};