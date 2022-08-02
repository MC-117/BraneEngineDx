#include "Live2DActorEditor.h"
#include "../Core/GUI/TextureViewer.h"

RegistEditor(Live2DActor);

void Live2DActorEditor::setInspectedObject(void* object)
{
	live2DActor = dynamic_cast<Live2DActor*>((Object*)object);
	ActorEditor::setInspectedObject(live2DActor);
}

Texture* Live2DActorEditor::getPreviewTexture(const Vector2f& desiredSize)
{
	if (live2DActor == NULL)
		return NULL;
	if (live2DActor->live2DRender.is3DMode())
		return NULL;
	live2DActor->live2DRender.setRefSize(desiredSize.x());
	return live2DActor->live2DRender.getOutputTexture();
}

void Live2DActorEditor::onActorGUI(EditorInfo& info)
{
	ActorEditor::onActorGUI(info);
	auto motions = live2DActor->getMotions();
	if (ImGui::CollapsingHeader("Parameters")) {
		for (int i = 0; i < Live2DParameter::ParamCount; i++) {
			Live2DParameter::Type type = (Live2DParameter::Type)i;
			if (live2DActor->hasParameter(type)) {
				Range<float> range = live2DActor->getParameterRange(type);
				float value = live2DActor->getParameter(type);
				if (ImGui::SliderFloat(Live2DParameter::defaultParameterNames[type],
						&value, range.minVal, range.maxVal)) {
					live2DActor->setParameter(type, value);
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Motions")) {
		int id = 0;
		for (auto b = motions.begin(), e = motions.end(); b != e; b++, id++) {
			ImGui::PushID(id);
			if (ImGui::Button(b->first.c_str(), { -1, 36 })) {
				live2DActor->playMotion(b->first);
			}
			ImGui::PopID();
		}
	}
}

void Live2DActorEditor::onRenderersGUI(EditorInfo& info)
{
	if (live2DActor->getModel() == NULL) {
		ImGui::Text("No Live2D Model");
	}
	else {
		string modelPath = MeshAssetInfo::getPath(live2DActor->getModel());
		ImGui::Text(modelPath.c_str());
	}

	Asset* assignAsset = EditorManager::getSelectedAsset();
	if (assignAsset != NULL && assignAsset->assetInfo.type == "Live2DModel") {
		if (ImGui::Button("Set Selected Live2DModel", { -1, 40 })) {
			live2DActor->setModel((Live2DModel*)assignAsset->load());
		}
	}

	bool is3DMode = live2DActor->live2DRender.is3DMode();
	if (ImGui::Checkbox("Is3DMode", &is3DMode))
		live2DActor->live2DRender.set3DMode(is3DMode);

	bool premulAlpha = live2DActor->live2DRender.isPremultipliedAlpha();
	if (ImGui::Checkbox("PremultipliedAlpha", &premulAlpha))
		live2DActor->live2DRender.setPremultipliedAlpha(premulAlpha);

	Texture2D* outputTexture = live2DActor->live2DRender.getOutputTexture();

	if (outputTexture != NULL) {
		float width = ImGui::GetWindowContentRegionWidth();
		unsigned long long id = outputTexture->getTextureID();
		float height = width * outputTexture->getHeight() / outputTexture->getWidth();
		if (ImGui::ImageButton((ImTextureID)id, { width, height })) {
			TextureViewer::showTexture(*info.gui, *outputTexture);
		}
	}
}
