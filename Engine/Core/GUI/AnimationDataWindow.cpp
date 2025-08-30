#include "AnimationDataWindow.h"
#include "../Engine.h"
#include "GUIUtility.h"
#include "../WUI/ProgressUI.h"

AnimationDataWindow::AnimationDataWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void AnimationDataWindow::setAnimationData(AnimationClipData* data)
{
	this->data = data;
}

void AnimationDataWindow::onWindowGUI(GUIRenderInfo& info)
{
	float splitterSize = 3;
	ImVec2 contentPos = ImGui::GetWindowContentRegionMin();
	float height = ImGui::GetWindowContentRegionMax().y -
		contentPos.y - ImGui::GetStyle().WindowPadding.y;
	float width = ImGui::GetWindowContentRegionMax().x - contentPos.x;
	float viewWidth = width - splitterSize;

	float inspectViewWidth = viewWidth * inspectViewWidthRadio;
	float plotViewWidth = viewWidth - inspectViewWidth;

	if (ImGui::Splitter(true, splitterSize, &inspectViewWidth, &plotViewWidth, 20, 20)) {
		inspectViewWidthRadio = inspectViewWidth / viewWidth;
	}

	bool applyNewCurveName = false;
	ImGui::BeginChild("InspectView", ImVec2(inspectViewWidth, -1));

	float buttonHeight = 25;
	ImVec2 buttonSize = { buttonHeight, buttonHeight };
	ImGui::BeginHorizontal("ToolBarHorizontal");

	if (ImGui::BeginPopup("##NewAnimationPopup")) {
		ImGui::InputText("Name", &newName);
		if (!newName.empty()) {
			if (ImGui::Button("Create")) {
				AnimationClipData* newData = new AnimationClipData(newName);
				setAnimationData(newData);
			}
		}
	}

	if (ImGui::ButtonEx(ICON_FA_FILE, buttonSize)) {
		ImGui::OpenPopup("##NewAnimationPopup");
	}

	ImGui::BeginDisabled(data == NULL);
	if (ImGui::ButtonEx(ICON_FA_FLOPPY_DISK, buttonSize)) {
		string path = AssetInfo::getPath(data);
		thread td = thread([](AnimationClipData* tar, string path) {
			if (path.empty()) {
				FileDlgDesc desc;
				desc.title = "Save Graph";
				desc.filter = "graph(*.graph)|*.graph";
				desc.initDir = "./Content";
				desc.defFileExt = "graph";
				desc.save = true;
				if (openFileDlg(desc)) {
					path = desc.filePath;
				}
			}
			if (!path.empty()) {
				ProgressUI ui = ProgressUI("Write Animation");
				ui.doModelAsync();
				const bool result = AnimationLoader::writeAnimation(*tar, path, &ui.work);
				ui.close();
				if (!result) {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
				}
			}
		}, data, path);
		td.detach();
	}
	ImGui::EndDisabled();
	ImGui::EndHorizontal();

	if (data) {
		for (auto& item : data->curveNames) {
			bool selected = item.first == selectedCurveName;
			ImGui::PushID(item.first.c_str());
			if (ImGui::Selectable(item.first.c_str(), &selected)) {
				selectedCurveName = item.first;
				selectedCurve = &data->curves[item.second];
			}
			if (ImGui::BeginPopupContextItem("CurveItemPop")) {
				oldCurveName = item.first;
				ImGui::Text("OldName: %s", oldCurveName.c_str());
				ImGui::SameLine();
				if (ImGui::Button("Replace")) {
					newCurveName = oldCurveName;
				}
				ImGui::InputText("NewName", &newCurveName);
				if (!newCurveName.empty() &&
					data->curveNames.find(newCurveName) == data->curveNames.end()) {
					if (ImGui::Button("Apply", { -1, 36 })) {
						applyNewCurveName = true;
					}
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("PlotView", ImVec2(plotViewWidth, -1));
	if (ImPlot::BeginPlot((selectedCurveName + "##Plot").c_str(), { -1, -1 })) {
		ImPlot::SetupAxes("Time", "Value", ImPlotAxisFlags_Opposite);
		if (selectedCurve)
			ImGui::CurveView(selectedCurveName.c_str(), selectedCurve);
		ImPlot::EndPlot();
	}
	ImGui::EndChild();

	if (applyNewCurveName) {
		auto iter = data->curveNames.find(oldCurveName);
		float index = iter->second;
		data->curveNames.erase(iter);
		data->curveNames.insert(make_pair(newCurveName, index));
	}
}

void AnimationDataWindow::showAnimationData(GUI& gui, AnimationClipData* data)
{
	AnimationDataWindow* win = dynamic_cast<AnimationDataWindow*>(gui.getUIControl("AnimationData"));
	if (win == NULL) {
		win = new AnimationDataWindow();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setAnimationData(data);
}
