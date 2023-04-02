#pragma once
#include "Importer.h"

class TextureMipImporter : public IImporter
{
public:
	TextureMipImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};

class TextureImporter : public IImporter
{
public:
	TextureImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};