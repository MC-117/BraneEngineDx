#pragma once

#include "../BaseEditor.h"
#include "../../RenderCore/RenderInterface.h"

class RenderPassEditor : public Editor
{
public:
	RenderPassEditor() = default;
	virtual ~RenderPassEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onRenderPassGUI(EditorInfo & info);

	virtual void onGUI(EditorInfo & info);
protected:
	RenderPass* renderPass = NULL;
};