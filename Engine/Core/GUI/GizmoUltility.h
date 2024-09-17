#pragma once

#include "Gizmo.h"

ENGINE_API float getFitIconSize(GizmoInfo& info);

struct CameraData;
ENGINE_API void drawFrustum(Gizmo& gizmo, const CameraData& data, const Color& color);