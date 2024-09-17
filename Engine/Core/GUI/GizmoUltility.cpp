#include "GizmoUltility.h"
#include "../Camera.h"

float getFitIconSize(GizmoInfo& info)
{
    if (info.camera == NULL)
        return 50;
    float size = max(info.camera->size.x, info.camera->size.y);
    size *= 0.025f;
    size = max(size, 10);
    return size;
}

void drawFrustum(Gizmo& gizmo, const CameraData& data, const Color& color)
{
    float vLen = data.zFar * tan(data.fovy * PI / 180 * 0.5f);
    float hLen = vLen * data.aspect;
    Vector3f worldPos = data.cameraLoc;
    Vector3f upVec = data.cameraUp;
    Vector3f leftVec = data.cameraLeft;
    Vector3f forVec = data.cameraDir;

    Vector3f vVec = upVec * vLen;
    Vector3f hVec = leftVec * hLen;

    Vector3f farPoint = worldPos + forVec * data.zFar;
    Vector3f nearPoint = worldPos + forVec * data.zNear;

    Vector3f corner[4] =
    {
        farPoint - vVec - hVec,
        farPoint - vVec + hVec,
        farPoint + vVec - hVec,
        farPoint + vVec + hVec,
    };

    gizmo.drawLine(corner[0], worldPos, color);
    gizmo.drawLine(corner[1], worldPos, color);
    gizmo.drawLine(corner[2], worldPos, color);
    gizmo.drawLine(corner[3], worldPos, color);
    gizmo.drawLine(corner[0], corner[1], color);
    gizmo.drawLine(corner[0], corner[2], color);
    gizmo.drawLine(corner[3], corner[1], color);
    gizmo.drawLine(corner[3], corner[2], color);
}
