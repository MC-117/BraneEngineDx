#pragma once
#include <iostream>

enum TexDimension
{
	TD_Default = -1,
	TD_Single = 0,
	TD_Array = 1,
	TD_Cube = 2,
	TD_CubeArray = 3
};

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
	TIT_R8_UF,
	TIT_R8_F,
	TIT_R8_UI,
	TIT_R8_I,
	TIT_RG8_UF,
	TIT_RG8_F,
	TIT_RGBA8_UF,
	TIT_RGBA8_F,
	TIT_RGBA8_UI,
	TIT_RGBA8_I,
	TIT_SRGBA8_UF,
	TIT_BGRA8_UF,
	TIT_SBGRA8_UF,
	TIT_RGB10A2_UF,
	TIT_RGBA16_UF,
	TIT_RGBA16_F,
	TIT_RGBA16_FF,
	TIT_D32_F,
	TIT_R32_F,
	TIT_R32_UI,
	TIT_R32_I,
	TIT_RG32_F,
	TIT_RG32_UI,
	TIT_RG32_I,
	TIT_RGB32_F,
	TIT_RGB32_UI,
	TIT_RGB32_I,
	TIT_RGBA32_F,
	TIT_RGBA32_UI,
	TIT_RGBA32_I,
};

enum CubeFace
{
	CF_Front,
	CF_Back,
	CF_Left,
	CF_Right,
	CF_Top,
	CF_Bottom,
	CF_Faces
};

struct BufferOption
{
	bool output = false;
	unsigned int offset = 0;
	unsigned int stride = (unsigned int)-1;
};

struct MipOption
{
	TexDimension dimension = TD_Default;
	unsigned int detailMip = 0;
	unsigned int mipCount = 0;
	unsigned int arrayBase = 0;
	unsigned int arrayCount = 1;
};

struct RTOption
{
	unsigned int mipLevel = 0;
	unsigned int arrayBase = 0;
	unsigned int arrayCount = 1;
	bool multisample = false;
};

struct RWOption
{
	TexDimension dimension = TD_Default;
	unsigned int mipLevel = 0;
	unsigned int arrayBase = 0;
	unsigned int arrayCount = 1;
};

enum GPUBufferType
{
	GB_Constant,
	GB_Storage,
	GB_Command,
	GB_Vertex,
	GB_Index
};

enum GPUBufferFormat
{
	GBF_Struct,
	GBF_Raw,
	GBF_Float,
	GBF_Float2,
	GBF_Float3,
	GBF_Float4,
	GBF_Int,
	GBF_Int2,
	GBF_Int3,
	GBF_Int4,
	GBF_UInt,
	GBF_UInt2,
	GBF_UInt3,
	GBF_UInt4,
};

enum CPUAccessFlag
{
	CAF_None = 0,
	CAF_Read = 1,
	CAF_Write = 2,
	CAF_ReadWrite = 3
};

enum GPUAccessFlag
{
	GAF_Read = 0,
	GAF_ReadWrite = 1
};

enum CullType
{
	Cull_Off,
	Cull_Back,
	Cull_Front
};

enum ShaderFeature
{
	Shader_Default		 = 0,
	Shader_Deferred		 = 1 << 0,
	Shader_Lighting		 = 1 << 1,
	Shader_Depth		 = 1 << 2,
	Shader_Postprocess	 = 1 << 3,
	Shader_Skeleton		 = 1 << 4,
	Shader_Morph		 = 1 << 5,
	Shader_Particle		 = 1 << 6,
	Shader_Modifier		 = 1 << 7,
	Shader_Terrain		 = 1 << 8,
	Shader_VSM			 = 1 << 9,
	Shader_Debug		 = 1 << 10,
	Shader_Custom_1		 = 1 << 11,
	Shader_Custom_2		 = 1 << 12,
	Shader_Custom_3		 = 1 << 13,
	Shader_Custom_4		 = 1 << 14,
	Shader_Custom_5		 = 1 << 15,
	Shader_Custom_6		 = 1 << 16,
	Shader_Custom_7		 = 1 << 17,
	Shader_Custom_8		 = 1 << 18,
	Shader_Custom_9		 = 1 << 19,
	Shader_Custom_10	 = 1 << 20,
	Shader_Feature_Count = 21,
	Shader_Feature_Mask  = (1 << Shader_Feature_Count) - 1,
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

enum ClearFlags : uint8_t
{
	Clear_None = 0,
	Clear_Colors = 1,
	Clear_Depth = 2,
	Clear_Stencil = 4,
	Clear_All = Clear_Colors | Clear_Depth | Clear_Stencil
};

enum RenderStage : uint16_t
{
	RS_PreBackground = 0,
	RS_Geometry = 500,
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
	RenderMode(uint16_t renderStage, uint8_t blendMode, uint16_t subOrder);
	RenderMode(const RenderMode& mode);
	RenderMode& operator=(const RenderMode& mode);

	uint16_t getRenderStage() const;
	BlendMode getBlendMode() const;
	uint32_t getOrder() const;
	uint16_t getSubOrder() const;

	operator uint32_t() const;
};

struct ShaderPropertyName
{
	size_t hash;

	ShaderPropertyName(const char* name);
	ShaderPropertyName(const std::string& name);

	static size_t calHash(const char* name);

	size_t getHash() const;

	bool operator==(const ShaderPropertyName& name) const;
};

namespace std
{
	template<>
	struct hash<ShaderPropertyName>
	{
		size_t operator()(const ShaderPropertyName& name) const
		{
			return name.getHash();
		}
	};
}

struct ShaderProperty
{
	enum Type
	{
		None,
		Parameter,
		ConstantBuffer,
		TextureBuffer,
		Texture,
		Sampler,
		Image
	};
	Type type = None;
	std::string name;
	int offset;
	int size;
	int meta;
};