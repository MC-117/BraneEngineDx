#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../Object.h"
#include "../Asset.h"
#include "GUIUtility.h"

namespace ImGui {

	void PlotTimeLine(const char* label, PlotTimeLineData(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, ImVec2 frame_size)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 content_avail = GetContentRegionAvail();
		if (frame_size.x == 0.0f)
			frame_size.x = content_avail.x;
		if (frame_size.y == 0.0f)
			frame_size.y = label_size.y + (style.FramePadding.y * 2);

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
		const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, 0, &frame_bb))
			return;
		const bool hovered = ItemHoverable(frame_bb, id);

		float sum = 0;
		for (int i = 0; i < values_count; i++)
		{
			const float v = values_getter(data, i).val;
			sum += v;
		}

		RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

		const int values_count_min = 1;
		if (values_count >= values_count_min)
		{
			int res_w = ImMin((int)frame_size.x, values_count);
			int item_count = values_count;

			// Tooltip on hover
			int v_hovered = -1;
			if (hovered && inner_bb.Contains(g.IO.MousePos))
			{
				const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
				PlotTimeLineData d;
				float ds = 0;
				int v_idx = -1;
				for (int i = 0; i < values_count; i++) {
					d = values_getter(data, i);
					ds += d.val / sum;
					if (t < ds) {
						v_idx = i;
						break;
					}
				}
				IM_ASSERT(v_idx >= 0 && v_idx < values_count);

				SetTooltip("%s: %8.4g", d.label, d.val);
				v_hovered = v_idx;
			}

			const float t_step = 1.0f / (float)res_w;
			const float inv_scale = 1.0f / sum;

			float v0 = values_getter(data, (0 + values_offset) % values_count).val;
			float t0 = 0;

			for (int n = 0; n < res_w; n++)
			{
				PlotTimeLineData v = values_getter(data, (n + values_offset) % values_count);
				const float t1 = t0 + ImSaturate(v.val * inv_scale);

				// NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
				ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(t0, 0.0));
				ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(t1, 1.0));
				if (pos1.x >= pos0.x + 2.0f)
					pos1.x -= 1.0f;
				window->DrawList->AddRectFilled(pos0, pos1, v_hovered == n ? v.col_hovered : v.col, 1);
				RenderTextClipped(pos0, pos1, v.label, NULL, NULL, ImVec2(0.5f, 0.5f));
				t0 = t1;
			}
		}

		if (label_size.x > 0.0f)
			RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
	}

	bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

	void AddTextCentered(ImDrawList* drawList, const ImVec2& min_p, const ImVec2& max_p, const char* text, ImU32 col)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text);

		ImVec2 textPos;
		if ((max_p.x - min_p.x) >= textSize.x) {
			textPos.x = (max_p.x + min_p.x - textSize.x) * 0.5;
		}
		else {
			textPos.x = min_p.x;
		}
		textPos.y = min_p.y + (max_p.y - min_p.y - textSize.y) * 0.5f;

		drawList->PushClipRect(min_p, max_p);
		drawList->AddText(textPos, col, text);
		drawList->PopClipRect();

	}

	void Header(const char* label, const ImVec2& size, float rounding)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 content_avail = GetContentRegionAvail();
		ImVec2 frame_size = size;
		if (frame_size.x == 0.0f)
			frame_size.x = content_avail.x;
		if (frame_size.y == 0.0f)
			frame_size.y = label_size.y + (style.FramePadding.y * 2);

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
		const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, 0, &frame_bb))
			return;

		RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Header), true, rounding);

		AddTextCentered(window->DrawList, frame_bb.Min, frame_bb.Max, label, GetColorU32(ImGuiCol_Text));
		window->DrawList->AddRectFilled({ frame_bb.Min.x, frame_bb.Max.y - window->DC.CurrLineSize.y }, frame_bb.Max, GetColorU32(ImGuiCol_Separator),
			rounding, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight);
	}

	bool HeaderWithCloser(const char* label, const ImVec2& size, float rounding)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return true;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 content_avail = GetContentRegionAvail();
		ImVec2 frame_size = size;
		if (frame_size.x == 0.0f)
			frame_size.x = content_avail.x;
		if (frame_size.y == 0.0f)
			frame_size.y = label_size.y + (style.FramePadding.y * 2);

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
		const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, 0, &frame_bb))
			return true;

		RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Header), true, rounding);

		AddTextCentered(window->DrawList, frame_bb.Min, frame_bb.Max, label, GetColorU32(ImGuiCol_Text));
		window->DrawList->AddRectFilled({ frame_bb.Min.x, frame_bb.Max.y - window->DC.CurrLineSize.y }, frame_bb.Max, GetColorU32(ImGuiCol_Separator),
			rounding, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight);

		const ImRect button_bb(frame_bb.Max - ImVec2{ frame_size.y, frame_size.y }, frame_bb.Max);

		bool hovered = false;
		bool held = false;
		bool clicked = ButtonBehavior(button_bb, id, &hovered, &held);

		if (hovered)
			window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, GetColorU32(ImGuiCol_ButtonHovered),
				rounding, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight);
		if (held)
			window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, GetColorU32(ImGuiCol_ButtonActive),
				rounding, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight);

		AddTextCentered(window->DrawList, button_bb.Min, button_bb.Max, ICON_FA_TIMES, GetColorU32(ImGuiCol_Text));

		return !clicked;
	}

	const float HeaderBoxPadding = 5.0f;
	const float HeaderBoxRounding = 6.0f;
	const ImU32 HeaderBoxBlackColor = ImColor{ 25, 25, 25 };

	static ImVector<float> s_HeaderBoxStack;

	bool BeginHeaderBox(const char* label, const ImVec2& size)
	{
		ImGui::Indent(HeaderBoxPadding);

		// Header
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		ImGuiWindow* window = GetCurrentWindow();

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 content_avail = GetContentRegionAvail();
		ImVec2 frame_size = size;
		if (frame_size.x == 0.0f)
			frame_size.x = content_avail.x - HeaderBoxPadding * 2;
		if (frame_size.y == 0.0f)
			frame_size.y = label_size.y + (style.FramePadding.y * 2);

		s_HeaderBoxStack.push_back(frame_size.x);

		ImVec2 rectMin = window->DC.CursorPos;
		ImVec2 rectMax = { rectMin.x + content_avail.x - HeaderBoxPadding * 2, rectMin.y + 1000000 };
		ImGui::PushClipRect(rectMin, rectMax, true);

		ImGui::PushID("HeaderBox");
		const ImGuiID id = window->GetID(label);
		ImGui::PopID();

		window->DrawList->ChannelsSplit(2);
		window->DrawList->ChannelsSetCurrent(1);

		ImGui::BeginGroup();

		if (window->SkipItems)
			return true;

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
		const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, 0, &frame_bb))
			return true;

		RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Header), true, HeaderBoxRounding);

		AddTextCentered(window->DrawList, frame_bb.Min, frame_bb.Max, label, GetColorU32(ImGuiCol_Text));
		window->DrawList->AddRectFilled({ frame_bb.Min.x, frame_bb.Max.y - window->DC.CurrLineSize.y },
			frame_bb.Max, GetColorU32(ImGuiCol_Separator), HeaderBoxRounding);

		const ImRect button_bb(frame_bb.Max - ImVec2{ frame_size.y, frame_size.y }, frame_bb.Max);

		bool hovered = false;
		bool held = false;
		
		bool clicked = ButtonBehavior(button_bb, id, &hovered, &held);

		if (hovered)
			window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, GetColorU32(ImGuiCol_ButtonHovered),
				HeaderBoxRounding, ImDrawCornerFlags_TopRight);
		if (held)
			window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, GetColorU32(ImGuiCol_ButtonActive),
				HeaderBoxRounding, ImDrawCornerFlags_TopRight);

		AddTextCentered(window->DrawList, button_bb.Min, button_bb.Max, ICON_FA_TIMES, GetColorU32(ImGuiCol_Text));

		return !clicked;
	}

	void EndHeaderBox()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();

		ImGui::EndGroup();

		float frameWidth = s_HeaderBoxStack.back();
		s_HeaderBoxStack.pop_back();

		ImRect rect = g.LastItemData.Rect;
		rect.Max.x = rect.Min.x + frameWidth;
		rect.Max.y += HeaderBoxRounding;

		ImGui::Dummy({ 0, HeaderBoxRounding });

		window->DrawList->AddRect(rect.Min, rect.Max, GetColorU32(ImGuiCol_Separator),
			HeaderBoxRounding, ImDrawCornerFlags_All, window->DC.CurrLineSize.y * 2.0f);

		window->DrawList->ChannelsSetCurrent(0);

		window->DrawList->AddRectFilled(rect.Min, rect.Max, HeaderBoxBlackColor,
			HeaderBoxRounding, ImDrawCornerFlags_All);

		window->DrawList->ChannelsMerge();

		ImGui::PopClipRect();

		ImGui::Unindent(HeaderBoxPadding);
	}

	static ImVector<ImRect> s_GroupPanelLabelStack;

	void BeginGroupPanel(const char* name, const ImVec2& size)
	{
		ImGui::BeginGroup();

		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvail().x;
		else
			effectiveSize.x = size.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(name);
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		//ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x -= frameHeight;

		auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

		s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
	}

	void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		auto labelRect = s_GroupPanelLabelStack.back();
		s_GroupPanelLabelStack.pop_back();

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
		ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
		labelRect.Min.x -= itemSpacing.x;
		labelRect.Max.x += itemSpacing.x;
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
				// left half-plane
			case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
				// right half-plane
			case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
				// top
			case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
				// bottom
			case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
			}

			ImGui::GetWindowDrawList()->AddRect(
				frameRect.Min, frameRect.Max,
				ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
				halfFrame.x);

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}\

	void objectFilterContextMenuImpl(Object* root, const string& filterName, const string& filterType, Object*& selectObject)
	{
		list<Object*> objs;
		ObjectConstIterator iter(root);
		Serialization* serialization = SerializationManager::getSerialization(filterType);
		while (iter.next()) {
			Serialization& _serialization = iter.current().getSerialization();
			if ((filterType.empty() ||
				serialization == &_serialization ||
				(serialization != NULL && _serialization.isChildOf(*serialization))) &&
				iter.current().name.find(filterName) != string::npos)
				objs.push_back(&iter.current());
		}
		int i = 0;
		for (auto b = objs.begin(), e = objs.end(); b != e; b++) {
			Object* obj = *b;
			ImGui::PushID(i);
			string displayName = obj->name + '(' + obj->getSerialization().type + ')';
			if (ImGui::Selectable(displayName.c_str())) {
				selectObject = obj;
			}
			ImGui::PopID();
		}
	}

	void objectRecursiveContextMenuImpl(Object* root, Object*& selectObject)
	{
		if (root == NULL)
			return;
		for (int i = 0; i < root->children.size(); i++) {
			Object* obj = root->children[i];
			ImGui::PushID(i);
			string displayName = obj->name + '(' + obj->getSerialization().type + ')';
			if (obj->children.empty()) {
				if (ImGui::MenuItem(displayName.c_str())) {
					selectObject = obj;
				}
			}
			else{
				if (ImGui::BeginMenu(displayName.c_str())) {
					objectRecursiveContextMenuImpl(obj, selectObject);
					ImGui::EndMenu();
				}
				if (ImGui::IsItemClicked()) {
					selectObject = obj;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::PopID();
		}
	}

	bool ObjectCombo(const char* label, Object*& selectObject, Object* root, string& filterName, const string& filterType)
	{
		Object* object = selectObject;
		string displayName;
		if (selectObject == NULL)
			displayName = "(null)";
		else
			displayName = selectObject->name + '(' + selectObject->getSerialization().type + ')';
		if (ImGui::BeginCombo(label, displayName.c_str())) {
			ImGui::InputText("Name", &filterName);
			ImGui::BeginChild("ScollView", { -1, 100 });
			if (filterName.empty() && filterType.empty())
				objectRecursiveContextMenuImpl(root, selectObject);
			else
				objectFilterContextMenuImpl(root, filterName, filterType, selectObject);
			ImGui::EndChild();
			ImGui::EndCombo();
		}
		return object != selectObject;
	}
	bool ObjectCombo(const char* label, Object*& selectObject, Object* root, const string& filterType)
	{
		static string filterName;
		return ObjectCombo(label, selectObject, root, filterName, filterType);
	}

	bool AssetCombo(const char* label, Asset*& selectAsset, string& filterName, const string& AssetType)
	{
		Asset* asset = selectAsset;
		string displayName;
		if (selectAsset == NULL)
			displayName = "(null)";
		else
			displayName = selectAsset->name;
		AssetInfo* assetInfo = AssetManager::getAssetInfo(AssetType);
		if (ImGui::BeginCombo(label, displayName.c_str())) {
			ImGui::InputText("Name", &filterName);
			ImGui::BeginChild("ScollView", { -1, 100 });
			if (assetInfo != NULL) {
				for (auto b = assetInfo->assets.begin(), e = assetInfo->assets.end(); b != e; b++) {
					if (!filterName.empty() && b->second->name.find(filterName) == string::npos)
						continue;
					if (ImGui::Selectable(b->first.c_str())) {
						selectAsset = b->second;
					}
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text(b->second->path.c_str());
						ImGui::EndTooltip();
					}
				}
			}
			ImGui::EndChild();
			ImGui::EndCombo();
		}
		return asset != selectAsset;
	}

	bool AssetCombo(const char* label, Asset*& selectAsset, const string& AssetType)
	{
		static string filterName;
		return AssetCombo(label, selectAsset, filterName, AssetType);
	}
	bool TypeCombo(const char* label, Serialization*& selectType, string& filterName, const Serialization& baseType)
	{
		Serialization* oldType = selectType;
		string displayName;
		if (selectType == NULL)
			displayName = "(null)";
		else
			displayName = selectType->type;
		vector<Serialization*> types;
		baseType.getChildren(types);
		if (ImGui::BeginCombo(label, displayName.c_str())) {
			ImGui::InputText("Name", &filterName);
			ImGui::BeginChild("ScollView", { -1, 100 });
			for each (Serialization* type in types) {
				if (!filterName.empty() && type->type.find(filterName) == string::npos)
					continue;
				if (ImGui::Selectable(type->type.c_str())) {
					selectType = type;
				}
			}
			ImGui::EndChild();
			ImGui::EndCombo();
		}
		return oldType != selectType;
	}
	bool TypeCombo(const char* label, Serialization*& selectType, const Serialization& baseType)
	{
		static string filterName;
		return TypeCombo(label, selectType, filterName, baseType);
	}
}