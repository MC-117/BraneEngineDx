#pragma once

#include "Utility.h"
#include "../Serialization.h"

ENGINE_API string getGoodRelativePath(const string& path);
ENGINE_API string getFileRoot(const string& path);
ENGINE_API string getFileName(const string& path);
ENGINE_API string getFileNameWithoutExt(const string& path);
ENGINE_API string getExtension(const string& path);

ENGINE_API void newSerializationInfoGuid(const SerializationInfo& info, map<Guid, Guid>& guidMap);
ENGINE_API void replaceSerializationInfoGuid(SerializationInfo& info, const map<Guid, Guid>& guidMap, void(*externalProc)(SerializationInfo& dst) = NULL);
ENGINE_API void newSerializationInfoGuid(SerializationInfo& info);

class Material;

ENGINE_API bool isBaseMaterial(const Material& material);
