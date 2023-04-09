#include "DX12RenderTarget.h"

unsigned int DX12RenderTarget::nextDxFrameID = 1;
DX12RenderTarget* DX12RenderTarget::currentRenderTarget = NULL;

string DX12RenderTarget::depthBlitName = "DepthBlit";
ShaderStageDesc DX12RenderTarget::depthBlitDesc = { Compute_Shader_Stage, Shader_Default, depthBlitName };
DX12RootSignatureDesc DX12RenderTarget::depthBlitRootSignatureDesc;
DX12RootSignature* DX12RenderTarget::depthBlitRootSignature = NULL;
DX12ShaderStage* DX12RenderTarget::depthBlitCSStage = NULL;
DX12ShaderProgram* DX12RenderTarget::depthBlitCSShader = NULL;
bool DX12RenderTarget::depthBlitInit = false;

DX12RenderTarget::DX12RenderTarget(DX12Context& context, RenderTargetDesc& desc)
    : dxContext(context), IRenderTarget(desc)
{
    dxFrameID = nextDxFrameID;
    nextDxFrameID++;
}

DX12RenderTarget::~DX12RenderTarget()
{
    if (dx12DepthTex != NULL)
        delete dx12DepthTex;
}

ITexture2D* DX12RenderTarget::getInternalDepthTexture()
{
    return dx12DepthTex;
}

unsigned int DX12RenderTarget::bindFrame()
{
	if (!desc.inited) {
		resize(desc.width, desc.height);
		desc.frameID = dxFrameID;
		currentRenderTarget = NULL;
	}
	else {
		if (desc.depthOnly) {
			DX12Texture2D* dxdt = (DX12Texture2D*)desc.depthTexure->getVendorTexture();
			dx12DSV = dxdt->getDSV(0);
		}
		else if (desc.withDepthStencil) {
			if (desc.multisampleLevel > 1)
				dx12DSV = multisampleDepthTex->getDSV(0);
			else
				dx12DSV = dx12DepthTex->getDSV(0);
		}
	}
	if (currentRenderTarget == this)
		return desc.frameID;
	for (int i = 0; i < desc.textureList.size(); i++) {
		if (desc.multisampleLevel > 1) {
			RTInfo& rtInfo = desc.textureList[i];
			MSTex& mstex = multisampleTexs[i];
			dx12RTVs[i] = mstex.tex->getRTV(rtInfo.mipLevel, true);
		}
		else {
			RTInfo& rtInfo = desc.textureList[i];
			DX12Texture2D* tex = (DX12Texture2D*)rtInfo.texture->getVendorTexture();
			dx12RTVs[i] = tex->getRTV(rtInfo.mipLevel, false);
		}
	}
	dxContext.graphicContext.setRTVs(dx12RTVs.size(), dx12RTVs.data(), dx12DSV);
	dxContext.graphicContext.setSampleCount(desc.multisampleLevel);
	currentRenderTarget = this;
	return desc.frameID;
}

void DX12RenderTarget::clearBind()
{
}

void DX12RenderTarget::resize(unsigned int width, unsigned int height)
{
	if (desc.inited && desc.width == width && desc.height == height)
		return;
	desc.width = width;
	desc.height = height;

	if (desc.depthOnly) {
		if (desc.depthTexure != NULL) {
			if (desc.depthTexure->resize(width, height) == 0)
				throw runtime_error("DX12: Resize depth texture failed");
			DX12Texture2D* dxdt = (DX12Texture2D*)desc.depthTexure->getVendorTexture();
			dx12DSV = dxdt->getDSV(0);
		}
	}
	else {
		dx12RTVs.resize(desc.textureList.size());
		if (desc.multisampleLevel > 1) {
			multisampleTexs.resize(desc.textureList.size());
			for (int i = 0; i < desc.textureList.size(); i++) {
				RTInfo& rtInfo = desc.textureList[i];
				rtInfo.texture->resize(width, height);
				DX12Texture2D* tex = (DX12Texture2D*)rtInfo.texture->getVendorTexture();
				MSTex& mstex = multisampleTexs[i];
				if (mstex.tex == NULL) {
					mstex.desc = tex->desc;
					mstex.desc.data = NULL;
					mstex.desc.mipLevel = 1;
					mstex.desc.textureHandle = 0;
					mstex.tex = new DX12Texture2D(dxContext, mstex.desc);
				}
				mstex.desc.info.sampleCount = desc.multisampleLevel;
				if (tex->resize(width, height) == 0)
					throw runtime_error("DX12: Resize texture failed");
				if (mstex.tex->resize(width, height) == 0)
					throw runtime_error("DX12: Resize ms texture failed");

				dx12RTVs[i] = mstex.tex->getRTV(rtInfo.mipLevel, true);
			}
		}
		else {
			for (int i = 0; i < desc.textureList.size(); i++) {
				RTInfo& rtInfo = desc.textureList[i];
				rtInfo.texture->resize(width, height);
				DX12Texture2D* tex = (DX12Texture2D*)rtInfo.texture->getVendorTexture();
				tex->desc.info.sampleCount = desc.multisampleLevel == 0 ? 1 : desc.multisampleLevel;
				if (tex->resize(width, height) == 0)
					throw runtime_error("DX12: Resize texture failed");
				dx12RTVs[i] = tex->getRTV(rtInfo.mipLevel, false);
			}
		}
	}
	if (desc.depthTexure == NULL && desc.withDepthStencil) {
		dx12DepthTexDesc.autoGenMip = true;
		dx12DepthTexDesc.channel = 1;
		dx12DepthTexDesc.width = width;
		dx12DepthTexDesc.height = height;
		dx12DepthTexDesc.info.internalType = desc.multisampleLevel > 1 ? TIT_R32_F : TIT_D32_F;
		dx12DepthTexDesc.info.wrapSType = TW_Clamp;
		dx12DepthTexDesc.info.wrapTType = TW_Clamp;
		dx12DepthTexDesc.info.magFilterType = TF_Point;
		dx12DepthTexDesc.info.minFilterType = TF_Point;
		dx12DepthTexDesc.info.sampleCount = 1;
		if (dx12DepthTex == NULL)
			dx12DepthTex = new DX12Texture2D(dxContext, dx12DepthTexDesc);
		else
			dx12DepthTex->release();
		dx12DepthTex->resize(width, height);
		if (desc.multisampleLevel > 1) {
			multisampleDepthTexDesc.autoGenMip = true;
			multisampleDepthTexDesc.channel = 1;
			multisampleDepthTexDesc.width = width;
			multisampleDepthTexDesc.height = height;
			multisampleDepthTexDesc.info.internalType = TIT_D32_F;
			multisampleDepthTexDesc.info.wrapSType = TW_Clamp;
			multisampleDepthTexDesc.info.wrapTType = TW_Clamp;
			multisampleDepthTexDesc.info.magFilterType = TF_Point;
			multisampleDepthTexDesc.info.minFilterType = TF_Point;
			multisampleDepthTexDesc.info.sampleCount = desc.multisampleLevel;
			if (multisampleDepthTex == NULL)
				multisampleDepthTex = new DX12Texture2D(dxContext, multisampleDepthTexDesc);
			else
				multisampleDepthTex->release();
			multisampleDepthTex->resize(width, height);
			dx12DSV = multisampleDepthTex->getDSV(0);
			if (!dx12DSV.isActive())
				throw runtime_error("DX12: Create DSV failed");
		}
		else {
			dx12DSV = dx12DepthTex->getDSV(0);
			if (!dx12DSV.isActive())
				throw runtime_error("DX12: Create DSV failed");
		}
	}
	desc.inited = true;
}

void DX12RenderTarget::SetMultisampleFrame()
{
	resize(desc.width, desc.height);
	if (desc.multisampleLevel > 1) {
		for (int i = 0; i < desc.textureList.size(); i++) {
			DX12Texture2D* tex = (DX12Texture2D*)desc.textureList[i].texture->getVendorTexture();
			desc.textureList[i].texture->bind();
			dxContext.graphicContext.resolveMS(*tex->dx12Texture2D,
				*multisampleTexs[i].tex->dx12Texture2D, tex->info.texture2DDesc.Format);
		}
		if (desc.withDepthStencil) {
			initDepthBlit();
			auto cmdlst = dxContext.graphicContext.getCommandList()->get(false);
			multisampleDepthTex->dx12Texture2D->transitionBarrier(cmdlst,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | 
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			dx12DepthTex->dx12Texture2D->transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			depthBlitCSShader->bind();
			currentRenderTarget = NULL;
			depthBlitRootSignature->setTexture(7, multisampleDepthTex->getSRV());
			depthBlitRootSignature->setImage(0, dx12DepthTex->getUAV(0));
			dxContext.graphicContext.setComputeRootSignature(depthBlitRootSignature);
			depthBlitCSShader->dispatchCompute(ceil(desc.width / 16.0f), ceil(desc.height / 16.0f), 1);
			dx12DepthTex->dx12Texture2D->transitionBarrier(cmdlst,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
	}
}

void DX12RenderTarget::clearColor(const Color& color)
{
	/*if (isDefault()) {
		dxContext.graphicContext.clearRTV(dxContext.backBufferRTVDescHeap[0], color,
			{ 0, 0, (UINT)desc.width, (UINT)desc.height });
	}
	else */{
		if (dx12RTVs.empty())
			return;
		auto& rtv = dx12RTVs.front();
		if (rtv.isActive())
			dxContext.graphicContext.clearRTV(rtv, color, { 0, 0, (UINT)desc.width, (UINT)desc.height });
	}
}

void DX12RenderTarget::clearColors(const vector<Color>& colors)
{
	if (colors.empty())
		return;
	/*if (isDefault()) {
		dxContext.graphicContext.clearRTV(dxContext.backBufferRTVDescHeap[0], colors[0],
			{ 0, 0, (UINT)desc.width, (UINT)desc.height });
	}
	else */for (int i = 0; i < dx12RTVs.size(); i++) {
		Color color;
		if (i < colors.size())
			color = colors[i];
		else
			color = colors.back();
		if (dx12RTVs[i].isActive())
			dxContext.graphicContext.clearRTV(dx12RTVs[i], color, { 0, 0, (UINT)desc.width, (UINT)desc.height });
	}
}

void DX12RenderTarget::clearDepth(float depth)
{
	if (dx12DSV.isActive())
		dxContext.graphicContext.clearDSV(dx12DSV, D3D12_CLEAR_FLAG_DEPTH, depth, 0, { 0, 0, (UINT)desc.width, (UINT)desc.height });
}

void DX12RenderTarget::clearStencil(unsigned char stencil)
{
	if (dx12DSV.isActive())
		dxContext.graphicContext.clearDSV(dx12DSV, D3D12_CLEAR_FLAG_STENCIL, 0, stencil, { 0, 0, (UINT)desc.width, (UINT)desc.height });
}

void DX12RenderTarget::initDepthBlit()
{
	if (depthBlitInit)
		return;
	string code = "\
		Texture2DMS<float> msDepth : register(t7);\n\
		RWTexture2D<float> depth : register(u0);\n\
		[numthreads(16, 16, 1)]\n\
		void main(uint3 gid : SV_DispatchThreadID)\n\
		{\n\
			uint2 size;\n\
			uint samples;\n\
			msDepth.GetDimensions(size.x, size.y, samples);\n\
			if (gid.x < size.x && gid.y < size.y)\n\
			{\n\
				float d = 1;\n\
				for (int i = 0; i < samples; i++)\n\
					d = min(d, msDepth.Load(gid.xy, i).x);\n\
				depth[gid.xy] = d;\n\
			}\n\
		}";
	string error;
	depthBlitCSStage = new DX12ShaderStage(dxContext, depthBlitDesc);
	if (depthBlitCSStage->compile(ShaderMacroSet(), code, error) == 0) {
		cout << error;
		//throw runtime_error(error);
	}
	depthBlitCSShader = new DX12ShaderProgram(dxContext);
	depthBlitCSShader->setMeshStage(*depthBlitCSStage);

	depthBlitRootSignatureDesc.addImage(0);
	depthBlitRootSignatureDesc.addTexture(7);

	depthBlitRootSignature = new DX12RootSignature();
	depthBlitRootSignature->init(dxContext.device);
	depthBlitRootSignature->setDesc(depthBlitRootSignatureDesc);

	depthBlitInit = true;
}
