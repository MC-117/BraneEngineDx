#pragma once
#include "TransformRenderData.h"

struct MeshBatchDrawKeyBase
{
	bool isValid() const { return false; }
	void assignDrawCommand(DrawElementsIndirectCommand& command) { }
	unsigned int getDrawCommandKey() { return 0; }
};

template<class Data>
struct TDrawCallBase
{
	template<class K>
	void addDraw(const K& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount) { }

	void calculateCountAndOffset(unsigned int instanceOffset, unsigned int commandOffset) { }
	
	void fetchInstanceData(Data* dst, unsigned int count) { }
	unsigned int getInstanceCount() const { return 0; }
	unsigned int getInstanceOffset() const { return 0; }

	void fetchDrawCommands(DrawElementsIndirectCommand* dst, unsigned int count) { }
	unsigned int getDrawCommandCount() const { return 0; }
	unsigned int getDrawCommandOffset() const { return 0; }
	
	void clean() { }
};

template<class Key, class Data, template<class T> class TCall>
struct TMeshBatchDrawCallCollection
{
	using Call = TCall<Data>;
	using CallMap = map<Key, Call>;
	using CallItem = pair<const Key, Call>;

	CallMap callMap;
	unsigned int instanceCount = 0;
	unsigned int commandCount = 0;

	Call* getMeshBatchDrawCall(const Key& key);
	Call* setMeshBatchDrawCall(const Key& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount = 1);
	
	void calculateCountAndOffset();

	void fetchInstanceData(vector<Data>& data);
	void fetchDrawCommands(vector<DrawElementsIndirectCommand>& commands);

	void clean();
	bool cleanByKey(const Key& key);
};

template<class Key, class Data, template<class T> class TCall>
inline TCall<Data>* TMeshBatchDrawCallCollection<Key, Data, TCall>::getMeshBatchDrawCall(const Key& key)
{
	auto iter = callMap.find(key);
	if (iter != callMap.end())
		return &iter->second;
	return NULL;
}

template<class Key, class Data, template<class T> class TCall>
inline TCall<Data>* TMeshBatchDrawCallCollection<Key, Data, TCall>::setMeshBatchDrawCall(const Key& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount)
{
	auto& call = callMap.try_emplace(key).first->second;
	call.addDraw(key, data, instanceIndex, instanceCount);
	return &call;
}

template<class Key, class Data, template<class T> class TCall>
inline void TMeshBatchDrawCallCollection<Key, Data, TCall>::calculateCountAndOffset()
{
	unsigned int instanceOffset = 0;
	unsigned int commandOffset = 0;
	for (auto b = callMap.begin(), e = callMap.end(); b != e; b++) {
		b->second.calculateCountAndOffset(instanceOffset, commandOffset);
		instanceOffset += b->second.getInstanceCount();
		commandOffset += b->second.getDrawCommandCount();
	}
	instanceCount = instanceOffset;
	commandCount = commandOffset;
}

template<class Key, class Data, template<class T> class TCall>
inline void TMeshBatchDrawCallCollection<Key, Data, TCall>::fetchInstanceData(vector<Data>& data)
{
	data.resize(instanceCount);
	for (auto b = callMap.begin(), e = callMap.end(); b != e; b++) {
		if (const unsigned int count = b->second.getInstanceCount())
			b->second.fetchInstanceData(&data[b->second.getInstanceOffset()], count);
	}
}

template<class Key, class Data, template<class T> class TCall>
inline void TMeshBatchDrawCallCollection<Key, Data, TCall>::fetchDrawCommands(vector<DrawElementsIndirectCommand>& commands)
{
	commands.resize(commandCount);
	for (auto b = callMap.begin(), e = callMap.end(); b != e; b++) {
		if (const unsigned int count = b->second.getDrawCommandCount())
			b->second.fetchDrawCommands(&commands[b->second.getDrawCommandOffset()], count);
	}
}

template<class Key, class Data, template<class T> class TCall>
inline void TMeshBatchDrawCallCollection<Key, Data, TCall>::clean()
{
	for (auto b = callMap.begin(), e = callMap.end(); b != e; b++) {
		b->second.clean();
	}
	instanceCount = 0;
	commandCount = 0;
}

template<class Key, class Data, template<class T> class TCall>
inline bool TMeshBatchDrawCallCollection<Key, Data, TCall>::cleanByKey(const Key& key)
{
	auto iter = callMap.find(key);
	if (iter != callMap.end()) {
		instanceCount -= iter->second.getInstanceCount();
		commandCount -= iter->second.getDrawCommandOffset();
		iter->second.clean();
		return true;
	}
	return false;
}

struct MeshBatchDrawKey : MeshBatchDrawKeyBase
{
	MeshPart* meshPart = NULL;
	Material* material = NULL;
	bool negativeScale = false;

	MeshBatchDrawKey(MeshPart* meshPart, Material* material, bool negativeScale = false);
	bool isValid() const;
	void assignDrawCommand(DrawElementsIndirectCommand& command) const;
	unsigned int getDrawCommandKey() const;
	bool operator<(const MeshBatchDrawKey& key) const;
	bool operator==(const MeshBatchDrawKey& key) const;
};

template <>
struct std::hash<MeshBatchDrawKey>
{
	std::size_t operator()(const MeshBatchDrawKey& key) const;
};

template<class T>
T drawInstanceIncrease(const T& data);

template<>
inline InstanceDrawData drawInstanceIncrease(const InstanceDrawData& data)
{
	InstanceDrawData outData = data;
	outData.instanceID++;
	return outData;
}

template<class Data>
struct TMeshBatchDrawCall : TDrawCallBase<Data>
{
	struct PerCall
	{
		vector<Data> instanceData;
		DrawElementsIndirectCommand command;

		PerCall() : command({ 0, 0, 0, 0, 0 }) {}
		bool empty() const { return instanceData.empty(); }
	};
	unordered_map<unsigned int, PerCall> batches;
	unsigned int instanceCount = 0;
	unsigned int instanceOffset = 0;
	unsigned int drawCommandCount = 0;
	unsigned int drawCommandOffset = 0;
	bool reverseCullMode = false;
	
	template<class K>
	void addDraw(const K& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount);

	void calculateCountAndOffset(unsigned int instanceOffset, unsigned int commandOffset);
	
	void fetchInstanceData(Data* dst, unsigned int count);
	unsigned int getInstanceCount() const;
	unsigned int getInstanceOffset() const;

	void fetchDrawCommands(DrawElementsIndirectCommand* dst, unsigned int count);
	unsigned int getDrawCommandCount() const;
	unsigned int getDrawCommandOffset() const;
	
	void clean();
};

template <class Data>
template <class K>
void TMeshBatchDrawCall<Data>::addDraw(const K& key, const Data& data, unsigned int instanceIndex, unsigned int instanceCount)
{
	unsigned int commandKey = key.getDrawCommandKey();
	PerCall& call = batches.try_emplace(commandKey).first->second;
	if (call.empty())
	{
		key.assignDrawCommand(call.command);
		this->drawCommandCount++;
	}
	unsigned int index = call.instanceData.size();
	unsigned int count = index + instanceCount;
	call.instanceData.resize(count);
	Data _data = data;
	for (; index < count; index++)
	{
		call.instanceData[index] = _data;
		_data = drawInstanceIncrease(_data);
	}
	call.command.instanceCount += instanceCount;
	this->instanceCount += instanceCount;
}

template <class Data>
void TMeshBatchDrawCall<Data>::calculateCountAndOffset(unsigned int instanceOffset, unsigned int commandOffset)
{
	this->instanceOffset = instanceOffset;
	this->drawCommandOffset = commandOffset;
	for (auto& item : batches)
	{
		DrawElementsIndirectCommand& command = item.second.command;
		command.baseInstance = instanceOffset;
		instanceOffset += command.instanceCount;
	}
}

template <class Data>
void TMeshBatchDrawCall<Data>::fetchInstanceData(Data* dst, unsigned int count)
{
	unsigned int offset = 0;
	for (auto& item : batches)
	{
		DrawElementsIndirectCommand& command = item.second.command;
		memcpy(dst + offset, item.second.instanceData.data(), sizeof(Data) * command.instanceCount);
		offset += command.instanceCount;
	}
}

template <class Data>
unsigned TMeshBatchDrawCall<Data>::getInstanceCount() const
{
	return instanceCount;
}

template <class Data>
unsigned TMeshBatchDrawCall<Data>::getInstanceOffset() const
{
	return instanceOffset;
}

template <class Data>
void TMeshBatchDrawCall<Data>::fetchDrawCommands(DrawElementsIndirectCommand* dst, unsigned int count)
{
	unsigned int offset = 0;
	for (auto& item : batches)
	{
		memcpy(dst + offset, &item.second.command, sizeof(DrawElementsIndirectCommand));
		offset++;
	}
}

template <class Data>
unsigned TMeshBatchDrawCall<Data>::getDrawCommandCount() const
{
	return drawCommandCount;
}

template <class Data>
unsigned TMeshBatchDrawCall<Data>::getDrawCommandOffset() const
{
	return drawCommandOffset;
}

template <class Data>
void TMeshBatchDrawCall<Data>::clean()
{
	batches.clear();
	instanceCount = 0;
	instanceOffset = 0;
	drawCommandCount = 0;
	drawCommandOffset = 0;
	reverseCullMode = false;
}

typedef TMeshBatchDrawCall<InstanceDrawData> MeshBatchDrawCall;
typedef TMeshBatchDrawCallCollection<MeshBatchDrawKey, InstanceDrawData, TMeshBatchDrawCall> MeshBatchDrawCallCollection;

struct IMeshBatchDrawCommandArray : IBatchDrawCommandArray
{
	virtual MeshBatchDrawCall* getMeshBatchDrawCall(const MeshBatchDrawKey& key) = 0;
	virtual MeshBatchDrawCall* setMeshBatchDrawCall(const MeshBatchDrawKey& key, unsigned int instanceIndex, unsigned int instanceCount = 1) = 0;
};

struct MeshBatchDrawCommandArray : IMeshBatchDrawCommandArray
{
	GPUBuffer instanceDataBuffer;
	GPUBuffer commandBuffer;

	MeshBatchDrawCallCollection meshBatchDrawCallCollection;

	MeshBatchDrawCommandArray();
	
	virtual MeshBatchDrawCall* getMeshBatchDrawCall(const MeshBatchDrawKey& key);
	virtual MeshBatchDrawCall* setMeshBatchDrawCall(const MeshBatchDrawKey& key, unsigned int instanceIndex, unsigned int instanceCount = 1);

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bindInstanceBuffer(IRenderContext& context);
	virtual IGPUBuffer* getInstanceBuffer();
	virtual IGPUBuffer* getCommandBuffer();
	virtual unsigned int getInstanceCount() const;
	virtual unsigned int getCommandCount() const;
	void clean();
	void cleanPart(MeshPart* meshPart, Material* material);
};

typedef TBatchDrawData<MeshTransformRenderData, MeshBatchDrawCommandArray> MeshBatchDrawData;
