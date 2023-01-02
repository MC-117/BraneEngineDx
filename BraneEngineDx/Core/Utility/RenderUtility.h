#pragma once

#include "../GraphicType.h"
#include "../RenderCore/CameraData.h"
#include "Utility.h"

string getShaderFeatureNames(Enum<ShaderFeature> feature);
int getGPUBufferFormatCellSize(GPUBufferFormat format);
int getPixelSize(TexInternalType type, int channels);
bool isFloatPixel(TexInternalType type);
bool frustumCulling(const CameraData& camData, const Range<Vector3f>& bound, const Matrix4f& mat);
