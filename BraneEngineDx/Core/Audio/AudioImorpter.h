#pragma once
#include "../Importer/Importer.h"

class AudioImporter : public IImporter
{
public:
    

	AudioImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};