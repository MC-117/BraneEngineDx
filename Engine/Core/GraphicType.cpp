#include "GraphicType.h"
#include "Utility/hash.h"

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

RenderMode::RenderMode(uint16_t renderStage, uint8_t blendMode, uint16_t subOrder)
	: value(subOrder
		| ((blendMode << BM_Order_BitOffset) & BM_Order_BitMask)
		| ((renderStage << RS_Order_BitOffset) & RS_Order_BitMask)
	) {}

RenderMode::RenderMode(const RenderMode& mode) { value = mode.value; }

RenderMode& RenderMode::operator=(const RenderMode& mode) { value = mode.value; return *this; }

uint16_t RenderMode::getRenderStage() const { return (value & RS_Order_BitMask) >> RS_Order_BitOffset; }
BlendMode RenderMode::getBlendMode() const { return (BlendMode)((value & BM_Order_BitMask) >> BM_Order_BitOffset); }
uint32_t RenderMode::getOrder() const { return value & Order_BitMask; }
uint16_t RenderMode::getSubOrder() const { return value & SubOrder_BitMask; }

RenderMode::operator uint32_t() const { return value; }

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
