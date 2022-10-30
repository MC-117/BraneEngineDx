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