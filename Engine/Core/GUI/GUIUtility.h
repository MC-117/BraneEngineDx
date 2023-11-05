#pragma once

#include "../Config.h"
#include "../Serialization.h"
#include "../Utility/Curve.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../ThirdParty/ImGui/imgui_internal.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../../ThirdParty/ImGui/ImGuiIconHelp.h"
#include "../../ThirdParty/ImGui/ImPlot/implot.h"
#include "../../ThirdParty/ImGui/ImPlot/implot_internal.h"

class Asset;
class Object;

namespace ImGui {

	ENGINE_API void Setup();
	
	ENGINE_API void SetDefaultStyleColors();

	ENGINE_API void Release();

	struct PlotTimeLineData
	{
		const char* label;
		float val;
		ImU32 col;
		ImU32 col_hovered;
	};

	ENGINE_API void PlotTimeLine(const char* label, PlotTimeLineData(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, ImVec2 frame_size);

	ENGINE_API bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

	ENGINE_API void AddTextCentered(ImDrawList* drawList, const ImVec2& min_p, const ImVec2& max_p, const char* text, ImU32 col = GetColorU32(ImGuiCol_Text));

	ENGINE_API void Header(const char* label, const ImVec2& size = { 0, 0 }, float rounding = 0);
	ENGINE_API bool HeaderWithCloser(const char* label, const ImVec2& size = { 0, 0 }, float rounding = 0);

	ENGINE_API bool BeginHeaderBox(const char* label, const ImVec2& size = { 0, 0 });
	ENGINE_API void EndHeaderBox();

	ENGINE_API void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
	ENGINE_API void EndGroupPanel();

	struct TimelineContext
	{

	};

	struct TimelineActionData
	{

	};

	template<class K>
	struct CurveContext
	{
		ImVec4 curveNormalColor;
		ImVec4 curveSelectedColor;
		ImVec4 keyNormalColor;
		ImVec4 keySelectedColor;
		bool isKeySelected;
		K selectedKey;
	};

	ENGINE_API void CurveView(const char* name, Curve<float, float>* curve);

	ENGINE_API bool ObjectCombo(const char* label, Object*& selectObject, Object* root, string& filterName, const string& filterType = "");
	ENGINE_API bool ObjectCombo(const char* label, Object*& selectObject, Object* root, const string& filterType = "");
	ENGINE_API bool AssetCombo(const char* label, Asset*& selectAsset, string& filterName, const string& AssetType);
	ENGINE_API bool AssetCombo(const char* label, Asset*& selectAsset, const string& AssetType);
	ENGINE_API bool TypeCombo(const char* label, Serialization*& selectType, string& filterName, const Serialization& baseType);
	ENGINE_API bool TypeCombo(const char* label, Serialization*& selectType, const Serialization& baseType);
}