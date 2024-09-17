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
	memset(this, 0, sizeof(DepthStencilMode));
	stencilReadMask = 0xFF;
	stencilWriteMask = 0xFF;
	depthTest = true;
	depthWrite = true;

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
	uint64_t value;
	memcpy(&value, this, sizeof(DepthStencilMode));
	return value;
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

RenderMode::RenderMode()
{
	bits = 0;
	mode.stencilReadMask = 0xFF;
	mode.stencilWriteMask = 0xFF;
	mode.depthTest = true;
	mode.depthWrite = true;
}

RenderMode::RenderMode(uint16_t renderStage, uint8_t blendMode) : RenderMode()
{
	mode.renderStage = renderStage;
	mode.blendMode = blendMode;

	setDepthStateFromRenderOrder(mode, renderStage);

	mode.stencilComparion_front = RCT_Always;
	mode.stencilPassOp_front = SOT_Replace;
	mode.stencilFailedOp_front = SOT_Keep;
	mode.stencilDepthFailedOp_front = SOT_Keep;

	mode.stencilComparion_back = RCT_Always;
	mode.stencilPassOp_back = SOT_Replace;
	mode.stencilFailedOp_back = SOT_Keep;
	mode.stencilDepthFailedOp_back = SOT_Keep;
}

RenderMode::RenderMode(const RenderMode& mode) { bits = mode.bits; }

RenderMode& RenderMode::operator=(const RenderMode& mode) { bits = mode.bits; return *this; }

uint16_t RenderMode::getRenderStage() const { return mode.renderStage; }
BlendMode RenderMode::getBlendMode() const { return (BlendMode)mode.blendMode; }

uint64_t RenderMode::getOrder() const { return bits; }

DepthStencilMode RenderMode::getDepthStencilMode() const { return mode; }

RenderMode::operator uint64_t() const { return bits; }

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
