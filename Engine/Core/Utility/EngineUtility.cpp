#include "EngineUtility.h"
#include "../Engine.h"
#include <stack>

string getContentFolder()
{
    return string();
}

string getEngineFolder()
{
    return string();
}

string getGoodRelativePath(const string& path)
{
	filesystem::path p = absolute(filesystem::u8path(path));
	filesystem::path ep = filesystem::u8path(Engine::windowContext.workingPath);
	string filename = p.filename().string();
	vector<string> ps = split(p.relative_path().make_preferred().remove_filename().string(), '\\');
	vector<string> eps = split(ep.relative_path().make_preferred().string(), '\\');
	bool found = false;
	string rpath;
	if (ps.size() > eps.size()) {
		for (int i = 0; i < ps.size(); i++) {
			if (i < eps.size()) {
				if (ps[i] != eps[i])
					break;
			}
			else if (found)
				rpath += ps[i] + '/';
			else if (ps[i] == "Content" || ps[i] == "Engine") {
				found = true;
				rpath += ps[i] + '/';
			}
			else
				break;
		}
	}
	if (found)
		return rpath + filename;
	else
		return string();
}

string getFileRoot(const string& path)
{
    size_t pos = path.find_last_of('/');
    if (pos == string::npos) {
        pos = path.find_last_of('\\');
        if (pos == string::npos) {
            return path;
        }
    }

    return path.substr(0, pos);
}

string getFileName(const string& path)
{
    size_t pos = path.find_last_of('/');
    if (pos == string::npos) {
        pos = path.find_last_of('\\');
        if (pos == string::npos) {
            return path;
        }
    }

    return path.substr(pos + 1);
}

string getFileNameWithoutExt(const string& path)
{
    size_t pos = path.find_last_of('/');
    if (pos == string::npos) {
        pos = path.find_last_of('\\');
        if (pos == string::npos) {
            return path;
        }
    }

    size_t dotPos = path.find_last_of('.');
    size_t size = dotPos - pos - 1;

    return path.substr(pos + 1, size > 0 ? size : string::npos);
}

string getExtension(const string& path)
{
    string name = getFileName(path);
    size_t dotPos = name.find_last_of('.');
    if (dotPos == string::npos)
        return "";
    return name.substr(dotPos);
}

void charToWcharString(const char* charStr, wstring& wcharStr)
{
    size_t len = strlen(charStr) + 1;
    size_t converted = 0;
    wcharStr.resize(len);
    mbstowcs_s(&converted, wcharStr.data(), len, charStr, len);
}

void newSerializationInfoGuid(const SerializationInfo& info, map<Guid, Guid>& guidMap)
{
    auto newGuid = [&](const Guid& guid)
    {
        if (guidMap.find(guid) == guidMap.end())
            guidMap[guid] = Guid::newGuid();
    };
    string guidStr;
    if (info.type != "Ref" && info.get("guid", guidStr)) {
        Guid oldGuid = Guid::fromString(guidStr);
        newGuid(oldGuid);
    }
    stack<pair<vector<SerializationInfo>::const_iterator, vector<SerializationInfo>::const_iterator>> iters;
    if (!info.sublists.empty())
        iters.push({ info.sublists.begin(),info.sublists.end() });
    while (!iters.empty()) {
        auto& iter = iters.top();
        if (iter.first == iter.second) {
            iters.pop();
        }
        else {
            const SerializationInfo& subinfo = *iter.first;
            if (subinfo.type != "Ref" && subinfo.get("guid", guidStr)) {
                Guid oldGuid = Guid::fromString(guidStr);
                newGuid(oldGuid);
            }
            if (!subinfo.sublists.empty())
                iters.push({ subinfo.sublists.begin(), subinfo.sublists.end() });
            iter.first++;
        }
    }
}

void replaceSerializationInfoGuid(SerializationInfo& info, const map<Guid, Guid>& guidMap, void(*externalProc)(SerializationInfo& dst))
{
    string guidStr;
    auto replaceGuid = [&](SerializationInfo& dst)
    {
        dst.path.clear();
        if (dst.get("guid", guidStr)) {
            Guid oldGuid = Guid::fromString(guidStr);
            auto iter = guidMap.find(oldGuid);
            if (iter == guidMap.end()) {
                if (externalProc)
                    externalProc(dst);
            }
            else {
                dst.set("guid", iter->second.toString());
            }
        }
    };
    replaceGuid(info);
    stack<pair<vector<SerializationInfo>::iterator, vector<SerializationInfo>::iterator>> iters;
    if (!info.sublists.empty())
        iters.push({ info.sublists.begin(),info.sublists.end() });
    while (!iters.empty()) {
        auto& iter = iters.top();
        if (iter.first == iter.second) {
            iters.pop();
        }
        else {
            SerializationInfo& subinfo = *iter.first;
            replaceGuid(subinfo);
            if (!subinfo.sublists.empty())
                iters.push({ subinfo.sublists.begin(), subinfo.sublists.end() });
            iter.first++;
        }
    }
}

void newSerializationInfoGuid(SerializationInfo& info)
{
    map<Guid, Guid> guidMap;
    newSerializationInfoGuid(info, guidMap);
    replaceSerializationInfoGuid(info, guidMap);
}

bool isBaseMaterial(const Material& material)
{
    Shader* shader = material.getShader();
    return shader && shader->getBaseMaterial() == &material;
}
