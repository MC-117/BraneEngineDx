#pragma once

#include "TimelinePlayableEditor.h"
#include "AudioPlayable.h"

class AudioPlayableEditor : public TimelinePlayableEditor
{
public:
	AudioPlayableEditor() = default;
	virtual ~AudioPlayableEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPlayableGUI(EditorInfo & info);
protected:
	AudioPlayable* audioPlayable = NULL;
};