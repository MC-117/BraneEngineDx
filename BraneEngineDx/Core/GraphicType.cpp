#include "GraphicType.h"


ShaderPropertyName::ShaderPropertyName(const char* name)
{
	if (name == NULL)
		throw std::runtime_error("Empty string is invalid");
	hash = calHash(name);
}

ShaderPropertyName::ShaderPropertyName(const std::string& name)
{
	if (name.empty())
		throw std::runtime_error("Empty string is invalid");
	hash = calHash(name.c_str());
}

size_t ShaderPropertyName::calHash(const char* name)
{
	const char* ptr = name;
	size_t hash = 0;
	while (true) {
		size_t data = 0;
		int len = 0;
		for (; len < sizeof(size_t); len++) {
			if (ptr[len] == '\0')
				break;
		}
		memcpy(&data, ptr, len);
		hash ^= data + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		if (len < sizeof(size_t))
			break;
		ptr += sizeof(size_t);
	}
	return hash;
}

size_t ShaderPropertyName::getHash() const
{
	return hash;
}

size_t ShaderPropertyName::operator()() const
{
	return hash;
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
