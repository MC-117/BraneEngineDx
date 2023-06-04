#include "Bone.h"
#include "Importer/AssimpImporter.h"
#include "Camera.h"
#include "Console.h"
#include "RenderCore/RenderCommandList.h"

SerializeInstance(Bone);

Bone::Bone(const string & name) : Actor(name)
{
	setupFlags = SetupFlags::Transform;
}

int Bone::getBoneIndex() const
{
	return boneIndex;
}

Serializable * Bone::instantiate(const SerializationInfo & from)
{
	Bone* bone = new Bone(from.name);
	ChildrenInstantiateObject(from, bone, IR_Default);
	return bone;
}

bool Bone::deserialize(const SerializationInfo & from)
{
	return Actor::deserialize(from);
}

bool Bone::serialize(SerializationInfo & to)
{
	SerializationInfo* child = to.add("children");
	if (!Transform::serialize(to))
		return false;

	for (int i = 0; i < children.size(); i++) {
		if (children[i]->getSerialization().type != "Bone") {
			Console::log(children[i]->name);
		}
	}
	/*if (child != NULL)
		for (int i = 0; i < children.size(); i++) {
			if (children[i]->getSerialization().type != "Bone") {
				SerializationInfo* info = child->add(children[i]->name);
				if (info != NULL) {
					children[i]->serialize(*info);
				}
			}
		}*/
	return true;
}

Bone::Bone(int boneIndex, const string& name) : Actor(name), boneIndex(boneIndex)
{
	setupFlags = SetupFlags::Transform;
}
