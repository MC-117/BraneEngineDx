#include "IgnoreImporter.h"

ImporterRegister<IgnoreImporter> ttfImporter(".ttf");
ImporterRegister<IgnoreImporter> iniImporter(".ini");
ImporterRegister<IgnoreImporter> shadapterImporter(".shadapter");
ImporterRegister<IgnoreImporter> jsonImporter(".json");

bool IgnoreImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	return true;
}