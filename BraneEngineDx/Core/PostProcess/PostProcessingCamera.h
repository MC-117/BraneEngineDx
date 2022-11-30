#pragma once
#ifndef _POSTPROCESSINGCAMERA_H_
#define _POSTPROCESSINGCAMERA_H_

#include "../Camera.h"
#include "PostProcessCameraRender.h"

class PostProcessingCamera : public Camera
{
public:
	Serialize(PostProcessingCamera, Camera);

	bool autoDof = false;
	PostProcessCameraRender postProcessCameraRender;
	Texture2D texture = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear });
	RenderTarget renderTarget = RenderTarget(size.x, size.y, 4, true);
	PostProcessingCamera(string name = "PostProcessingCamera");
	PostProcessingCamera(Material& material, string name = "PostProcessingCamera");

	void setVolumnicLight(DirectLight& light);

	virtual void tick(float deltaTime);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	/*static Shader postProcessingShader;
	static Material postProcessingMaterial;
	static Texture2D ssaoRandTex;
	static bool loadedDefaultResource;*/
	
	//static void loadDefaultResource();
};

#endif // !_POSTPROCESSINGCAMERA_H_
