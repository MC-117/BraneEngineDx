#include "CameraManager.h"

#include "GUI/GUISurface.h"

void CameraManager::setCamera(Camera* camera, const Name& tag)
{
    if (tag == CameraTag::main) {
        if (guiSurface)
            guiSurface->bindCamera(camera);
    }
    
    if (camera == NULL)
        return;

    if (camera->manager) {
        if (camera->manager == this)
            return;
        camera->manager->removeCamera(camera);
    }
    camera->manager = this;
    
    auto iter = cameraMap.find(camera);
    if (iter == cameraMap.end()) {
        cameraMap[camera] = tag;
    }
    else {
        Name& oldTag = iter->second;
        if (oldTag == tag) {
            return;
        }
        if (!oldTag.isNone()) {
            namedCameras.erase(oldTag);
        }
        oldTag = tag;
    }

    if (!tag.isNone()) {
        auto iter = namedCameras.find(tag);
        if (iter == namedCameras.end()) {
            namedCameras[tag] = camera;
        }
        else {
            cameraMap[iter->second] = Name::none;
            iter->second = camera;
        }
    }
}

Camera* CameraManager::getCamera(const Name& tag) const
{
    if (tag.isNone())
        return NULL;
    auto iter = namedCameras.find(tag);
    if (iter == namedCameras.end()) {
        return NULL;
    }
    else {
        return iter->second;
    }
}

Name CameraManager::getCameraTag(Camera* camera) const
{
    if (camera) {
        auto iter = cameraMap.find(camera);
        if (iter != cameraMap.end()) {
            return iter->second;
        }
    }
    return Name::none;
}

void CameraManager::removeCamera(Camera* camera)
{
    if (camera == NULL || camera->manager != this)
        return;

    camera->manager = NULL;
    
    auto iter = cameraMap.find(camera);
    if (iter != cameraMap.end()) {
        if (!iter->second.isNone()) {
            namedCameras.erase(iter->second);
            if (iter->second == CameraTag::main && guiSurface) {
                guiSurface->bindCamera(NULL);
            }
        }
        cameraMap.erase(iter);
    }
}

void CameraManager::setGUISurface(GUISurface* newGUISurface)
{
    if (guiSurface == newGUISurface)
        return;
    if (guiSurface) {
        guiSurface->bindCamera(NULL);
    }
    guiSurface = newGUISurface;
    if (guiSurface) {
        guiSurface->bindCamera(getCamera(CameraTag::main));
    }
}

GUISurface* CameraManager::getGUISurface() const
{
    return guiSurface;
}
