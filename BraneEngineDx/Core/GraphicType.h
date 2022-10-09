#pragma once

enum TexWrapType
{
	TW_Repeat,
	TW_Mirror,
	TW_Clamp,
	TW_Clamp_Edge,
	TW_Border,
	TW_Mirror_Once
};

enum TexFilter
{
	TF_Point,
	TF_Linear,
	TF_Point_Mip_Point,
	TF_Linear_Mip_Point,
	TF_Point_Mip_Linear,
	TF_Linear_Mip_Linear
};

enum TexInternalType
{
	TIT_Default,
	TIT_R,
	TIT_RG,
	TIT_RGBA,
	TIT_SRGBA,
	TIT_HRGBA,
	TIT_Depth,
	TIT_R32
};

enum GPUBufferType
{
	GB_Constant,
	GB_Storage,
	GB_Struct,
	GB_Command,
	GB_Vertex,
	GB_Index,
	GB_ReadBack
};

enum CPUAccessFlag
{
	CAF_None = 0,
	CAF_Read = 1,
	CAF_Write = 2,
	CAF_ReadWrite = 3
};

enum CullType
{
	Cull_Off,
	Cull_Back,
	Cull_Front
};

enum ShaderFeature
{
	Shader_Default		= 0,	Shader_Custom		= 1,
	Shader_Deferred		= 2,	Shader_Custom_1		= 2 + 1,
	Shader_Lighting		= 4,	Shader_Custom_2		= 4 + 1,
	Shader_Postprocess	= 8,	Shader_Custom_3		= 8 + 1,
	Shader_Skeleton		= 16,	Shader_Custom_4		= 16 + 1,
	Shader_Morph		= 32,	Shader_Custom_5		= 32 + 1,
	Shader_Particle		= 64,	Shader_Custom_6		= 64 + 1,
	Shader_Modifier		= 128,	Shader_Custom_7		= 128 + 1,
	Shader_Terrain		= 256,	Shader_Custom_8		= 256 + 1
};

enum ShaderStageType
{
	None_Shader_Stage,
	Vertex_Shader_Stage,
	Tessellation_Control_Shader_Stage,
	Tessellation_Evalution_Shader_Stage,
	Geometry_Shader_Stage,
	Fragment_Shader_Stage,
	Compute_Shader_Stage
};

enum MeshType
{
	MT_Mesh,
	MT_SkeletonMesh,
	MT_Terrain
};

enum RenderStage : uint16_t
{
	RS_PreBackground = 0,
	RS_Opaque = 1000,
	RS_Aplha = 2450,
	RS_Transparent = 2500,
	RS_Post = 0x1000
};

const uint32_t RS_Order_BitMask = 0x1FFF0000;
const uint8_t RS_Order_BitOffset = 16;

enum BlendMode : uint8_t
{
	BM_Default = 0,
	BM_Additive = 1,
	BM_Multipy = 2,
	BM_PremultiplyAlpha = 3,
	BM_Mask = 4
};

const uint32_t BM_Order_BitMask = 0xE0000000;
const uint8_t BM_Order_BitOffset = 32 - 3;

struct RenderMode
{
	const uint32_t Order_BitMask = 0x1FFFFFFF;
	const uint32_t SubOrder_BitMask = 0xFFFF;
	uint32_t value = 0;
	RenderMode() = default;
	RenderMode(uint16_t renderStage, uint8_t blendMode, uint16_t subOrder)
		: value(subOrder
			| ((blendMode << BM_Order_BitOffset) & BM_Order_BitMask)
			| ((renderStage << RS_Order_BitOffset) & RS_Order_BitMask)
		) {}
	RenderMode(const RenderMode& mode) { value = mode.value; }
	RenderMode& operator=(const RenderMode& mode) { value = mode.value; return *this; }

	uint16_t getRenderStage() const { return (value & RS_Order_BitMask) >> RS_Order_BitOffset; }
	BlendMode getBlendMode() const { return (BlendMode)((value & BM_Order_BitMask) >> BM_Order_BitOffset); }
	uint32_t getOrder() const { return value & Order_BitMask; }
	uint16_t getSubOrder() const { return value & SubOrder_BitMask; }

	operator uint32_t() const { return value; }
};