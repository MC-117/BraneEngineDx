#include "DX11RenderTarget.h"

#ifdef VENDOR_USE_DX11

unsigned int DX11RenderTarget::nextDxFrameID = 1;
DX11RenderTarget* DX11RenderTarget::currentRenderTarget = NULL;

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
		dxContext.deviceContext->OMSetRenderTargets(dx11RTVs.size(), dx11RTVs.data(), dx11DSV);
		currentRenderTarget = this;
		return 0;
	}
	if (!desc.inited) {
		resize(desc.width, desc.height);
		desc.inited = true;
		desc.frameID = dxFrameID;
		currentRenderTarget = NULL;
	}
	if (currentRenderTarget == this)
		return desc.frameID;
	dxContext.clearSRV();
	dxContext.deviceContext->OMSetRenderTargets(dx11RTVs.size(), dx11RTVs.data(), dx11DSV);
	currentRenderTarget = this;
    return desc.frameID;
}

void DX11RenderTarget::resize(unsigned int width, unsigned int height)
{
	if (desc.inited && desc.width == width && desc.height == height)
		return;
	desc.width = width;
	desc.height = height;
	if (isDefault()) {
		dxContext.createSwapChain(width, height, desc.multisampleLevel);
		ID3D11Texture2D* screenTex = NULL;
		dxContext.swapChain->GetBuffer(0, IID_PPV_ARGS(&screenTex));
		if (dx11RTVs.size() != 1)
			dx11RTVs.resize(1, NULL);
		ID3D11RenderTargetView* rtv = dx11RTVs[0];
		if (rtv != NULL)
			rtv->Release();
		if (FAILED(dxContext.device->CreateRenderTargetView(screenTex, NULL, &rtv)))
			throw runtime_error("DX11: Create default render target view failed");
		dx11RTVs[0] = rtv;
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
		for (int i = 0; i < desc.textureList.size(); i++) {
			ID3D11RenderTargetView* rtv = dx11RTVs[i];
			if (rtv != NULL) {
				rtv->Release();
				rtv = NULL;
			}
			desc.textureList[i].texture->bind();
			DX11Texture2D* tex = (DX11Texture2D*)desc.textureList[i].texture->getVendorTexture();
			tex->desc.info.sampleCount = desc.multisampleLevel == 0 ? 1 : desc.multisampleLevel;
			if (tex->resize(width, height) == 0)
				throw runtime_error("DX11: Resize texture failed");
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
			ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
			rtvDesc.Format = tex->info.texture2DDesc.Format;
			rtvDesc.Texture2D.MipSlice = desc.textureList[i].mipLevel;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if (FAILED(dxContext.device->CreateRenderTargetView(tex->dx11Texture2D, &rtvDesc, &rtv)))
				throw runtime_error("DX11: Create render target view failed");
			dx11RTVs[i] = rtv;
		}
	}
	if (desc.depthTexure == NULL && desc.withDepthStencil) {
		if (dx11DepthTex == NULL) {
			dx11DepthTexDesc.autoGenMip = true;
			dx11DepthTexDesc.channel = 1;
			dx11DepthTexDesc.width = width;
			dx11DepthTexDesc.height = height;
			dx11DepthTexDesc.info.internalType = TIT_Depth;
			dx11DepthTexDesc.info.wrapSType = TW_Clamp;
			dx11DepthTexDesc.info.wrapTType = TW_Clamp;
			dx11DepthTexDesc.info.magFilterType = TF_Point;
			dx11DepthTexDesc.info.minFilterType = TF_Point;
			dx11DepthTexDesc.info.sampleCount = desc.multisampleLevel;
			dx11DepthTex = new DX11Texture2D(dxContext, dx11DepthTexDesc);
		}
		dx11DepthTex->resize(width, height);
		dx11DSV = dx11DepthTex->getDSV(0);
		if (dx11DSV == NULL)
			throw runtime_error("DX11: Create DSV failed");
	}
}

void DX11RenderTarget::clearColor(const Color& color)
{
	if (dx11RTVs.empty())
		return;
	auto rt = dx11RTVs.front();
	if (rt != NULL)
		dxContext.deviceContext->ClearRenderTargetView(rt, (const float*)&color);
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
			dxContext.deviceContext->ClearRenderTargetView(dx11RTVs[i], (const float*)color);
	}
}

void DX11RenderTarget::clearDepth(float depth)
{
	if (dx11DSV != NULL)
		dxContext.deviceContext->ClearDepthStencilView(dx11DSV, D3D11_CLEAR_DEPTH, depth, 0);
}

void DX11RenderTarget::clearStencil(unsigned char stencil)
{
	if (dx11DSV != NULL)
		dxContext.deviceContext->ClearDepthStencilView(dx11DSV, D3D11_CLEAR_STENCIL, 0, stencil);
}

#endif // VENDOR_USE_DX11