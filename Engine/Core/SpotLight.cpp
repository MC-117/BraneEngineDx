#include "SpotLight.h"

SerializeInstance(SpotLight);

SpotLight::SpotLight(const string& name, Color color, float intensity, float attenuation, float radius, float coneAngle)
    : PointLight(name, color, intensity, attenuation, radius)
{
    setConeAngle(coneAngle);
}

void SpotLight::setConeAngle(float coneAngle)
{
    this->coneAngle = std::clamp(coneAngle, 0.0f, 80.0f);
}

float SpotLight::getConeAngle() const
{
    return coneAngle;
}

Serializable* SpotLight::instantiate(const SerializationInfo& from)
{
    return new SpotLight(from.name);
}

bool SpotLight::deserialize(const SerializationInfo& from)
{
    if (!PointLight::deserialize(from)) {
        return false;
    }
    from.get("coneAngle", coneAngle);
    return true;
}

bool SpotLight::serialize(SerializationInfo& to)
{
    if (!PointLight::serialize(to)) {
        return false;
    }
    to.set("coneAngle", coneAngle);
    return true;
}
