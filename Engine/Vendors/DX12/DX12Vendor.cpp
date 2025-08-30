#include "DX12Vendor.h"
#include "../imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#ifdef VENDOR_USE_DX12

REG_VENDOR_DEC(DX12Vendor);
REG_VENDOR_IMP(DX12Vendor, "DX12");

DX12Vendor::DX12Vendor() : StaticMeshData(dxContext)
{
	name = "DX12";
}

DX12Vendor::~DX12Vendor()
{
}

bool DX12Vendor::windowSetup(EngineConfig& config, WindowContext& context)
{
	return true;
}

bool DX12Vendor::setup(const EngineConfig& config, const WindowContext& context)
{
	dxContext.setHWnd(context.hwnd);
	if (!dxContext.createDevice(context.screenSize.x, context.screenSize.y))
		return false;
	return true;
}

bool DX12Vendor::imGuiInit(const EngineConfig& config, const WindowContext& context)
{
	if (!ImGui_ImplWin32_Init(context.hwnd))
		return false;
	if (!ImGui_ImplDX12_Init(dxContext.device.Get(), dxContext.backBufferCount,
		DXGI_FORMAT_B8G8R8A8_UNORM))
		return false;
	return true;
}

bool DX12Vendor::imGuiNewFrame(const EngineConfig& config, const WindowContext& context)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	return true;
}

bool DX12Vendor::imGuiDrawFrame(const EngineConfig& config, const WindowContext& context)
{
	DX12CommandList& dxCmdlst = *dxContext.graphicContext.getCommandList();
	ComPtr<ID3D12GraphicsCommandList> cmdlst = dxCmdlst.get(true);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdlst.Get());

	dxContext.backBuffer[dxContext.activeBackBufferIndex].transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_PRESENT);

	dxContext.graphicContext.push();
	return true;
}

bool DX12Vendor::imGuiShutdown(const EngineConfig& config, const WindowContext& context)
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	return true;
}

bool DX12Vendor::clean(const EngineConfig& config, const WindowContext& context)
{
	dxContext.cleanupDevice();
	return true;
}

bool DX12Vendor::guiOnlyRender(const Color& clearColor)
{
	return true;
}

bool DX12Vendor::resizeWindow(const EngineConfig& config, const WindowContext& context, unsigned int width, unsigned int height)
{
	return true;
}

bool DX12Vendor::toggleFullscreen(const EngineConfig& config, const WindowContext& context, bool fullscreen)
{
	return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT DX12Vendor::wndProcFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

uint64_t DX12Vendor::getGPUFrequency()
{
	return dxContext.gpuFrequency;
}

IRenderContext* DX12Vendor::getDefaultRenderContext()
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

IRenderContext* DX12Vendor::newRenderContext()
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

IDeviceSurface* DX12Vendor::newDeviceSurface(DeviceSurfaceDesc& desc)
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

ITexture2D* DX12Vendor::newTexture2D(TextureDesc& desc)
{
	return new DX12Texture2D(dxContext, desc);
}

ITexture3D* DX12Vendor::newTexture3D(TextureDesc& desc)
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

ShaderStage* DX12Vendor::newShaderStage(const ShaderStageDesc& desc)
{
	return new DX12ShaderStage(dxContext, desc);
}

ShaderProgram* DX12Vendor::newShaderProgram()
{
	return new DX12ShaderProgram(dxContext);
}

IMaterial* DX12Vendor::newMaterial(MaterialDesc& desc)
{
	return new DX12Material(dxContext, desc);
}

IRenderTarget* DX12Vendor::newRenderTarget(RenderTargetDesc& desc)
{
	return new DX12RenderTarget(dxContext, desc);
}

IGPUBuffer* DX12Vendor::newGPUBuffer(GPUBufferDesc& desc)
{
	return new DX12GPUBuffer(dxContext, desc);
}

IGPUQuery* DX12Vendor::newGPUQuery(GPUQueryDesc& desc)
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

GraphicsPipelineState* DX12Vendor::fetchGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

ComputePipelineState* DX12Vendor::fetchComputePipelineState(const ComputePipelineStateDesc& desc)
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

MeshPartDesc DX12Vendor::newMeshPart(unsigned int vertCount, unsigned int elementCount)
{
	DX12MeshData* data = NULL;
	if (StaticMeshData.isGenerated()) {
		for (auto b = MeshDataCollection.begin(), e = MeshDataCollection.end(); b != e; b++) {
			if ((*b)->type == MT_Mesh && !(*b)->isGenerated()) {
				data = (DX12MeshData*)*b;
				break;
			}
		}
	}
	else
		data = &StaticMeshData;
	if (data == NULL) {
		data = new DX12MeshData(dxContext);
		MeshDataCollection.insert(data);
	}
	MeshPartDesc desc = { data, (unsigned int)data->vertices.size(), vertCount, (unsigned int)data->elements.size(), elementCount };
	data->vertices.resize(data->vertices.size() + vertCount);
	data->uvs.resize(data->uvs.size() + vertCount);
	data->normals.resize(data->normals.size() + vertCount);
	data->elements.resize(data->elements.size() + elementCount);
	return desc;
}

SkeletonMeshPartDesc DX12Vendor::newSkeletonMeshPart(unsigned int vertCount, unsigned int elementCount, unsigned int boneCount, unsigned int morphVertCount, unsigned int morphCount)
{
	SkeletonMeshData* data = NULL;
	data = new DX12SkeletonMeshData(dxContext);
	MeshDataCollection.insert(data);
	SkeletonMeshPartDesc desc = { data, (unsigned int)data->vertices.size(), vertCount, (unsigned int)data->elements.size(), elementCount };
	if (morphCount > 0)
		desc.morphMeshData = &data->morphMeshData;
	data->vertices.resize(data->vertices.size() + vertCount);
	data->uvs.resize(data->uvs.size() + vertCount);
	data->normals.resize(data->normals.size() + vertCount);
	data->boneIndexes.resize(data->boneIndexes.size() + vertCount, Vector4u::Zero());
	data->weights.resize(data->weights.size() + vertCount, Vector4f::Zero());
	data->elements.resize(data->elements.size() + elementCount);
	data->boneCount = boneCount;
	data->morphMeshData.init(morphVertCount, morphCount);
	return desc;
}

IRenderExecution* DX12Vendor::newRenderExecution()
{
	return new DX12RenderExecution(dxContext);
}

void DX12Vendor::setRenderPreState(DepthStencilMode depthStencilMode, uint8_t stencilValue)
{
	dxContext.graphicContext.setBlendFlags(DX12BlendFlags::None);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::Test | (uint8_t)DX12DepthFlags::Write);
}

void DX12Vendor::setRenderGeomtryState(DepthStencilMode depthStencilMode, uint8_t stencilValue)
{
	dxContext.graphicContext.setBlendFlags(DX12BlendFlags::Write);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::Test | (uint8_t)DX12DepthFlags::Write);
}

void DX12Vendor::setRenderOpaqueState(DepthStencilMode depthStencilMode, uint8_t stencilValue)
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::Test | (uint8_t)DX12DepthFlags::Write);
}

void DX12Vendor::setRenderAlphaState(DepthStencilMode depthStencilMode, uint8_t stencilValue)
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::AlphaTest);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::Test | (uint8_t)DX12DepthFlags::Write);
}

void DX12Vendor::setRenderTransparentState(DepthStencilMode depthStencilMode, uint8_t stencilValue)
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::Blend);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::Test);
}

void DX12Vendor::setRenderOverlayState()
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::Blend);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::None);
}

void DX12Vendor::setRenderPostState()
{
	dxContext.graphicContext.setBlendFlags(DX12BlendFlags::None);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::None);
}

void DX12Vendor::setRenderPostAddState()
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::BlendAdd);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::None);
}

void DX12Vendor::setRenderPostPremultiplyAlphaState()
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::BlendPremultiplyAlpha);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::None);
}

void DX12Vendor::setRenderPostMultiplyState()
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::BlendMultiply);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::None);
}

void DX12Vendor::setRenderPostMaskState()
{
	dxContext.graphicContext.setBlendFlags((uint8_t)DX12BlendFlags::Write | (uint8_t)DX12BlendFlags::BlendMask);
	dxContext.graphicContext.setDepthFlags((uint8_t)DX12DepthFlags::None);
}

void DX12Vendor::setCullState(CullType type)
{
	dxContext.graphicContext.setCullType((DX12CullType)type);
}

void DX12Vendor::setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	dxContext.graphicContext.setViewport({ x, y, w, h });
}

void DX12Vendor::setMeshDrawContext()
{
	dxContext.graphicContext.setTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.graphicContext.setInputLayout(DX12InputLayoutType::Mesh);
}

void DX12Vendor::setSkeletonMeshDrawContext()
{
	dxContext.graphicContext.setTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.graphicContext.setInputLayout(DX12InputLayoutType::SkeletonMesh);
}

void DX12Vendor::setTerrainDrawContext()
{
	dxContext.graphicContext.setTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	dxContext.graphicContext.setInputLayout(DX12InputLayoutType::Terrain);
}

void DX12Vendor::meshDrawCall(const MeshPartDesc& mesh)
{
	if (mesh.meshData != NULL) {
		mesh.meshData->bindShape();
	}
	dxContext.graphicContext.drawIndexed(mesh.elementCount, mesh.elementFirst, mesh.vertexFirst);
}

void DX12Vendor::postProcessCall()
{
	dxContext.graphicContext.setMeshData(DX12GraphicMeshData());
	dxContext.graphicContext.setTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.graphicContext.draw(4, 0);
	dxContext.graphicContext.setTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DX12Vendor::readBackTexture(ITexture* texture, void* data)
{
	DX12Texture2D* dxTexture = dynamic_cast<DX12Texture2D*>(texture);
	if (dxTexture == NULL || data == NULL)
		return;
	TextureDesc desc = dxTexture->desc;
	desc.info.cpuAccessFlag = CAF_Read;
	desc.mipLevel = 1;
	desc.autoGenMip = false;
	DX12Texture2D* buffer = (DX12Texture2D*)newTexture2D(desc);
	dxTexture->bind();
	buffer->bind();
	dxContext.graphicContext.copyBuffer(*dxTexture->dx12Texture2D, 0,
		*buffer->dx12Texture2D, 0, dxTexture->dx12Texture2D->getSize());
	dxContext.executeCommandQueue();
	dxContext.waitForGPU();
	void* srcData = buffer->dx12Texture2D->map();
	int size = desc.width * desc.height * desc.channel * sizeof(char);
	memcpy_s(data, size, srcData, size);
}

DX12Context& DX12Vendor::getDXContext()
{
	return dxContext;
}

#endif  // VENDOR_USE_DX12