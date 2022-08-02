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
