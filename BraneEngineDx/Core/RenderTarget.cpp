#include "RenderTarget.h"

RenderTarget RenderTarget::defaultRenderTarget;
unsigned int RenderTarget::currentFbo = 0;

RenderTarget::RenderTarget()
{
	if (isDefault()) {
		desc.channel = 4;
		desc.withDepthStencil = true;
		desc.multisampleLevel = 1;
	}
}

RenderTarget::RenderTarget(int width, int height, int channel, bool withDepthStencil, int multisampleLevel)
{
	desc.width = width;
	desc.height = height;
	desc.channel = channel;
	desc.withDepthStencil = withDepthStencil;
	desc.multisampleLevel = multisampleLevel;
}

RenderTarget::RenderTarget(int width, int height, Texture & depthTexture)
{
	desc.width = width;
	desc.height = height;
	desc.channel = 1;
	desc.depthTexure = &depthTexture;
	desc.depthOnly = true;
}

RenderTarget::~RenderTarget()
{
	if (vendorRenderTarget != 0)
		delete vendorRenderTarget;
}

bool RenderTarget::isValid()
{
	return desc.frameID != 0;
}

bool RenderTarget::isDefault()
{
	return this == &defaultRenderTarget;
}

bool RenderTarget::isDepthOnly()
{
	return desc.depthOnly;
}

unsigned int RenderTarget::bindFrame()
{
	newVendorRenderTarget();
	vendorRenderTarget->bindFrame();
	return isDefault() ? 0 : desc.frameID;
}

void RenderTarget::addTexture(const string & name, Texture & texture, unsigned int mipLevel)
{
	auto iter = desc.textures.find(name);
	if (iter == desc.textures.end()) {
		unsigned int index = desc.textureList.size();
		desc.textures.insert(pair<string, int>(name, index));
		desc.textureList.push_back({ index, name, mipLevel, &texture });
	}
	else {
		RTInfo& info = desc.textureList[iter->second];
		info.texture = &texture;
		info.mipLevel = mipLevel;
	}
	desc.inited = false;
}

Texture * RenderTarget::getTexture(const string & name)
{
	if (name == "depthMap")
		return desc.depthTexure;
	auto re = desc.textures.find(name);
	if (re == desc.textures.end())
		return nullptr;
	return desc.textureList[re->second].texture;
}

Texture * RenderTarget::getTexture(unsigned int index)
{
	if (index >= desc.textureList.size())
		return nullptr;
	return desc.textureList[index].texture;
}

unsigned int RenderTarget::getTextureCount()
{
	return desc.textureList.size();
}

bool RenderTarget::setTextureMipLevel(const string & name, unsigned int mipLevel)
{
	auto iter = desc.textures.find(name);
	if (iter == desc.textures.end())
		return false;
	RTInfo& info = desc.textureList[iter->second];
	if (info.mipLevel == mipLevel)
		return true;
	info.mipLevel = mipLevel;
	desc.inited = false;
	return true;
}

bool RenderTarget::setTextureMipLevel(unsigned int index, unsigned int mipLevel)
{
	if (index >= desc.textureList.size())
		return false;
	RTInfo& info = desc.textureList[index];
	if (info.mipLevel == mipLevel)
		return true;
	info.mipLevel = mipLevel;
	desc.inited = false;
	return true;
}

void RenderTarget::setMultisampleLevel(unsigned int level)
{
	if (desc.multisampleLevel != level) {
		desc.multisampleLevel = level;
		desc.inited = false;
	}
}

unsigned int RenderTarget::getMultisampleLevel()
{
	return desc.multisampleLevel;
}

void RenderTarget::setDepthTexture(Texture & depthTexture)
{
	desc.depthTexure = &depthTexture;
	desc.inited = false;
}

Texture2D * RenderTarget::getInternalDepthTexture()
{
	if (internalDepthTexure != NULL)
		return internalDepthTexure;
	newVendorRenderTarget();
	ITexture2D* vendorTex = vendorRenderTarget->getInternalDepthTexture();
	internalDepthTexure = new Texture2D(vendorTex);
	return internalDepthTexure;
}

void RenderTarget::setTexture(Material & mat)
{
	for (auto b = desc.textureList.begin(), e = desc.textureList.end(); b != e; b++) {
		mat.setTexture(b->name, *b->texture);
	}
	if (desc.depthTexure != NULL)
		mat.setTexture("depthMap", *desc.depthTexure);
}

void RenderTarget::SetMultisampleFrame()
{
	newVendorRenderTarget();
	vendorRenderTarget->SetMultisampleFrame();
}

unsigned int RenderTarget::getFBO()
{
	return desc.frameID;
}

void RenderTarget::resize(unsigned int width, unsigned int height)
{
	newVendorRenderTarget();
	vendorRenderTarget->resize(width, height);
}

void RenderTarget::clearColor(const Color& color)
{
	newVendorRenderTarget();
	vendorRenderTarget->clearColor(color);
}

void RenderTarget::clearColors(const vector<Color>& colors)
{
	newVendorRenderTarget();
	vendorRenderTarget->clearColors(colors);
}

void RenderTarget::clearDepth(float depth)
{
	newVendorRenderTarget();
	vendorRenderTarget->clearDepth(depth);
}

void RenderTarget::clearStencil(unsigned char stencil)
{
	newVendorRenderTarget();
	vendorRenderTarget->clearStencil(stencil);
}

void RenderTarget::newVendorRenderTarget()
{
	if (isDefault()) {
		if (IRenderTarget::defaultRenderTargetDesc == NULL)
			IRenderTarget::defaultRenderTargetDesc = &desc;
		if (IRenderTarget::defaultRenderTarget == NULL)
			IRenderTarget::defaultRenderTarget = VendorManager::getInstance().getVendor().newRenderTarget(desc);
		vendorRenderTarget = IRenderTarget::defaultRenderTarget;
	}
	else if (vendorRenderTarget == NULL) {
		vendorRenderTarget = VendorManager::getInstance().getVendor().newRenderTarget(desc);
		if (vendorRenderTarget == NULL) {
			throw runtime_error("Vendor new RenderTarget failed");
		}
	}
}
