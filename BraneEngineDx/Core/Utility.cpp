#include "Utility.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../ThirdParty/ImGui/imgui_internal.h"
#include <fstream>

vector<string> split(const string& str, char ch, unsigned int num, void(*post)(string&))
{
	vector<string> re;
	string p;
	int c = 0;
	for (int i = 0; i < str.size(); i++) {
		if (c < num - 1) {
			if (str[i] == ch) {
				if (!p.empty()) {
					if (post != NULL)
						post(p);
					re.push_back(p);
					p.clear();
					c++;
				}
			}
			else
				p += str[i];
		}
		else
			p += str[i];
	}
	if (!p.empty() && c < num)
		re.push_back(p);
	return re;
}

vector<string> split(const string& str, const string& chs, unsigned int num, void(*post)(string&))
{
	vector<string> re;
	string p;
	int c = 0;
	for (int i = 0; i < str.size(); i++) {
		if (c < num - 1) {
			if (chs.find(str[i]) != string::npos) {
				if (!p.empty()) {
					if (post != NULL)
						post(p);
					re.push_back(p);
					p.clear();
					c++;
				}
			}
			else
				p += str[i];
		}
		else
			p += str[i];
	}
	if (!p.empty() && c < num)
		re.push_back(p);
	return re;
}

string& trim(string& str, char ch)
{
	str.erase(0, str.find_first_not_of(ch));
	str.erase(str.find_last_not_of(ch) + 1);
	return str;
}

string& trimL(string& str, const string& chs)
{
	if (!str.empty())
		for (int i = 0; i < str.length(); i++) {
			char c = str[i];
			bool find = false;
			for (int j = 0; j < chs.length(); j++) {
				if (chs[j] == c) {
					find = true;
					break;
				}
			}
			if (!find) {
				str.erase(0, i);
				break;
			}
		}
	return str;
}

string& trimR(string& str, const string& chs)
{
	if (!str.empty())
		for (int i = str.length() - 1; i >= 0; i--) {
			char c = str[i];
			bool find = false;
			for (int j = 0; j < chs.length(); j++) {
				if (chs[j] == c) {
					find = true;
					break;
				}
			}
			if (!find) {
				if (i < str.length() - 1)
					str.erase(i + 1);
				break;
			}
		}
	return str;
}

string& trim(string& str, const string& chs)
{
	return trimR(trimL(str, chs), chs);
}

bool splitPattern(const string& str, char start, char end, vector<pair<bool, string>>& re, bool(*post)(bool, string&))
{
	if (str.empty())
		return true;
	string s = str, temp;
	int count = 0;
	for (int i = 0; i < s.length(); i++) {
		char c = s[i];
		if (c == start) {
			if (count == 0) {
				if (post == NULL || (post != NULL && post(false, temp)))
					re.push_back(pair<bool, string>(false, temp));
				temp.clear();
			}
			else
				temp += c;
			count++;
		}
		else if (c == end) {
			if (count > 0) {
				if (count == 1) {
					if (post == NULL || (post != NULL && post(true, temp)))
						re.push_back(pair<bool, string>(true, temp));
					temp.clear();
				}
				else
					temp += c;
				count--;
			}
			else
				return false;
		}
		else {
			temp += c;
		}
	}
	return count == 0;
}

bool splitMap(const string& str, char start, char end, map<string, string>& re, bool(*post)(string&, string&))
{
	vector<pair<bool, string>> t;
	if (!splitPattern(str, start, end, t, [](bool b, string& str) -> bool { trim(str, " \t\n"); return b || (!b && !str.empty()); }) || t.size() % 2 != 0)
		return false;
	for (int i = 0;i < t.size(); i += 2) {
		if (!t[i].first && t[i + 1].first) {
			return false;
		}
		if (post(t[i].second, t[i + 1].second))
			re.insert(pair<string, string>(t[i].second, t[i + 1].second));
	}
	return true;
}

bool equal(const Quaternionf& q0, const Quaternionf& q1, float threahold)
{
	return fabsf(q0.dot(q1) - 1) < threahold;
}

string& validateRelativePath(string& path)
{
	vector<string> terms = split(path, "\\/");
	for (int i = 0; i < terms.size();) {
		if (terms[i] == ".")
			terms.erase(terms.begin() + i);
		else if (i != 0 && terms[i] == "..") {
			if (i == 0)
				terms.erase(terms.begin() + i);
			else {
				terms.erase(terms.begin() + (i - 1), terms.begin() + (i + 1));
				i--;
			}
		}
		else i++;
	}
	path.clear();
	for (int i = 0; i < terms.size(); i++)
		path += i == 0 ? terms[i] : ('/' + terms[i]);
	return path;
}

string getFilePath(const string& path, const string& pwd)
{
	filesystem::path p = pwd;
	p /= path;
	if (filesystem::exists(p))
		return p.generic_u8string();
	if (filesystem::exists(path))
		return path;
	return "";
}

bool readHeadFile(const string& codeLine, string& code, const string& envPath, unordered_set<string>& headFiles)
{
	size_t tpos = codeLine.find("#include");
	if (tpos == -1) {
		code += codeLine + '\n';
		return true;
	}
	string filePath = codeLine.substr(codeLine.find("#include") + strlen("#include"));
	trim(filePath, " <>\"");
	string _filePath = getFilePath(filePath, envPath);
	validateRelativePath(_filePath);
	if (_filePath.empty()) {
		printf("Head file \"%s\" open failed", filePath.c_str());
		return false;
	}
	filePath = _filePath;
	if (headFiles.find(filePath) != headFiles.end())
		return true;
	ifstream f = ifstream(filePath);
	if (!f.is_open()) {
		printf("Head file \"%s\" open failed", filePath.c_str());
		return false;
	}
	headFiles.insert(filePath);
	string line;
	while (1) {
		if (!getline(f, line))
			break;
		if (line.empty())
			continue;
		filesystem::path p = filePath;
		if (!readHeadFile(line, code, p.parent_path().generic_u8string(), headFiles)) {
			f.close();
			return false;
		}
	}
	f.close();
	return true;
}

bool openFileDlg(FileDlgDesc& desc)
{
	OPENFILENAME ofn = { 0 };
	char filePath[MAX_PATH] = { 0 };
	char fileName[FILENAME_MAX] = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = desc.own;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = fileName;
	ofn.nMaxFileTitle = FILENAME_MAX;
	ofn.lpstrInitialDir = desc.initDir.c_str();
	ofn.lpstrTitle = desc.title.c_str();
	ofn.lpstrFilter = desc.filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = desc.defFileExt.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (desc.addToRecent)
		ofn.Flags |= OFN_DONTADDTORECENT;
	bool res = desc.save ? GetSaveFileName(&ofn) : GetOpenFileName(&ofn);
	if (res) {
		desc.filePath = filePath;
		desc.fileName = fileName;
	}
	return res;
}

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
}

Path::Path()
{
}

Path::Path(const Path& path)
{
	logicPath = path.logicPath;
}

Path::Path(Path&& path)
{
	logicPath = path.logicPath;
}

Path::Path(const string& path)
{
	string p = path;
	trimSpace(p);
	vector<string> re = split(p, '.', -1, trimSpace);
	for (int i = 0; i < re.size(); i++) {
		if (!re[i].empty())
			logicPath.push_back(re[i]);
	}
}

Path::Path(const char* path)
{
	string p = path;
	trimSpace(p);
	vector<string> re = split(p, '.', -1, trimSpace);
	for (int i = 0; i < re.size(); i++) {
		if (!re[i].empty())
			logicPath.push_back(re[i]);
	}
}

bool Path::empty() const
{
	return logicPath.empty();
}

size_t Path::size() const
{
	return logicPath.size();
}

void Path::up()
{
	if (logicPath.empty())
		logicPath.pop_back();
}

void Path::down(const string& name)
{
	string n = name;
	trim(n, " \t.>");
	if (!n.empty())
		logicPath.push_back(name);
}

Path Path::popTop() const
{
	if (logicPath.empty())
		return Path();
	Path p;
	p.logicPath.resize(logicPath.size() - 1);
	copy(++logicPath.begin(), logicPath.end(), p.logicPath.begin());
	return p;
}

Path& Path::operator=(const Path& path)
{
	logicPath = path.logicPath;
	return *this;
}

Path& Path::operator=(Path&& path)
{
	logicPath = path.logicPath;
	return *this;
}

bool Path::operator==(const Path& path) const
{
	return logicPath == path.logicPath;
}

bool Path::operator!=(const Path& path) const
{
	return logicPath != path.logicPath;
}

string& Path::operator[](size_t i)
{
	return logicPath[i];
}

string Path::operator[](size_t i) const
{
	return logicPath[i];
}

Path::operator string() const
{
	string re;
	for (int i = 0; i < logicPath.size(); i++)
		if (i == 0)
			re += logicPath[i];
		else
			re += '|' + logicPath[i];
	return re;
}

bool AssetPath::operator<(const AssetPath& a) const
{
	if (path < a.path)
		return true;
	else if (path == a.path)
		return name < a.name;
	return false;
}
