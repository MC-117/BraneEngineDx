#pragma once
#include "Importer.h"

class AssetFileImporter : public IImporter
{
public:
	AssetFileImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};