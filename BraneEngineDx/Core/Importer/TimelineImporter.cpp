#include "TimelineImporter.h"
#include <fstream>
#include "../Asset.h"
#include "../Console.h"
#include "../Timeline/Timeline.h"

ImporterRegister<TimelineImporter> timelineImporter(".timeline", true);

bool TimelineImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	ifstream wf = ifstream(info.path);
	SerializationInfoParser sip = SerializationInfoParser(wf, info.path);
	if (!sip.parse()) {
		Console::error("%s: load failed '%s'", info.path.c_str(), sip.parseError.c_str());
		wf.close();
		result.status = ImportResult::LoadFailed;
		return false;
	}
	wf.close();
	SerializationInfo& timelinInfo = SerializationInfo(sip.infos[0]);
	Timeline* timeline = NULL;
	if (timelinInfo.serialization) {
		Serializable* seriaizable = timelinInfo.serialization->instantiate(timelinInfo);
		if (seriaizable) {
			timeline = dynamic_cast<Timeline*>(seriaizable);
			if (timeline)
				timeline->deserialize(timelinInfo);
			else
				delete seriaizable;
		}
	}

	if (timeline == NULL) {
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&TimelineAssetInfo::assetInfo, info.filename, info.path);
	asset->asset[0] = timeline;
	if (AssetManager::registAsset(*asset)) {
		result.assets.push_back(asset);
		return true;
	}
	else {
		delete timeline;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}