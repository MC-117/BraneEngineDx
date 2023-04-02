#pragma once
#include "Importer.h"

class MaterialImporter : public IImporter
{
public:
	MaterialImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};

class MaterialInstanceImporter : public IImporter
{
public:
	MaterialInstanceImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};