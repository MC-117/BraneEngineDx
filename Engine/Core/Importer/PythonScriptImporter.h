#pragma once
#include "Importer.h"

class PythonScriptImporter : public IImporter
{
public:
	PythonScriptImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};