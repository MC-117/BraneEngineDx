#include "DX11RenderTarget.h"

#ifdef VENDOR_USE_DX11

unsigned int DX11RenderTarget::nextDxFrameID = 1;
DX11RenderTarget* DX11RenderTarget::currentRenderTarget = NULL;

string DX11RenderTarget::depthBlitName = "DepthBlit";
ShaderStageDesc DX11RenderTarget::depthBlitDesc = { Compute_Shader_Stage, Shader_Default, depthBlitName };
DX11ShaderStage* DX11RenderTarget::depthBlitCSStage = NULL;
DX11ShaderProgram* DX11RenderTarget::depthBlitCSShader = NULL;
bool DX11RenderTarget::depthBlitInit = false;

DX11RenderTarget::DX11RenderTarget(DX11Context& context, RenderTargetDesc& desc)
    : dxContext(context), IRenderTarget(desc)
{
	dxFrameID = nextDxFrameID;
	nextDxFrameID++;
}

DX11RenderTarget::~DX11RenderTarget()
{
	if (dx11DepthTex != NULL)
		delete dx11DepthTex;
}

ITexture2D* DX11RenderTarget::getInternalDepthTexture()
{
	return dx11DepthTex;
}

unsigned int DX11RenderTarget::bindFrame()
{
	if (isDefault()) {
		desc.inited = true;
		dxContext.clearSRV();
		dxContext.deviceContext->OMSetRenderTargets(1, dxContext.backBufferRTV[0].GetAddressOf(), NULL);
		currentRenderTarget = this;
		return 0;
	}
	if (!desc.inited) {
		resize(desc.width, desc.height);
		currentRenderTarget = NULL;
	}
	else {
		if (desc.depthOnly) {
			DX11Texture2D* dxdt = (DX11Texture2D*)desc.depthTexure->getVendorTexture();
			dx11DSV = dxdt->getDSV(0);
		}
		else if (desc.withDepthStencil) {
			if (desc.multisampleLevel > 1)
				dx11DSV = multisampleDepthTex->getDSV(0);
			else
				dx11DSV = dx11DepthTex->getDSV(0);
		}
	}
	if (currentRenderTarget == this)
		return desc.frameID;
	for (int i = 0; i < desc.textureList.size(); i++) {
		RTInfo& rtInfo = desc.textureList[i];
		RTOption option;
		option.mipLevel = rtInfo.mipLevel;
		option.arrayBase = rtInfo.arrayBase;
		option.arrayCount = rtInfo.arrayCount;
		option.multisample = desc.multisampleLevel > 1;
		if (option.multisample) {
			MSTex& mstex = multisampleTexs[i];
			option.multisample = true;
			dx11RTVs[i] = mstex.tex->getRTV(option);
		}
		else {
			DX11Texture2D* tex = (DX11Texture2D*)rtInfo.texture->getVendorTexture();
			dx11RTVs[i] = tex->getRTV(option);
		}
	}
	dxContext.clearSRV();
	dxContext.clearUAV();
	dxContext.deviceContext->OMSetRenderTargets(dx11RTVs.size(),
		(ID3D11RenderTargetView* const*)dx11RTVs.data(), dx11DSV.Get());
	currentRenderTarget = this;
    return desc.frameID;
}

void DX11RenderTarget::clearBind()
{
	dxContext.clearRTV();
	dxContext.clearUAV();
}

void DX11RenderTarget::resize(unsigned int width, unsigned int height)
{
	bool sizeChanged = desc.width != width || desc.height != height;
	if (desc.inited && !sizeChanged)
		return;
	desc.width = width;
	desc.height = height;
	if (isDefault()) {
		if (sizeChanged)
			dxContext.createSwapChain(width, height, desc.multisampleLevel);
	}
	else if (desc.depthOnly) {
		if (desc.depthTexure != NULL) {
			if (desc.depthTexure->resize(width, height) == 0)
				throw runtime_error("DX11: Resize depth texture failed");
			DX11Texture2D* dxdt = (DX11Texture2D*)desc.depthTexure->getVendorTexture();
			dx11DSV = dxdt->getDSV(0);
		}
	}
	else {
		dx11RTVs.resize(desc.textureList.size());
		if (desc.multisampleLevel > 1) {
			multisampleTexs.resize(desc.textureList.size());
			for (int i = 0; i < desc.textureList.size(); i++) {
				RTInfo& rtInfo = desc.textureList[i];
				rtInfo.texture->resize(width, height);
				DX11Texture2D* tex = (DX11Texture2D*)rtInfo.texture->getVendorTexture();
				MSTex& mstex = multisampleTexs[i];
				if (mstex.tex == NULL) {
					mstex.desc = tex->desc;
					mstex.desc.data = NULL;
					mstex.desc.mipLevel = 1;
					mstex.desc.textureHandle = 0;
					mstex.tex = new DX11Texture2D(dxContext, mstex.desc);
				}
				mstex.desc.info.sampleCount = desc.multisampleLevel;
				if (tex->resize(width, height) == 0)
					throw runtime_error("DX11: Resize texture failed");
				if (mstex.tex->resize(width, height) == 0)
					throw runtime_error("DX11: Resize ms texture failed");
				RTOption option;
				option.mipLevel = rtInfo.mipLevel;
				option.arrayBase = rtInfo.arrayBase;
				option.arrayCount = rtInfo.arrayCount;
				option.multisample = true;
				dx11RTVs[i] = mstex.tex->getRTV(option);
			}
		}
		else {
			for (int i = 0; i < desc.textureList.size(); i++) {
				RTInfo& rtInfo = desc.textureList[i];
				rtInfo.texture->resize(width, height);
				DX11Texture2D* tex = (DX11Texture2D*)rtInfo.texture->getVendorTexture();
				tex->desc.info.sampleCount = desc.multisampleLevel == 0 ? 1 : desc.multisampleLevel;
				if (tex->resize(width, height) == 0)
					throw runtime_error("DX11: Resize texture failed");
				RTOption option;
				option.mipLevel = rtInfo.mipLevel;
				option.arrayBase = rtInfo.arrayBase;
				option.arrayCount = rtInfo.arrayCount;
				option.multisample = false;
				dx11RTVs[i] = tex->getRTV(option);
			}
		}
	}
	if (desc.depthTexure == NULL && desc.withDepthStencil) {
		dx11DepthTexDesc.autoGenMip = true;
		dx11DepthTexDesc.channel = 1;
		dx11DepthTexDesc.width = width;
		dx11DepthTexDesc.height = height;
		dx11DepthTexDesc.info.internalType = desc.multisampleLevel > 1 ? TIT_R32_F : TIT_D32_F;
		dx11DepthTexDesc.info.wrapSType = TW_Clamp;
		dx11DepthTexDesc.info.wrapTType = TW_Clamp;
		dx11DepthTexDesc.info.magFilterType = TF_Point;
		dx11DepthTexDesc.info.minFilterType = TF_Point;
		dx11DepthTexDesc.info.sampleCount = 1;
		if (dx11DepthTex == NULL)
			dx11DepthTex = new DX11Texture2D(dxContext, dx11DepthTexDesc);
		else
			dx11DepthTex->release();
		dx11DepthTex->resize(width, height);
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
				multisampleDepthTex = new DX11Texture2D(dxContext, multisampleDepthTexDesc);
			else
				multisampleDepthTex->release();
			multisampleDepthTex->resize(width, height);
			dx11DSV = multisampleDepthTex->getDSV(0);
			if (dx11DSV == NULL)
				throw runtime_error("DX11: Create DSV failed");
		}
		else {
			dx11DSV = dx11DepthTex->getDSV(0);
			if (dx11DSV == NULL)
				throw runtime_error("DX11: Create DSV failed");
		}
	}
	desc.inited = true;
	desc.frameID = dxFrameID;
}

void DX11RenderTarget::SetMultisampleFrame()
{
	if (desc.multisampleLevel > 1) {
		for (int i = 0; i < desc.textureList.size(); i++) {
			DX11Texture2D* tex = (DX11Texture2D*)desc.textureList[i].texture->getVendorTexture();
			desc.textureList[i].texture->bind();
			dxContext.deviceContext->ResolveSubresource(tex->dx11Texture2D.Get(), 0,
				multisampleTexs[i].tex->dx11Texture2D.Get(), 0, tex->info.texture2DDesc.Format);
		}
		if (desc.withDepthStencil) {
			/*dxContext.deviceContext->ResolveSubresource(dx11DepthTex->dx11Texture2D, 0,
				multisampleDepthTex->dx11Texture2D, 0, DXGI_FORMAT_D32_FLOAT);*/
			//initDepthBlit();
			depthBlitCSShader->bind();
			auto srv = multisampleDepthTex->getSRV();
			RWOption rwOption;
			rwOption.mipLevel = 0;
			auto uav = dx11DepthTex->getUAV(rwOption);
			dxContext.clearSRV();
			dxContext.clearUAV();
			dxContext.clearRTV();
			currentRenderTarget = NULL;
			dxContext.deviceContext->CSSetShaderResources(7, 1, srv.GetAddressOf());
			dxContext.deviceContext->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), NULL);
			depthBlitCSShader->dispatchCompute(ceil(desc.width / 16.0f), ceil(desc.height / 16.0f), 1);
			dxContext.clearUAV();
		}
	}
}

void DX11RenderTarget::clearColor(const Color& color)
{
	if (dx11RTVs.empty())
		return;
	auto rt = dx11RTVs.front();
	if (rt != NULL)
		dxContext.deviceContext->ClearRenderTargetView(rt.Get(), (const float*)&color);
}

void DX11RenderTarget::clearColors(const vector<Color>& colors)
{
	if (colors.empty())
		return;
	for (int i = 0; i < dx11RTVs.size(); i++) {
		const Color* color;
		if (i < colors.size())
			color = &colors[i];
		else
			color = &colors.back();
		if (dx11RTVs[i] != NULL)
			dxContext.deviceContext->ClearRenderTargetView(dx11RTVs[i].Get(), (const float*)color);
	}
}

void DX11RenderTarget::clearDepth(float depth)
{
	if (dx11DSV != NULL)
		dxContext.deviceContext->ClearDepthStencilView(dx11DSV.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

void DX11RenderTarget::clearStencil(unsigned char stencil)
{
	if (dx11DSV != NULL)
		dxContext.deviceContext->ClearDepthStencilView(dx11DSV.Get(), D3D11_CLEAR_STENCIL, 0, stencil);
}

void DX11RenderTarget::initDepthBlit(DX11Context& dxContext)
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
	depthBlitCSStage = new DX11ShaderStage(dxContext, depthBlitDesc);
	if (depthBlitCSStage->compile(code, error) == 0) {
		cout << error;
		throw runtime_error(error);
	}
	depthBlitCSShader = new DX11ShaderProgram(dxContext);
	depthBlitCSShader->setMeshStage(*depthBlitCSStage);
	depthBlitInit = true;
}

#endif // VENDOR_USE_DX11