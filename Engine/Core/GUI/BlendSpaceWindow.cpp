#include <fstream>
#include "BlendSpaceWindow.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../../ThirdParty/ImGui/imgui_internal.h"
#include "../Engine.h"
#include "../Console.h"
#include "../Editor/Editor.h"

ImVec2 to(const Vector2f & v)
{
	return{ v.x(), v.y() };
}

BlendSpaceWindow::BlendSpaceWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void BlendSpaceWindow::setBlendSpaceAnimation(BlendSpaceAnimation & animation)
{
	blendSpace = &animation;
	context.points.resize(blendSpace->animationClipWrap.size());
	for (int i = 0; i < context.points.size(); i++) {
		context.points[i] = to(blendSpace->animationClipWrap[i].first);
	}
}

void BlendSpaceWindow::onWindowGUI(GUIRenderInfo & info)
{
	if (blendSpace == NULL)
		return;
	if (ImGui::Button("LoadBlendSpace")) {
		thread td = thread([](BlendSpaceAnimation* tar) {
			FileDlgDesc desc;
			desc.title = "asset";
			desc.filter = "asset(*.asset)|*.asset";
			desc.initDir = "Content";
			desc.defFileExt = "asset";
			if (openFileDlg(desc)) {
				ifstream f = ifstream(desc.filePath);
				SerializationInfoParser parser = SerializationInfoParser(f);
				if (!parser.parse()) {
					MessageBox(NULL, "Load failed", "Error", MB_OK);
					Console::error("SerializationInfoParser error: %s", parser.parseError.c_str());
				}
				else if (!parser.infos.empty()) {
					if (!tar->deserialize(parser.infos[0])) {
						MessageBox(NULL, "Deserialize failed", "Error", MB_OK);
						Console::error("SerializationInfoParser error: %s", parser.parseError.c_str());
					}
				}
				f.close();
			}
		}, blendSpace);
		td.detach();
	}
	ImGui::SameLine();
	if (ImGui::Button("SaveBlendSpace")) {
		thread td = thread([](BlendSpaceAnimation* tar) {
			FileDlgDesc desc;
			desc.title = "asset";
			desc.filter = "asset(*.asset)|*.asset";
			desc.initDir = "Content";
			desc.save = true;
			desc.defFileExt = "asset";
			if (openFileDlg(desc)) {
				if (AssetManager::saveAsset(*tar, desc.filePath) == NULL) {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
					return;
				}
				else {
					MessageBox(NULL, "Complete", "Info", MB_OK);
				}
			}
		}, blendSpace);
		td.detach();
	}
	if (ImGui::DragInt("XStep", (int*)&context.xStep, 1, 0, 100)) {
		blendSpace->removeAllAnimationClipData();
		context.points.clear();
	}
	if (ImGui::DragInt("YStep", (int*)&context.yStep, 1, 0, 100)) {
		blendSpace->removeAllAnimationClipData();
		context.points.clear();
	}

	ImGui::BeginChild("NodeEditor", { 0, 0 }, false, ImGuiWindowFlags_NoMove);

	context.weights = blendSpace->animationWeights;
	context.previewPos = to(blendSpace->blendWeight);

	drawBlendSpace(info);
	if (context.highlightIndex != -1 && (ImGui::IsKeyPressedMap(ImGuiKey_Backspace) ||
		ImGui::IsKeyPressedMap(ImGuiKey_Delete))) {
		blendSpace->removeAnimationClipData(context.highlightIndex);
		context.points.erase(context.points.begin() + context.highlightIndex);
		context.highlightIndex = -1;
	}
}

float distance(const ImVec2& a, const ImVec2& b) {
	return sqrtf(powf(a.x - b.x, 2) + powf(a.y - b.y, 2));
}

void drawDashedLine(ImDrawList* list, const ImVec2& from, const ImVec2& to, float length, float space, ImU32 col, float thickness = 1) {
	float dis = distance(from, to), step = length + space;
	int s = dis / step;
	float remain = dis - s * step;
	ImVec2 dir = (to - from) / dis;
	ImVec2 pos = from;
	for (int i = 0; i < s; i++) {
		list->AddLine(pos, pos + dir * length, col, thickness);
		pos += dir * step;
	}
	if (remain >= length) {
		list->AddLine(pos, pos + dir * length, col, thickness);
	}
	else {
		list->AddLine(pos, to, col, thickness);
	}
}

void BlendSpaceWindow::drawBlendSpace(GUIRenderInfo & info)
{
	ImDrawList* list = ImGui::GetWindowDrawList();
	ImVec2 mpos = ImGui::GetMousePos();
	ImVec2 wpos = ImGui::GetWindowPos() + context.padding;
	ImVec2 owpos = wpos;
	mpos -= wpos;
	ImVec2 ompos = mpos;
	ImVec2 wsize = ImGui::GetWindowSize() - context.padding * 2;
	ImVec2 rwsize = ImVec2{ 1, 1 } / wsize;
	ImVec2 owsize = wsize;
	float xlen = wsize.x / (float)context.xStep;
	float ylen = wsize.y / (float)context.yStep;

	if (context.xStep != 0) {
		mpos.x = roundf(mpos.x / xlen) * xlen;
		for (int i = 0; i <= context.xStep; i++) {
			float v = i * (1 / (float)context.xStep);
			list->AddLine(ImVec2{ v, 0 } *owsize + owpos, ImVec2{ v, 1 } *owsize + owpos, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		}
	}
	else {
		drawDashedLine(list, ImVec2{ 0, 0 } *owsize + owpos, ImVec2{ 0, 1 } *owsize + owpos, 5, 5, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		list->AddLine(ImVec2{ 0.5, 0 } *owsize + owpos, ImVec2{ 0.5, 1 } *owsize + owpos, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		drawDashedLine(list, ImVec2{ 1, 0 } *owsize + owpos, ImVec2{ 1, 1 } *owsize + owpos, 5, 5, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		wpos.x += wsize.x / 2;
		mpos.x = 0;
		ompos.x = 0;
		wsize.x = 0;
		rwsize.x = 0;
	}
	if (context.yStep != 0) {
		mpos.y = roundf(mpos.y / ylen) * ylen;
		for (int i = 0; i <= context.yStep; i++) {
			float v = i * (1 / (float)context.yStep);
			list->AddLine(ImVec2{ 0, v } *owsize + owpos, ImVec2{ 1, v } *owsize + owpos, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		}
	}
	else {
		drawDashedLine(list, ImVec2{ 0, 0 } *owsize + owpos, ImVec2{ 1, 0 } *owsize + owpos, 5, 5, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		list->AddLine(ImVec2{ 0, 0.5 } *owsize + owpos, ImVec2{ 1, 0.5 } *owsize + owpos, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		drawDashedLine(list, ImVec2{ 0, 1 } *owsize + owpos, ImVec2{ 1, 1 } *owsize + owpos, 5, 5, ImColor(1.f, 1.f, 1.f, 0.2f), 2);
		wpos.y += wsize.y / 2;
		mpos.y = 0;
		ompos.y = 0;
		wsize.y = 0;
		rwsize.y = 0;
	}

	if (ompos.x >= 0 && ompos.y >= 0 && ompos.x <= owsize.x && mpos.y <= owsize.y) {
		list->AddCircleFilled(mpos + wpos, context.pointSize, ImColor(0.f, 1.f, 1.f, 0.3f));
		if (ImGui::IsMouseDoubleClicked(0)) {
			ImVec2 addPoint = mpos * rwsize;
			bool found = false;
			for (int i = 0; i < context.points.size(); i++)
				if (context.points[i].x == addPoint.x && context.points[i].y == addPoint.y)
					found = true;
			if (!found) {
				Asset* asset = EditorManager::getSelectedAsset();
				if (blendSpace != NULL && asset != NULL && asset->assetInfo.type == "AnimationClipData") {
					AnimationClipData* data = (AnimationClipData*)asset->load();
					if (blendSpace->addAnimationClipData({ addPoint.x, addPoint.y }, *data))
						context.points.emplace_back(addPoint);
				}
			}
		}
		if (ImGui::IsMouseDown(0)) {
			if (context.dragIndex == -1) {
				for (int i = 0; i < context.points.size(); i++) {
					if (distance(context.points[i] * wsize, ompos) < context.pointHitRadius) {
						context.dragIndex = i;
						context.dragPosReserve = mpos * rwsize;
						break;
					}
					else
						context.dragIndex = -1;
				}
			}
		}
		if (ImGui::IsMouseDragging(0)) {
			if (context.dragIndex != -1) {
				context.dragPos = mpos * rwsize;
			}
		}
		else if (ImGui::IsMouseClicked(0)) {
			for (int i = 0; i < context.points.size(); i++) {
				if (distance(context.points[i] * owsize, ompos) < context.pointHitRadius) {
					context.highlightIndex = i;
					break;
				}
				else
					context.highlightIndex = -1;
			}
		}
		if (ImGui::IsMouseReleased(0)) {
			if (context.dragIndex != -1) {
				ImVec2 p = mpos * rwsize;
				bool found = false;
				for (int i = 0; i < context.points.size(); i++)
					if (context.points[i].x == p.x && context.points[i].y == p.y)
						found = true;
				if (!found && blendSpace != NULL) {
					blendSpace->animationClipWrap[context.dragIndex].first = { p.x, p.y };
					context.points[context.dragIndex] = p;
				}
				context.dragIndex = -1;
			}
		}
		if (ImGui::IsMouseDown(2)) {
			ImVec2 p = ompos * rwsize;
			if (blendSpace != NULL) {
				blendSpace->setBlendWeight({ p.x, p.y });
				//context.previewPos = p;
			}
		}
	}
	for (int i = 0; i < context.weights.size(); i++) {
		if (context.weights[i].first != context.dragIndex || !ImGui::IsMouseDragging(0)) {
			string text = to_string(context.weights[i].second);
			ImVec2 from = context.previewPos * owsize + wpos;
			ImVec2 to = context.points[context.weights[i].first] * owsize + wpos;
			ImVec2 size = ImGui::CalcTextSize(text.c_str());
			ImVec2 tpos = from * 0.5 + to * 0.5 - size * 0.5;
			list->AddLine(from, to, ImColor(1.f, 1.f, 1.f));
			list->AddRectFilled(tpos - ImVec2{ 2, 2 }, tpos + size + ImVec2{ 2, 2 }, ImColor(1.f, 1.f, 1.f, 0.5f));
			list->AddText(tpos, ImColor(0.f, 0.f, 0.f, 0.5f), text.c_str());
		}
	}
	for (int i = 0; i < context.points.size(); i++) {
		if (i == context.dragIndex && ImGui::IsMouseDragging(0)) {
			list->AddCircle(context.dragPos * owsize + wpos, context.pointSize + context.pointBorder, ImColor(0.f, 1.f, 1.f));
			list->AddCircleFilled(context.dragPos * owsize + wpos, context.pointSize, ImColor(1.f, 1.f, 1.f));
		}
		else {
			if (i == context.highlightIndex)
				list->AddCircle(context.points[i] * owsize + wpos, context.pointSize + context.pointBorder, ImColor(1.f, 1.f, 0.f));
			list->AddCircleFilled(context.points[i] * owsize + wpos, context.pointSize, ImColor(1.f, 1.f, 1.f));
		}
	}
	list->AddCircleFilled(context.previewPos * owsize + wpos, context.pointSize, ImColor(0.f, 1.f, 0.f));
}
