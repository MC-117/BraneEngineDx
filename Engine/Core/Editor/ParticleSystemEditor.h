#pragma once

#include "ActorEditor.h"
#include "../ParticleSystem.h"

class ParticleSystemEditor : public ActorEditor
{
public:
	ParticleSystemEditor() = default;
	virtual ~ParticleSystemEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onActorGUI(EditorInfo& info);
	virtual void onRenderersGUI(EditorInfo& info);
protected:
	ParticleSystem* particleSystem = NULL;
};