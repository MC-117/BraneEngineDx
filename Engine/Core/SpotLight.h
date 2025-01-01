#pragma once

#include "PointLight.h"

class ENGINE_API SpotLight : public PointLight
{
public:
    Serialize(SpotLight, PointLight);

    SpotLight(const string& name, Color color = { 255, 255, 255, 255 }, float intensity = 1, float attenuation = 1, float radius = 10, float coneAngle = 45);

    void setConeAngle(float coneAngle);
    float getConeAngle() const;

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    float coneAngle = 45;
};