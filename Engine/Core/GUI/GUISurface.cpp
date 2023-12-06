#include "GUISurface.h"

#include "../Engine.h"

GUISurface::GUISurface(const string& name) : gizmo(name + "Gizmo")
{
}

GUISurface::~GUISurface()
{
    if (camera)
        camera->guiSurface = NULL;
}

void GUISurface::bindCamera(Camera* newCamera)
{
    if (camera)
        camera->guiSurface = NULL;
    camera = newCamera;
    if (newCamera && surfaceSize.x() * surfaceSize.y())
        newCamera->setSize({ surfaceSize.x(), surfaceSize.y() });
}

void GUISurface::setSize(const Vector2i& size)
{
    surfaceSize = size;
    if (camera && surfaceSize.x() * surfaceSize.y())
        camera->setSize({ size.x(), size.y() });
}

bool GUISurface::isValid() const
{
    return camera;
}

void GUISurface::gizmoUpdate()
{
    Camera* pCamera = camera;
    if (pCamera == NULL)
        return;
    if (Engine::getInput().getCursorHidden()) {
        gizmo.setCameraControl(Gizmo::CameraControlMode::None);
    }
    else {
        gizmo.setCameraControl(Gizmo::CameraControlMode::Free);
    }
    gizmo.onUpdate(*pCamera);
}

void GUISurface::gizmoFrame(ImDrawList* drawList, Object* rootObject)
{
    gizmo.beginFrame(drawList);
    gizmo.onGUI(rootObject);
    gizmo.endFrame();
}

void GUISurface::gizmoRender2D(ImDrawList* drawList)
{
    gizmo.onRender2D(drawList);
    gizmo.reset();
}

void GUISurface::gizmoRender3D(RenderInfo& info)
{
    gizmo.onRender3D(info);
}

Camera* GUISurface::getCamera()
{
    return camera;
}

CameraRender* GUISurface::getCameraRender()
{
    Camera* pCamera = camera;
    return pCamera ? &pCamera->cameraRender : NULL;
}

Texture* GUISurface::getSceneTexture()
{
    Camera* pCamera = camera;
    return pCamera ? pCamera->cameraRender.getSceneTexture() : NULL;
}

RenderTarget* GUISurface::getRenderTarget()
{
    Camera* pCamera = camera;
    return pCamera ? &pCamera->cameraRender.getRenderTarget() : NULL;
}

GUISurface& GUISurface::getFullScreenGUISurface()
{
    static GUISurface fullScreenSurface("FullScreen");
    return fullScreenSurface;
}

GUISurface& GUISurface::getMainViewportGUISurface()
{
    static GUISurface mainViewportSurface("MainViewport");
    return mainViewportSurface;
}

GUISurface& GUISurface::getMainGUISurface()
{
    return getMainViewportGUISurface().isValid() ? getMainViewportGUISurface() : getFullScreenGUISurface();
}
