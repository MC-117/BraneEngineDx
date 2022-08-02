#include "TimelinePlayableEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(TimelinePlayable);

void TimelinePlayableEditor::setInspectedObject(void* object)
{
    playable = dynamic_cast<TimelinePlayable*>((Serializable*)object);
}

void TimelinePlayableEditor::onPlayableGUI(EditorInfo& info)
{
}

void TimelinePlayableEditor::onGUI(EditorInfo& info)
{
    if (playable == NULL)
        return;
    ImGui::Header(playable->getSerialization().type.c_str(), { 0, 0 }, 3);
    onPlayableGUI(info);
}
