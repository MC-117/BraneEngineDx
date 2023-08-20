#pragma once

#include "../GraphicType.h"
#include "../RenderCore/CameraData.h"
#include "Utility.h"
#include "Boundings.h"

ShaderFeature getShaderFeatureFromName(const string& name);
string getShaderFeatureName(ShaderFeature feature);
string getShaderFeatureNames(Enum<ShaderFeature> feature);
const char* getShaderPropertyTypeName(ShaderProperty::Type type);
const char* getShaderExtension(ShaderStageType type);
int getGPUBufferFormatCellSize(GPUBufferFormat format);
int getPixelSize(TexInternalType type, int channels);
bool isFloatPixel(TexInternalType type);
bool frustumCulling(const CameraData& camData, const BoundBox& bound, const Matrix4f& mat);

const Vector3f& getCubeFaceForwardVector(CubeFace face);
const Vector3f& getCubeFaceLeftwardVector(CubeFace face);
const Vector3f& getCubeFaceUpwardVector(CubeFace face);

const Matrix4f& getCubeFaceProjectionMatrix(float zNear, float zFar);
const Matrix4f& getCubeFaceViewMatrix(CubeFace face, const Vector3f& position = Vector3f::Zero());
