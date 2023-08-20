#pragma once

#include "Utility.h"
#include "../Serialization.h"

string getGoodRelativePath(const string& path);
string getFileRoot(const string& path);
string getFileName(const string& path);
string getFileNameWithoutExt(const string& path);
string getExtension(const string& path);

void newSerializationInfoGuid(const SerializationInfo& info, map<Guid, Guid>& guidMap);
void replaceSerializationInfoGuid(SerializationInfo& info, const map<Guid, Guid>& guidMap, void(*externalProc)(SerializationInfo& dst) = NULL);
void newSerializationInfoGuid(SerializationInfo& info);
