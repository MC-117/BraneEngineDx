#pragma once

#include "../GraphicType.h"
#include "../RenderCore/CameraData.h"
#include "Utility.h"
#include "Boundings.h"

ENGINE_API ShaderFeature getShaderFeatureFromName(const string& name);
ENGINE_API string getShaderFeatureName(ShaderFeature feature);
ENGINE_API string getShaderFeatureNames(Enum<ShaderFeature> feature);
ENGINE_API const char* getShaderPropertyTypeName(ShaderProperty::Type type);
ENGINE_API const char* getShaderExtension(ShaderStageType type);
ENGINE_API RenderStage enumRenderStage(uint16_t stageNum);
ENGINE_API const char* getRenderStageName(RenderStage stage);
ENGINE_API int getGPUBufferFormatCellSize(GPUBufferFormat format);
ENGINE_API int getPixelSize(TexInternalType type, int channels);
ENGINE_API bool isFloatPixel(TexInternalType type);
ENGINE_API bool frustumCulling(const CameraData& camData, const BoundBox& bound, const Matrix4f& mat);

ENGINE_API const Vector3f& getCubeFaceForwardVector(CubeFace face);
ENGINE_API const Vector3f& getCubeFaceLeftwardVector(CubeFace face);
ENGINE_API const Vector3f& getCubeFaceUpwardVector(CubeFace face);

ENGINE_API Matrix4f&& getCubeFaceProjectionMatrix(float zNear, float zFar);
ENGINE_API Matrix4f&& getCubeFaceViewMatrix(CubeFace face, const Vector3f& position = Vector3f::Zero());
