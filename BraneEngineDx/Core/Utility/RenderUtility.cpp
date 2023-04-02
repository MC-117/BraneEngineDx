#include "RenderUtility.h"
#include "MathUtility.h"
#include "../ShaderStage.h"

const char* shaderFeatureNames[] = {
	"default",
	"deferred",
	"lighting",
	"postprocess",
	"skeleton",
	"morph",
	"particle",
	"modifier",
	"terrain",
	"vsm",
	"debug",
	"custom1",
	"custom2",
	"custom3",
	"custom4",
	"custom5",
	"custom6",
	"custom7",
	"custom8",
	"custom9",
	"custom10",
};

map<string, ShaderFeature> shaderFeatureNameMap = {
	{ "default", Shader_Default },
	{ "deferred", Shader_Deferred },
	{ "lighting", Shader_Lighting },
	{ "postprocess", Shader_Postprocess },
	{ "skeleton", Shader_Skeleton },
	{ "morph", Shader_Morph },
	{ "particle", Shader_Particle },
	{ "modifier", Shader_Modifier },
	{ "terrain", Shader_Terrain },
	{ "vsm", Shader_VSM },
	{ "debug", Shader_Debug },
	{ "custom1", Shader_Custom_1 },
	{ "custom2", Shader_Custom_2 },
	{ "custom3", Shader_Custom_3 },
	{ "custom4", Shader_Custom_4 },
	{ "custom5", Shader_Custom_5 },
	{ "custom6", Shader_Custom_6 },
	{ "custom7", Shader_Custom_7 },
	{ "custom8", Shader_Custom_8 },
	{ "custom9", Shader_Custom_9 },
	{ "custom10", Shader_Custom_10 },
};

ShaderFeature getShaderFeatureFromName(const string& name)
{
	auto iter = shaderFeatureNameMap.find(name);
	if (iter == shaderFeatureNameMap.end())
		throw runtime_error("Unknown shader feature");
	return iter->second;
}

string getShaderFeatureName(ShaderFeature feature)
{
	return shaderFeatureNames[feature];
}

string getShaderFeatureNames(Enum<ShaderFeature> feature)
{
	if (feature.enumValue == Shader_Default)
		return "[default]";
	string name = "";
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
	if (feature.has(Shader_VSM))
		name += "[vsm]";
	if (feature.has(Shader_Debug))
		name += "[debug]";
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
	if (feature.has(Shader_Custom_9))
		name += "[custom9]";
	if (feature.has(Shader_Custom_10))
		name += "[custom10]";
	return name;
}

const char* getShaderPropertyTypeName(ShaderProperty::Type type)
{
	switch (type)
	{
	case ShaderProperty::None:
		return "None";
	case ShaderProperty::Parameter:
		return "Parameter";
	case ShaderProperty::ConstantBuffer:
		return "ConstantBuffer";
	case ShaderProperty::TextureBuffer:
		return "TextureBuffer";
	case ShaderProperty::Texture:
		return "Texture";
	case ShaderProperty::Sampler:
		return "Sampler";
	case ShaderProperty::Image:
		return "Image";
	default:
		return "Unknown";
	}
}

const char* getShaderExtension(ShaderStageType type)
{
	switch (type)
	{
	case Vertex_Shader_Stage:
		return ".vs";
	case Tessellation_Control_Shader_Stage:
		return ".tcs";
	case Tessellation_Evalution_Shader_Stage:
		return ".tes";
	case Geometry_Shader_Stage:
		return ".gs";
	case Fragment_Shader_Stage:
		return ".fs";
	case Compute_Shader_Stage:
		return ".cs";
	}
	return "";
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

bool frustumCulling(const CameraData& camData, const BoundBox& bound, const Matrix4f& mat)
{
	{
		float vLen = camData.zFar * tan(camData.fovy * (0.5f * PI / 180.0f));
		float hLen = vLen * camData.aspect;

		Vector3f pos, sca;
		Quaternionf rot;
		mat.decompose(pos, rot, sca);
		Vector3f _maxVal = bound.maxPoint.cwiseProduct(sca);
		Vector3f _minVal = bound.minPoint.cwiseProduct(sca);
		Vector3f maxVal = Math::max(_maxVal, _minVal);
		Vector3f minVal = Math::min(_maxVal, _minVal);

		Matrix4f T = Matrix4f::Identity();
		T(0, 3) = pos.x();
		T(1, 3) = pos.y();
		T(2, 3) = pos.z();
		Matrix4f R = Matrix4f::Identity();
		R.block(0, 0, 3, 3) = rot.toRotationMatrix();;

		Matrix4f worldToLocal = (T * R).inverse();

		Vector3f worldPos = worldToLocal *
			Vector4f(camData.cameraLoc, 1);
		Vector3f upVec = worldToLocal *
			Vector4f(camData.cameraUp, 0);
		Vector3f rightVec = worldToLocal *
			Vector4f(camData.cameraLeft, 0);
		Vector3f forVec = worldToLocal *
			Vector4f(camData.cameraDir, 0);

		Vector3f vVec = upVec * vLen;
		Vector3f hVec = rightVec * hLen;

		Vector3f farPoint = worldPos + forVec * camData.zFar;
		Vector3f nearPoint = worldPos + forVec * camData.zNear;

		Vector3f corner[4] =
		{
			farPoint - vVec - hVec,
			farPoint - vVec + hVec,
			farPoint + vVec - hVec,
			farPoint + vVec + hVec,
		};

		Vector4f planes[8] =
		{
			Vector4f::Plane(corner[1], corner[0], worldPos),
			Vector4f::Plane(corner[2], corner[3], worldPos),
			Vector4f::Plane(corner[0], corner[2], worldPos),
			Vector4f::Plane(corner[3], corner[1], worldPos),
			Vector4f::Plane(-forVec, farPoint),
			Vector4f::Plane(forVec, nearPoint)
		};

		planes[6] = planes[0];
		planes[7] = planes[0];

		Vector3f extend = (maxVal - minVal) * 0.5f;
		Vector3f center = (maxVal + minVal) * 0.5f;

		return IntersectAABB8Plane(center, extend, planes);
	}

	Matrix4f MVP = camData.projectionViewMat * mat;

	Vector4f corners[8] = {
		{bound.minPoint.x(), bound.minPoint.y(), bound.minPoint.z(), 1.0}, // x y z
		{bound.maxPoint.x(), bound.minPoint.y(), bound.minPoint.z(), 1.0}, // X y z
		{bound.minPoint.x(), bound.maxPoint.y(), bound.minPoint.z(), 1.0}, // x Y z
		{bound.maxPoint.x(), bound.maxPoint.y(), bound.minPoint.z(), 1.0}, // X Y z

		{bound.minPoint.x(), bound.minPoint.y(), bound.maxPoint.z(), 1.0}, // x y Z
		{bound.maxPoint.x(), bound.minPoint.y(), bound.maxPoint.z(), 1.0}, // X y Z
		{bound.minPoint.x(), bound.maxPoint.y(), bound.maxPoint.z(), 1.0}, // x Y Z
		{bound.maxPoint.x(), bound.maxPoint.y(), bound.maxPoint.z(), 1.0}, // X Y Z
	};

	bool inside = false;
	for (size_t corner_idx = 0; corner_idx < 8; corner_idx++) {
		// Transform vertex
		Vector4f corner = MVP * corners[corner_idx];
		// Check vertex against clip space bounds
		if (abs(corner.x()) < corner.w() &&
			abs(corner.y()) < corner.w() &&
			corner.z() > 0 && corner.z() < corner.w())
			return true;
	}
	return false;
}
