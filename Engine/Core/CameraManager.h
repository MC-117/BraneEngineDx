#pragma once

#include "Camera.h"

class ENGINE_API CameraManager : public ICameraManager
{
public:
    virtual void setCamera(Camera* camera, const Name& tag);
    virtual Camera* getCamera(const Name& tag) const;
	virtual Name getCameraTag(Camera* camera) const;

    virtual void removeCamera(Camera* camera);
    
    virtual void setGUISurface(GUISurface* guiSurface);
    virtual GUISurface* getGUISurface() const;
protected:
    unordered_map<Name, Camera*> namedCameras;
    unordered_map<Camera*, Name> cameraMap;
    GUISurface* guiSurface = NULL;
};