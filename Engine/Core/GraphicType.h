#pragma once
#include "Utility/Name.h"
#include <vector>
#include <unordered_map>

enum TexDimension
{
	TD_Default = -1,
	TD_Single = 0,
	TD_Array = 1,
	TD_Cube = 2,
	TD_CubeArray = 3,
	TD_Volume = 4
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

enum TexInternalType : uint8_t
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
	TIT_D32_F_S8_UI,
	TIT_R32_F_S8_X,
	TIT_R32_X_S8_UI,
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
	bool bindStorageAsVertex = false;
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

enum DepthStencilAccessFlag : uint8_t
{
	DSA_Default = 0,
	DSA_DepthReadOnly = 1 << 0,
	DSA_StencilReadOnly = 1 << 1,
	DSA_DepthStencilReadOnly = DSA_DepthReadOnly | DSA_StencilReadOnly,
};

enum CullType : uint8_t
{
	Cull_Off,
	Cull_Back,
	Cull_Front
};

enum GPUQueryType
{
	GQT_Unknown,
	GQT_Occlusion,
	GQT_Timestamp
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

enum MeshType : uint8_t
{
	MT_Mesh,
	MT_SkeletonMesh,
	MT_Terrain,
	MT_Lines,
	MT_Screen,
	MT_Particle
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
	RS_Post = 0xFFF,
	RS_Max = 0x1000,
	RS_Count = 6
};

enum BlendMode : uint8_t
{
	BM_Default,
	BM_Disable,
	BM_DepthOnly,
	BM_AlphaTest,
	BM_Additive,
	BM_Multiply,
	BM_PremultiplyAlpha,
	BM_Mask,
	BM_Replace,
	BM_MultiplyAlpha,
	BM_Custom,
};

enum StencilOperationType : uint8_t
{
	SOT_Keep,
	SOT_Zero,
	SOT_Replace,
	SOT_Increase_Clamp,
	SOT_Decrease_Clamp,
	SOT_Invert,
	SOT_Increase,
	SOT_Decrease,
};

enum RenderComparionType : uint8_t
{
	RCT_Never,
	RCT_Less,
	RCT_Equal,
	RCT_LessEqual,
	RCT_Greater,
	RCT_NotEqual,
	RCT_GreaterEqual,
	RCT_Always,
};

enum BlendOperation : uint8_t
{
	BO_Add,
	BO_Sub,
	BO_RevSub,
	BO_Min,
	BO_Max,

	BO_Num,
};

enum BlendFactor : uint8_t
{
	BF_Zero,
	BF_One,
	BF_SrcColor,
	BF_InvSrcColor,
	BF_SrcAlpha,
	BF_InvSrcAlpha,
	BF_DestAplha,
	BF_InvDestAplha,
	BF_DestColor,
	BF_InvDestColor,
	BF_ConstFactor,
	BF_InvConstFactor,
	BF_Src1Color,
	BF_InvSrc1Color,
	BF_Src1Alpha,
	BF_InvSrc1Alpha,

	BF_BitMask = 0xF,
	BF_Num,
};

enum ColorWriteMask : uint8_t
{
	CWM_None = 0,
	
	CWM_R = 1 << 0,
	CWM_G = 1 << 1,
	CWM_B = 1 << 2,
	CWM_A = 1 << 3,

	CWM_RG = CWM_R | CWM_G,
	CWM_RB = CWM_R | CWM_B,
	CWM_RA = CWM_R | CWM_A,
	CWM_GB = CWM_G | CWM_B,
	CWM_GA = CWM_G | CWM_A,
	CWM_BA = CWM_B | CWM_A,
	
	CWM_RGB = CWM_R | CWM_G | CWM_B,
	CWM_RBA = CWM_R | CWM_B | CWM_A,
	CWM_RGA = CWM_R | CWM_G | CWM_A,
	CWM_GBA = CWM_G | CWM_B | CWM_A,
	
	CWM_RGBA = CWM_R | CWM_G | CWM_B | CWM_A,
};

enum DecalMask : uint8_t
{
	Decal_None      = 0,
	Decal_BaseColor = 1 << 0,
	Decal_Normal    = 1 << 1,
	Decal_Roughness = 1 << 2,
	Decal_Metallic  = 1 << 3,
	Decal_Specular  = 1 << 4,
	Decal_AO        = 1 << 5,
	Decal_Emission  = 1 << 6,
};

#pragma pack(push, 1)
struct DepthStencilMode
{
	union 
	{
		struct
		{
			uint8_t stencilReadMask : 8;
			uint8_t stencilWriteMask : 8;

			StencilOperationType stencilFailedOp_front : 4;
			StencilOperationType stencilDepthFailedOp_front : 4;
			StencilOperationType stencilPassOp_front : 4;
			RenderComparionType stencilComparion_front : 4;

			StencilOperationType stencilFailedOp_back : 4;
			StencilOperationType stencilDepthFailedOp_back : 4;
			StencilOperationType stencilPassOp_back : 4;
			RenderComparionType stencilComparion_back : 4;

			RenderComparionType depthComparion : 4;

			bool stencilTest : 1;
			bool depthWrite : 1;
			bool depthTest : 1;
			uint8_t pad1 : 1;
			DepthStencilAccessFlag accessFlag : 4;
			uint8_t pad2 : 4;
		};
		uint64_t word;
	};

	DepthStencilMode();

	operator uint64_t() const;

	DepthStencilMode& operator=(const DepthStencilMode& mode);

	static DepthStencilMode DepthTestWritable();
	static DepthStencilMode DepthTestNonWritable();
	static DepthStencilMode DepthNonTestNonWritable();
};

template <>
struct ENGINE_API std::hash<DepthStencilMode>
{
	std::size_t operator()(const DepthStencilMode& mode) const noexcept;
};

struct RenderTargetMode
{
	bool enableBlend : 1;
	BlendOperation colorOp : 3;
	bool enableAlphaTest : 1;
	BlendOperation alphaOp : 3;
	BlendFactor srcColorFactor : 8;
	BlendFactor destColorFactor : 8;
	BlendFactor srcAlphaFactor : 8;
	BlendFactor destAlphaFactor : 8;
	ColorWriteMask writeMask : 4;
	BlendMode blendMode : 4;

	RenderTargetMode();
	RenderTargetMode(BlendMode mode);

	void setBlendMode(BlendMode mode);

	operator uint64_t() const;

	RenderTargetMode& operator=(const RenderTargetMode& mode);
};

constexpr int MaxRenderTargets = 8;

struct RenderTargetModes
{
	RenderTargetMode rtModes[MaxRenderTargets];

	RenderTargetModes();
	RenderTargetModes(BlendMode blendMode);

	void setBlendModes(BlendMode blendMode);

	bool operator==(const RenderTargetModes& mode) const;

	RenderTargetMode& operator[](uint8_t rtIndex);
	const RenderTargetMode& operator[](uint8_t rtIndex) const;
};

template <>
struct ENGINE_API std::hash<RenderTargetModes>
{
	std::size_t operator()(const RenderTargetModes& modes) const noexcept;
};

struct ENGINE_API RenderMode
{
	union
	{
		struct
		{
			uint32_t renderStage;
			uint32_t subRenderStage;
			DepthStencilMode dsMode;
			RenderTargetModes rtModes;
		};
		struct
		{
			uint64_t words[8];
		};
	};
	RenderMode();
	RenderMode(uint16_t renderStage);
	RenderMode(uint16_t renderStage, BlendMode blendMode);
	RenderMode(const RenderMode& mode);
	RenderMode& operator=(const RenderMode& mode);

	uint16_t getRenderStage() const;
	uint64_t getOrder() const;

	void setDepthStencilMode(DepthStencilMode depthStencilMode);
	const DepthStencilMode& getDepthStencilMode() const;
	const RenderTargetModes& getRenderTargetModes() const;

	RenderTargetMode& operator[](uint8_t rtIndex);
	const RenderTargetMode& operator[](uint8_t rtIndex) const;

	bool operator==(const RenderMode& mode) const;
};

template<>
struct ENGINE_API std::hash<RenderMode>
{
	size_t operator()(const RenderMode& mode) const noexcept;
};
#pragma pack(pop)

struct ENGINE_API ShaderPropertyName : public Name
{
	ShaderPropertyName(const char* name);
	ShaderPropertyName(const std::string& name);
	ShaderPropertyName(const Name& name);
};

template<>
struct ENGINE_API std::hash<ShaderPropertyName>
{
	size_t operator()(const ShaderPropertyName& name) const noexcept
	{
		return name.getHash();
	}
};

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
	ShaderPropertyName name;
	int offset;
	int size;
	int meta;

	ShaderProperty(const ShaderPropertyName& name) : name(name), offset(0), size(0), meta(0) {}
};

struct ShaderPropertyDesc
{
	std::vector<std::pair<ShaderStageType, const ShaderProperty*>> properties;

	const ShaderPropertyName& getName() const;
	const ShaderProperty* getParameter() const;
	const ShaderProperty* getConstantBuffer() const;
};

class ShaderPropertyLayout
{
public:
	ShaderPropertyLayout() = default;
	size_t getBufferSize() const;
	const ShaderPropertyDesc* getLayout(const ShaderPropertyName& name) const;

	void setBufferSize(size_t size);
	ShaderPropertyDesc& emplaceLayout(const ShaderPropertyName& name);
	void clear();
protected:
	size_t bufferSize = 0;
	std::unordered_map<ShaderPropertyName, ShaderPropertyDesc> propertyDesces;
};

enum CameraRenderFlags : unsigned int
{
	CameraRender_Default = 0,
	CameraRender_GizmoDraw = 1 << 0,
	CameraRender_DebugDraw = 1 << 1,
	CameraRender_SceneCapture = 1 << 2,
};
