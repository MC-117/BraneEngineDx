#include "Utility.h"
#include <fstream>
#include <ShlObj.h>

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

PVec3 toPVec3(const Vector3f& v)
{
	return PVec3(v.x(), v.y(), v.z());
}

PQuat toPQuat(const Quaternionf& q)
{
	return PQuat(q.x(), q.y(), q.z(), q.w());
}

PTransform toPTransform(const Vector3f& v, const Quaternionf& q)
{
#ifdef PHYSICS_USE_PHYSX
	return PTransform(toPVec3(v), toPQuat(q));
#endif
}

Vector3f toVector3f(const PVec3& v)
{
#ifdef PHYSICS_USE_PHYSX
	return Vector3f(v.x, v.y, v.z);
#endif
}

Quaternionf toQuaternionf(const PQuat& q)
{
#ifdef PHYSICS_USE_PHYSX
	return Quaternionf(q.x, q.y, q.z, q.w);
#endif
}

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

const float xmin = 0.0001f;
const float xmax = 0.9999f;
const float ymin = 0.0001f;
const float ymax = 0.9999f;

int ComputeOutCode(const Vector2f& p)
{
	int code;

	code = INSIDE;          // initialised as being inside of [[clip window]]

	if (p.x() < xmin)           // to the left of clip window
		code |= LEFT;
	else if (p.x() > xmax)      // to the right of clip window
		code |= RIGHT;
	if (p.y() < ymin)           // below the clip window
		code |= BOTTOM;
	else if (p.y() > ymax)      // above the clip window
		code |= TOP;

	return code;
}

void CohenSutherlandLineClipAndDraw(Vector2f& p0, Vector2f& p1)
{
	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	int outcode0 = ComputeOutCode(p0);
	int outcode1 = ComputeOutCode(p1);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) {
			// bitwise OR is 0: both points inside window; trivially accept and exit loop
			accept = true;
			break;
		}
		else if (outcode0 & outcode1) {
			// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
			// or BOTTOM), so both must be outside window; exit loop (accept is false)
			break;
		}
		else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			float x = 0, y = 0;

			// At least one endpoint is outside the clip rectangle; pick it.
			int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

			// Now find the intersection point;
			// use formulas:
			//   slope = (y1 - y0) / (x1 - x0)
			//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
			//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
			// No need to worry about divide-by-zero because, in each case, the
			// outcode bit being tested guarantees the denominator is non-zero
			if (outcodeOut & TOP) {           // point is above the clip window
				x = p0.x() + (p1.x() - p0.x()) * (ymax - p0.y()) / (p1.y() - p0.y());
				y = ymax;
			}
			else if (outcodeOut & BOTTOM) { // point is below the clip window
				x = p0.x() + (p1.x() - p0.x()) * (ymin - p0.y()) / (p1.y() - p0.y());
				y = ymin;
			}
			else if (outcodeOut & RIGHT) {  // point is to the right of clip window
				y = p0.y() + (p1.y() - p0.y()) * (xmax - p0.x()) / (p1.x() - p0.x());
				x = xmax;
			}
			else if (outcodeOut & LEFT) {   // point is to the left of clip window
				y = p0.y() + (p1.y() - p0.y()) * (xmin - p0.x()) / (p1.x() - p0.x());
				x = xmin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				p0.x() = x;
				p0.y() = y;
				outcode0 = ComputeOutCode(p0);
			}
			else {
				p1.x() = x;
				p1.y() = y;
				outcode1 = ComputeOutCode(p1);
			}
		}
	}
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

	string pwd;
	if (filePath[0] == '/' || filePath[0] == '\\') {
		filePath.erase(0, 1);
		pwd = "Engine/Shaders";
	}
	else {
		pwd = envPath;
	}
	string _filePath = getFilePath(filePath, pwd);
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
	wchar_t cwd[MAX_PATH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH, cwd);
	OPENFILENAMEW ofn = { 0 };
	wchar_t filePath[MAX_PATH] = { 0 };
	wchar_t fileName[FILENAME_MAX] = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = desc.own;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = fileName;
	ofn.nMaxFileTitle = FILENAME_MAX;
	wstring dir = wstring(desc.initDir.begin(), desc.initDir.end());
	wstring title = wstring(desc.title.begin(), desc.title.end());
	wstring filter = wstring(desc.filter.begin(), desc.filter.end());
	for (auto b = filter.begin(), e = filter.end(); b != e; b++) {
		if (*b == '|')
			*b = '\0';
	}
	filter.push_back('\0');
	wstring defFileExt = wstring(desc.defFileExt.begin(), desc.defFileExt.end());
	ofn.lpstrInitialDir = dir.c_str();
	ofn.lpstrTitle = title.c_str();
	ofn.lpstrFilter = filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = defFileExt.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (desc.addToRecent)
		ofn.Flags |= OFN_DONTADDTORECENT;
	
	bool res = desc.save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
	if (res) {
		filesystem::path pfilePath = filePath;
		filesystem::path pfileName = fileName;
		desc.filePath = pfilePath.generic_u8string();
		desc.fileName = pfileName.generic_u8string();
	}
	SetCurrentDirectoryW(cwd);
	return res;
}

bool openFolderDlg(FolderDlgDesc& desc)
{
	/*BROWSEINFOA bi;
	bi.hwndOwner = desc.own;
	LPITEMIDLIST rlist = ILCreateFromPathA(desc.initDir.c_str());
	bi.pidlRoot = rlist;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = desc.title.c_str();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
	bi.lpfn = NULL;
	bi.iImage = 0;
	LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
	if (!pidl)
		return false;
	char szDisplayName[_MAX_PATH];
	SHGetPathFromIDListA(pidl, szDisplayName);
	desc.folderPath = szDisplayName;
	ILFree(pidl);
	ILFree(rlist);
	return true;*/
	wchar_t cwd[MAX_PATH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH, cwd);
	OPENFILENAMEW ofn = { 0 };
	wchar_t filePath[MAX_PATH] = L"Folder";
	wchar_t fileName[FILENAME_MAX] = L"Folder";
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = desc.own;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = fileName;
	ofn.nMaxFileTitle = FILENAME_MAX;
	wstring dir = wstring(desc.initDir.begin(), desc.initDir.end());
	wstring title = wstring(desc.title.begin(), desc.title.end());
	ofn.lpstrInitialDir = dir.c_str();
	ofn.lpstrTitle = title.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrFilter = L"folder\0folder\0\0";
	ofn.lpstrDefExt = L"folder";
	ofn.Flags = OFN_HIDEREADONLY | OFN_NOVALIDATE | OFN_PATHMUSTEXIST | OFN_READONLY;

	bool res = GetOpenFileNameW(&ofn);
	if (res) {
		filesystem::path pfilePath = filePath;
		desc.folderPath = pfilePath.parent_path().generic_u8string();
	}
	SetCurrentDirectoryW(cwd);
	return res;
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

void LongProgressWork::setProgress(float progress)
{
	this->progress = progress;
	callback(*this);
}

void LongProgressWork::setProgress(float progress, const string& text)
{
	this->progress = progress;
	this->text = text;
	callback(*this);
}
