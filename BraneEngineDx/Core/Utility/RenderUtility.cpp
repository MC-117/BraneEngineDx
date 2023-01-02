#include "RenderUtility.h"

string getShaderFeatureNames(Enum<ShaderFeature> feature)
{
	if (feature.enumValue == Shader_Default)
		return "[default]";
	string name = "";
	if (feature.has(Shader_Custom)) {
		if (feature.has(Shader_Custom_1))
			name += "[custom1]";
		if (feature.has(Shader_Custom_2))
			name += "[custom2]";
		if (feature.has(Shader_Custom_3))
			name += "[custom3]";
		if (feature.has(Shader_Custom_4))
			name += "[custom4]";
		if (feature.has(Shader_Custom_5))
			name += "[custom5]";
		if (feature.has(Shader_Custom_6))
			name += "[custom6]";
		if (feature.has(Shader_Custom_7))
			name += "[custom7]";
		if (feature.has(Shader_Custom_8))
			name += "[custom8]";
	}
	else {
		if (feature.has(Shader_Deferred))
			name += "[deferred]";
		if (feature.has(Shader_Lighting))
			name += "[lighting]";
		if (feature.has(Shader_Postprocess))
			name += "[postprocess]";
		if (feature.has(Shader_Skeleton))
			name += "[skeleton]";
		if (feature.has(Shader_Morph))
			name += "[morph]";
		if (feature.has(Shader_Particle))
			name += "[particle]";
		if (feature.has(Shader_Modifier))
			name += "[modifier]";
		if (feature.has(Shader_Terrain))
			name += "[terrain]";
	}
	return name;
}

int getGPUBufferFormatCellSize(GPUBufferFormat format)
{
	switch (format)
	{
	case GBF_Struct:
		return 0;
	case GBF_Float:
	case GBF_Int:
	case GBF_UInt:
		return 4;
	case GBF_Float2:
	case GBF_Int2:
	case GBF_UInt2:
		return 8;
	case GBF_Float3:
	case GBF_Int3:
	case GBF_UInt3:
		return 12;
	case GBF_Float4:
	case GBF_Int4:
	case GBF_UInt4:
		return 16;
	default:
		return -1;
	}
}

int getPixelSize(TexInternalType type, int channels)
{
	switch (type)
	{
	case TIT_R8_UF:
	case TIT_R8_F:
		return 1;
	case TIT_RG8_UF:
	case TIT_RG8_F:
		return 2;
	case TIT_RGBA8_UF:
	case TIT_RGBA8_F:
	case TIT_SRGBA8_UF:
	case TIT_RGB10A2_UF:
	case TIT_D32_F:
	case TIT_R32_F:
	case TIT_RGBA8_UI:
	case TIT_RGBA8_I:
		return 4;
	case TIT_RGBA16_UF:
	case TIT_RGBA16_F:
	case TIT_RGBA16_FF:
		return 8;
	default:
		return channels * sizeof(char);
	}
}

bool isFloatPixel(TexInternalType type)
{
	switch (type)
	{
	case TIT_RGBA16_FF:
		return true;
	default:
		return false;
	}
}

bool frustumCulling(const CameraData& camData, const Range<Vector3f>& bound, const Matrix4f& mat)
{
	Matrix4f MVP = camData.projectionViewMat * mat;

	Vector4f corners[8] = {
		{bound.minVal.x(), bound.minVal.y(), bound.minVal.z(), 1.0}, // x y z
		{bound.maxVal.x(), bound.minVal.y(), bound.minVal.z(), 1.0}, // X y z
		{bound.minVal.x(), bound.maxVal.y(), bound.minVal.z(), 1.0}, // x Y z
		{bound.maxVal.x(), bound.maxVal.y(), bound.minVal.z(), 1.0}, // X Y z

		{bound.minVal.x(), bound.minVal.y(), bound.maxVal.z(), 1.0}, // x y Z
		{bound.maxVal.x(), bound.minVal.y(), bound.maxVal.z(), 1.0}, // X y Z
		{bound.minVal.x(), bound.maxVal.y(), bound.maxVal.z(), 1.0}, // x Y Z
		{bound.maxVal.x(), bound.maxVal.y(), bound.maxVal.z(), 1.0}, // X Y Z
	};

	bool inside = false;
	for (size_t corner_idx = 0; corner_idx < 8; corner_idx++) {
		// Transform vertex
		Vector4f corner = MVP * corners[corner_idx];
		corner *= 1 / corner.w();
		// Check vertex against clip space bounds
		if (abs(corner.x()) < 1 && abs(corner.y()) < 1 && abs(corner.z() - 0.5f) < 0.5f)
			return true;
	}
	return false;
}
