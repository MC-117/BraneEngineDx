#pragma once
#include "Importer.h"

class TimelineImporter : public IImporter
{
public:
	TimelineImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};