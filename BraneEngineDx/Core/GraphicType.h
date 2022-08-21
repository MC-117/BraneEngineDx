#pragma once

enum TexWrapType
{
	TW_Repeat, TW_Mirror, TW_Clamp, TW_Clamp_Edge, TW_Border, TW_Mirror_Once
};

enum TexFilter
{
	TF_Point, TF_Linear, TF_Point_Mip_Point, TF_Linear_Mip_Point, TF_Point_Mip_Linear, TF_Linear_Mip_Linear
};

enum TexInternalType
{
	TIT_Default, TIT_R, TIT_RG, TIT_RGBA, TIT_SRGBA, TIT_Depth, TIT_R32
};

enum GPUBufferType
{
	GB_Constant, GB_Storage, GB_Struct, GB_Command, GB_Vertex, GB_Index, GB_ReadBack
};

enum CPUAccessFlag
{
	CAF_None = 0, CAF_Read = 1, CAF_Write = 2, CAF_ReadWrite = 3
};

enum CullType
{
	Cull_Off, Cull_Back, Cull_Front
};