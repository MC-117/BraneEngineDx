#include "GraphicType.h"
#include "Utility/hash.h"
#include "Utility/RenderUtility.h"

ShaderPropertyName::ShaderPropertyName(const char* name) : Name(name)
{
	if (name == NULL)
		throw std::runtime_error("Empty string is invalid");
}

ShaderPropertyName::ShaderPropertyName(const std::string& name) : Name(name)
{
	if (name.empty())
		throw std::runtime_error("Empty string is invalid");
}

ShaderPropertyName::ShaderPropertyName(const Name& name) : Name(name)
{
	if (name.isNone())
		throw std::runtime_error("Empty string is invalid");
}

DepthStencilMode::DepthStencilMode()
{
	word = 0;
	stencilReadMask = 0xFF;
	stencilWriteMask = 0xFF;
	depthTest = true;
	depthWrite = true;
	depthComparion = RCT_GreaterEqual;

	stencilComparion_front = RCT_Always;
	stencilPassOp_front = SOT_Replace;
	stencilFailedOp_front = SOT_Keep;
	stencilDepthFailedOp_front = SOT_Keep;

	stencilComparion_back = RCT_Always;
	stencilPassOp_back = SOT_Replace;
	stencilFailedOp_back = SOT_Keep;
	stencilDepthFailedOp_back = SOT_Keep;
}

DepthStencilMode::operator uint64_t() const
{
	return word;
}

DepthStencilMode& DepthStencilMode::operator=(const DepthStencilMode& mode)
{
	word = mode.word;
	return *this;
}

DepthStencilMode DepthStencilMode::DepthTestWritable()
{
	DepthStencilMode mode;
	return mode;
}

DepthStencilMode DepthStencilMode::DepthTestNonWritable()
{
	DepthStencilMode mode;
	mode.depthWrite = false;
	return mode;
}

DepthStencilMode DepthStencilMode::DepthNonTestNonWritable()
{
	DepthStencilMode mode;
	mode.depthTest = false;
	mode.depthWrite = false;
	return mode;
}

std::size_t hash<DepthStencilMode>::operator()(const DepthStencilMode& mode) const noexcept
{
	return mode;
}

RenderTargetMode::RenderTargetMode()
{
	setBlendMode(BM_Default);
}

RenderTargetMode::RenderTargetMode(BlendMode mode)
{
	setBlendMode(mode);
}

void RenderTargetMode::setBlendMode(BlendMode mode)
{
	switch (mode) {
	case BM_Default:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_SrcAlpha;
		destColorFactor = BF_InvSrcAlpha;
		colorOp = BO_Add;
		srcAlphaFactor = BF_One;
		destAlphaFactor = BF_Zero;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_Disable:
		enableBlend = false;
		enableAlphaTest = false;
		srcColorFactor = BF_Zero;
		destColorFactor = BF_One;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_One;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_DepthOnly:
		enableBlend = false;
		enableAlphaTest = false;
		srcColorFactor = BF_Zero;
		destColorFactor = BF_One;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_One;
		alphaOp = BO_Add;
		writeMask = CWM_None;
		break;
	case BM_Replace:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_One;
		destColorFactor = BF_Zero;
		colorOp = BO_Add;
		srcAlphaFactor = BF_One;
		destAlphaFactor = BF_Zero;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_AlphaTest:
		enableBlend = false;
		enableAlphaTest = true;
		srcColorFactor = BF_Zero;
		destColorFactor = BF_One;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_One;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_Additive:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_One;
		destColorFactor = BF_One;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_One;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_PremultiplyAlpha:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_One;
		destColorFactor = BF_InvSrcAlpha;
		colorOp = BO_Add;
		srcAlphaFactor = BF_One;
		destAlphaFactor = BF_InvSrcAlpha;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_MultiplyAlpha:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_One;
		destColorFactor = BF_SrcAlpha;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_One;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_Multiply:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_DestColor;
		destColorFactor = BF_InvSrcAlpha;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_One;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	case BM_Mask:
		enableBlend = true;
		enableAlphaTest = false;
		srcColorFactor = BF_Zero;
		destColorFactor = BF_InvSrcColor;
		colorOp = BO_Add;
		srcAlphaFactor = BF_Zero;
		destAlphaFactor = BF_InvSrcAlpha;
		alphaOp = BO_Add;
		writeMask = CWM_RGBA;
		break;
	default:
		assert(false);
	}
	blendMode = mode;
}

RenderTargetMode::operator unsigned long long() const
{
	uint64_t value;
	memcpy(&value, this, sizeof(DepthStencilMode));
	return value;
}

RenderTargetMode& RenderTargetMode::operator=(const RenderTargetMode& mode)
{
	enableBlend = mode.enableBlend;
	colorOp = mode.colorOp;
	enableAlphaTest = mode.enableAlphaTest;
	alphaOp = mode.alphaOp;
	srcColorFactor = mode.srcColorFactor;
	destColorFactor = mode.destColorFactor;
	srcAlphaFactor = mode.srcAlphaFactor;
	destAlphaFactor = mode.destAlphaFactor;
	writeMask = mode.writeMask;
	blendMode = mode.blendMode;

	return *this;
}

RenderTargetModes::RenderTargetModes()
{
	for (auto& rtMode : rtModes) {
		rtMode.setBlendMode(BM_Default);
	}
}

RenderTargetModes::RenderTargetModes(BlendMode blendMode)
{
	setBlendModes(blendMode);
}

void RenderTargetModes::setBlendModes(BlendMode blendMode)
{
	for (auto& rtMode : rtModes) {
		rtMode.setBlendMode(blendMode);
	}
}

bool RenderTargetModes::operator==(const RenderTargetModes& mode) const
{
	for (int index = 0; index < MaxRenderTargets; index++) {
		if (rtModes[index] != mode.rtModes[index])
			return false;
	}
	return true;
}

RenderTargetMode& RenderTargetModes::operator[](uint8_t rtIndex)
{
	return rtModes[rtIndex];
}

const RenderTargetMode& RenderTargetModes::operator[](uint8_t rtIndex) const
{
	return rtModes[rtIndex];
}

std::size_t hash<RenderTargetModes>::operator()(const RenderTargetModes& modes) const noexcept
{
	size_t hash = 0;
	for (auto& rtMode : modes.rtModes) {
		hash_combine(hash, (uint64_t)rtMode);
	}
	return hash;
}

RenderMode::RenderMode()
{
	memset(words, 0, sizeof(words));
	rtModes.setBlendModes(BM_Default);
	dsMode = DepthStencilMode::DepthTestWritable();
}

RenderMode::RenderMode(uint16_t renderStage) : RenderMode()
{
	this->renderStage = renderStage;

	BlendMode blendMode = getBlendModeFromRenderOrder(renderStage);
	rtModes.setBlendModes(blendMode);
	setDepthStateFromRenderOrder(dsMode, renderStage);

	dsMode.stencilComparion_front = RCT_Always;
	dsMode.stencilPassOp_front = SOT_Replace;
	dsMode.stencilFailedOp_front = SOT_Keep;
	dsMode.stencilDepthFailedOp_front = SOT_Keep;

	dsMode.stencilComparion_back = RCT_Always;
	dsMode.stencilPassOp_back = SOT_Replace;
	dsMode.stencilFailedOp_back = SOT_Keep;
	dsMode.stencilDepthFailedOp_back = SOT_Keep;
}

RenderMode::RenderMode(uint16_t renderStage, BlendMode blendMode) : RenderMode(renderStage)
{
	rtModes.setBlendModes(blendMode);
}

RenderMode::RenderMode(const RenderMode& mode) { memcpy_s(words, sizeof(words), mode.words, sizeof(mode.words)); }

RenderMode& RenderMode::operator=(const RenderMode& mode) { memcpy_s(words, sizeof(words), mode.words, sizeof(mode.words)); return *this; }

uint16_t RenderMode::getRenderStage() const { return renderStage; }

uint64_t RenderMode::getOrder() const { return renderStage; }

void RenderMode::setDepthStencilMode(DepthStencilMode depthStencilMode)
{
	dsMode = depthStencilMode;
}

const DepthStencilMode& RenderMode::getDepthStencilMode() const
{
	return dsMode;
}

const RenderTargetModes& RenderMode::getRenderTargetModes() const
{
	return rtModes;
}

RenderTargetMode& RenderMode::operator[](uint8_t rtIndex)
{
	return rtModes[rtIndex];
}

const RenderTargetMode& RenderMode::operator[](uint8_t rtIndex) const
{
	return rtModes[rtIndex];
}

bool RenderMode::operator==(const RenderMode& mode) const
{
	return memcmp(words, mode.words, sizeof(words)) == 0;
}

size_t hash<RenderMode>::operator()(const RenderMode& mode) const noexcept
{
	size_t hash = 0;
	for (uint64_t word : mode.words) {
		hash_combine(hash, word);
	}
	return hash;
}

const ShaderPropertyName& ShaderPropertyDesc::getName() const
{
	return properties.front().second->name;
}

const ShaderProperty* ShaderPropertyDesc::getParameter() const
{
	for (auto& prop : properties)
		if (prop.second->type == ShaderProperty::Parameter)
			return prop.second;
	return NULL;
}

const ShaderProperty* ShaderPropertyDesc::getConstantBuffer() const
{
	for (auto& prop : properties)
		if (prop.second->type == ShaderProperty::ConstantBuffer)
			return prop.second;
	return NULL;
}

size_t ShaderPropertyLayout::getBufferSize() const
{
	return bufferSize;
}

const ShaderPropertyDesc* ShaderPropertyLayout::getLayout(const ShaderPropertyName& name) const
{
	auto iter = propertyDesces.find(name);
	if (iter == propertyDesces.end())
		return NULL;
	return &iter->second;
}

void ShaderPropertyLayout::setBufferSize(size_t size)
{
	bufferSize = size;
}

ShaderPropertyDesc& ShaderPropertyLayout::emplaceLayout(const ShaderPropertyName& name)
{
	auto iter = propertyDesces.find(name);
	ShaderPropertyDesc* desc;
	if (iter == propertyDesces.end())
		desc = &propertyDesces.emplace(name, ShaderPropertyDesc()).first->second;
	else
		desc = &iter->second;
	return *desc;
}

void ShaderPropertyLayout::clear()
{
	bufferSize = 0;
	propertyDesces.clear();
}
