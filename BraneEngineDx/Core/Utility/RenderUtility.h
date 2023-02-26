#pragma once

#include "../GraphicType.h"
#include "../RenderCore/CameraData.h"
#include "Utility.h"
#include "Boundings.h"

string getShaderFeatureNames(Enum<ShaderFeature> feature);
const char* getShaderPropertyTypeName(ShaderProperty::Type type);
int getGPUBufferFormatCellSize(GPUBufferFormat format);
int getPixelSize(TexInternalType type, int channels);
bool isFloatPixel(TexInternalType type);
bool frustumCulling(const CameraData& camData, const BoundBox& bound, const Matrix4f& mat);
