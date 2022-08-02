#pragma once
#include "../../Core/IRenderTarget.h"

#ifdef VENDOR_USE_DX12

#include "DX12Context.h"
#include "DX12Texture2D.h"
#include "DX12Shader.h"

class DX12RenderTarget : public IRenderTarget
{
public:
	static unsigned int nextDxFrameID;
	static DX12RenderTarget* currentRenderTarget;
	unsigned int dxFrameID = 0;
	DX12Context& dxContext;
	vector<DX12Descriptor> dx12RTVs;
	DX12Descriptor dx12DSV;
	Texture2DDesc dx12DepthTexDesc;
	DX12Texture2D* dx12DepthTex = NULL;

	Texture2DDesc multisampleDepthTexDesc;
	DX12Texture2D* multisampleDepthTex = NULL;

	static string depthBlitName;
	static ShaderStageDesc depthBlitDesc;
	static DX12RootSignatureDesc depthBlitRootSignatureDesc;
	static DX12RootSignature* depthBlitRootSignature;
	static DX12ShaderStage* depthBlitCSStage;
	static DX12ShaderProgram* depthBlitCSShader;
	static bool depthBlitInit;

	struct MSTex
	{
		Texture2DDesc desc;
		DX12Texture2D* tex = NULL;
	};

	vector<MSTex> multisampleTexs;

	DX12RenderTarget(DX12Context& context, RenderTargetDesc& desc);
	virtual ~DX12RenderTarget();

	virtual ITexture2D* getInternalDepthTexture();
	virtual unsigned int bindFrame();
	virtual void clearBind();
	virtual void resize(unsigned int width, unsigned int height);
	virtual void SetMultisampleFrame();

	virtual void clearColor(const Color& color);
	virtual void clearColors(const vector<Color>& colors);
	virtual void clearDepth(float depth);
	virtual void clearStencil(unsigned char stencil);

	void initDepthBlit();
};

#endif // VENDOR_USE_DX12
