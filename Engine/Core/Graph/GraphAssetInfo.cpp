#include "GraphAssetInfo.h"
#include <fstream>
#include "../Console.h"

GraphAssetInfo GraphAssetInfo::assetInfo;

GraphAssetInfo::GraphAssetInfo() : AssetInfo("Graph")
{
}

void* GraphAssetInfo::load(const string& name, const string& path) const
{
	ifstream wf = ifstream(path);
	SerializationInfoParser sip = SerializationInfoParser(wf, path);
	if (!sip.parse()) {
		cout << path << ": load failed '" << sip.errorString << "'\n";
		wf.close();
		return NULL;
	}
	wf.close();
	Graph* graph = NULL;
	SerializationInfo& info = SerializationInfo(sip.infos[0]);
	if (info.serialization) {
		Serializable* seriaizable = info.serialization->instantiate(info);
		if (seriaizable) {
			graph = dynamic_cast<Graph*>(seriaizable);
			if (graph)
				graph->deserialize(info);
			else
				delete seriaizable;
		}
	}
	return graph;
}

AssetInfo& GraphAssetInfo::getInstance()
{
    return assetInfo;
}

ImporterRegister<GraphImporter> graphImporter(".graph");

bool GraphImporter::loadInternal(const ImportInfo& info, ImportResult& result)
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
	Graph* graph = NULL;
	SerializationInfo& graphinfo = SerializationInfo(sip.infos[0]);
	if (graphinfo.serialization) {
		Serializable* seriaizable = graphinfo.serialization->instantiate(graphinfo);
		if (seriaizable) {
			graph = dynamic_cast<Graph*>(seriaizable);
			if (graph)
				graph->deserialize(graphinfo);
			else
				delete seriaizable;
		}
	}

	if (graph == NULL) {
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&GraphAssetInfo::assetInfo, info.filename, info.path);
	asset->asset[0] = graph;
	if (AssetManager::registAsset(*asset)) {
		result.assets.push_back(asset);
		return true;
	}
	else {
		delete graph;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}
