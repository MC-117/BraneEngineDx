#pragma once

#include "DX12PipelineState.h"
#include "DX12CommandListDispatcher.h"
#include "DX12BufferPool.h"
#include "DX12DescriptorHeap.h"
#include "DX12RootSignature.h"

struct DX12GraphicMeshData
{
	int startSlot = 0;
	int slotCount = 0;
	DX12InputLayoutType layoutType;
	DX12Buffer* vertex = NULL;
	DX12Buffer* uv = NULL;
	DX12Buffer* normal = NULL;
	DX12Buffer* bone = NULL;
	DX12Buffer* weight = NULL;
	DX12Buffer* element = NULL;
};

struct DX12VertexBufferView
{
	DX12Buffer* buffer = NULL;
	unsigned int stride = 0;
};

class DX12GraphicContext
{
public:
	DX12GraphicContext() = default;
	void init(ComPtr<ID3D12Device> device);
	bool executeCommandList(ComPtr<ID3D12CommandQueue> queue);
	bool executeComputeCommandList(ComPtr<ID3D12CommandQueue> queue);
	void reset(int threadID);

	void beginRender(int threadID, D3D12_COMMAND_LIST_TYPE type);
	void endRender();

	DX12CommandList* getCommandList();
	DX12CommandList& getImGuiCommandList();

	void transitionBarrier(DX12Buffer& buffer, D3D12_SUBRESOURCE_DATA* data, D3D12_RESOURCE_STATES transitionState, D3D12_RESOURCE_BARRIER_FLAGS flags);

	void setRootSignature(DX12RootSignature* rootSignature);
	void setComputeRootSignature(DX12RootSignature* rootSignature);

	void setInputLayout(DX12InputLayoutType type);

	void setCullType(DX12CullType type);
	void setBlendFlags(Enum<DX12BlendFlags> flags);
	void setDepthFlags(Enum<DX12DepthFlags> flags);

	void setRTVs(int count, DX12Descriptor* rtvs, DX12Descriptor& dsv);
	void setSampleCount(int count);

	DX12Descriptor getRTV(int index);

	void setGraphicShader(const DX12ShaderProgramData& data);

	void setComputeShader(const DX12ShaderProgramData& data);

	void bindVertexBuffer(int slot, DX12SubBuffer* buffer);
	void bindIndexBuffer(DX12SubBuffer* buffer);
	void bindDefaultCBV(int slot, const DX12ResourceView& cbv);
	void bindDefaultSRV(int slot, const DX12ResourceView& srv);

	void setMeshData(const DX12GraphicMeshData& meshData);
	void setTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

	// { x, y, width, height }
	void setViewport(const Vector4u& view);

	// { left, top, right, bottom }
	void clearRTV(DX12Descriptor rtv, const Color& color, const Vector4u& rect);
	// { left, top, right, bottom }
	void clearDSV(DX12Descriptor rtv, D3D12_CLEAR_FLAGS flags, float depth, uint8_t stencil, const Vector4u& rect);

	void resolveMS(DX12Buffer& dst, DX12Buffer& src, DXGI_FORMAT format);

	void copyBuffer(DX12SubBuffer& dst, unsigned int dstOff, DX12SubBuffer& src, unsigned int srcOff, unsigned int size);
	void copyBuffer(DX12Buffer& dst, unsigned int dstOff, DX12SubBuffer& src, unsigned int srcOff, unsigned int size);
	void copyBuffer(DX12SubBuffer& dst, unsigned int dstOff, DX12Buffer& src, unsigned int srcOff, unsigned int size);
	void copyBuffer(DX12Buffer& dst, unsigned int dstOff, DX12Buffer& src, unsigned int srcOff, unsigned int size);

	void dispatch(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	void push();
	void draw(unsigned int vertexCount, unsigned int vertexBase);
	void drawIndexed(unsigned int indexCount, unsigned int indexBase, unsigned int vertexBase);
	void drawIndirect();
	void drawInstanced(unsigned int vertexCount, unsigned int instanceCount,
		unsigned int vertexBase, unsigned int instanceBase);
	void drawIndexedInstanced(unsigned int indexCount, unsigned int instanceCount,
		unsigned int indexBase, unsigned int vertexBase, unsigned int instanceBase);
	void drawInstancedIndirect();
protected:
	ComPtr<ID3D12Device> device;

	DX12RootSignature* rootSignature = NULL;

	map<int, DX12SubBuffer*> defaultVBufferBindings;
	map<int, DX12ResourceView> defaultCBufferBindings;
	map<int, DX12ResourceView> defaultTBufferBindings;

	DX12SubBuffer* defaultIBuffer = NULL;

	DX12GraphicMeshData meshData;
	D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	vector<DX12Descriptor> renderTargets;
	DX12Descriptor depthStencilTarget;
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = { 0 };
	int sampleCount = 0;

	Vector4u viewportRect;

	DX12CommandListDispatcher commandListDispatcher;
	DX12CommandList* commandList = NULL;
	DX12CommandList* computeCommandList = NULL;

	DX12CommandList* imGuiCommandList = NULL;

	map<DX12GraphicPipelineState::ID, DX12GraphicPipelineState> graphicPipelineStateMap;
	map<DX12ComputePipelineState::ID, DX12ComputePipelineState> computePipelineStateMap;

	DX12GraphicPipelineState graphicPipelineState;
	DX12ComputePipelineState computePipelineState;

	set<DX12CommandList*> commandListSet;
	set<DX12CommandList*> computeCommandListSet;

	bool validateGraphic();
	bool validateCompute();
	void flushGraphic();
	void flushCompute();
};

