#pragma once

#include "../Base.h"

class Bone;
class Skeleton;

class ENGINE_API BoneConstraint : public Serializable
{
	friend class Skeleton;
public:
	Serialize(BoneConstraint,);

	int index = -1;
	bool enable = true;

	virtual bool isValid() const;
	virtual bool isEnable() const;
	virtual bool setup();
	virtual void solve();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	void setSerializeUseBoneName(bool value);
protected:
	Skeleton* skeleton = NULL;
	bool serializeUseBoneName = false;
};