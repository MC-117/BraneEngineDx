#pragma once
#include "Importer.h"

class IgnoreImporter : public IImporter
{
public:
	IgnoreImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};