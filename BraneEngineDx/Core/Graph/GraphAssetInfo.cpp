#include "GraphAssetInfo.h"
#include <fstream>

GraphAssetInfo GraphAssetInfo::assetInfo;

GraphAssetInfo::GraphAssetInfo() : AssetInfo("Graph")
{
}

void* GraphAssetInfo::load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const
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
