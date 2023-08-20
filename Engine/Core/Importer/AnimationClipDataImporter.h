#pragma once
#include "Importer.h"

class AnimationClipDataImporter : public IImporter
{
public:
	AnimationClipDataImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};