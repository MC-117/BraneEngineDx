#include "SpotLightEditor.h"
#include "../Camera.h"
#include "../Utility/MathUtility.h"

RegistEditor(SpotLight);

Texture2D* SpotLightEditor::spotLightIcon = NULL;

void SpotLightEditor::setInspectedObject(void* object)
{
    spotLight = dynamic_cast<SpotLight*>((Object*)object);
    PointLightEditor::setInspectedObject(spotLight);
}

void SpotLightEditor::onPersistentGizmo(GizmoInfo& info)
{
    PointLightEditor::onPersistentGizmo(info);
    Vector3f pos = light->getPosition(WORLD);
    Vector3f forward = light->getForward(WORLD);
    info.gizmo->drawLine(pos, pos + forward * 10, light->color);
}

void SpotLightEditor::onLocalLightShapeGizmo(GizmoInfo& info)
{
    const float coneDegree = spotLight->getConeAngle() * Math::Deg2Rad;
    const float coneLength = spotLight->getRadius() * std::cos(coneDegree);
    const float coneRadius = spotLight->getRadius() * std::sin(coneDegree);
    info.gizmo->drawPyramidX(Vector3f::UnitX() * coneLength, coneRadius, -coneLength, 36, pointLight->getTransformMat(), pointLight->color);
}

void SpotLightEditor::onLightGUI(EditorInfo& info)
{
    PointLightEditor::onLightGUI(info);
    float coneAngle = spotLight->getConeAngle();
    if (ImGui::DragFloat("ConeAngle", &coneAngle, 0.01f)) {
        spotLight->setConeAngle(coneAngle);
    }
}

Texture2D* SpotLightEditor::getIcon()
{
    if (spotLightIcon == NULL)
        spotLightIcon = getAssetByPath<Texture2D>("Engine/Icons/SpotLight_Icon.png");
    return spotLightIcon;
}