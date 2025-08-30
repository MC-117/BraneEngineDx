#pragma once

#include "MeshRender.h"

class ENGINE_API DecalRender : public MeshRender
{
public:
    DecalRender();

    virtual Enum<DecalMask> getDecalMask() const;
    virtual void setDecalMask(Enum<DecalMask> mask);

    virtual void setDecalBoxMesh();

    virtual void render(RenderInfo& info);

    virtual bool getCanCastShadow() const;

    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    Enum<DecalMask> decalMask;
};
