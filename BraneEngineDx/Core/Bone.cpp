#include "Bone.h"
#include "Importer.h"
#include "Camera.h"
#include "Console.h"
#include "RenderCore/RenderCommandList.h"

SerializeInstance(Bone);

Mesh Bone::sphereMesh;
Mesh Bone::coneMesh;
Shader Bone::outLineShader;
Material Bone::outLineMaterial = Material(Bone::outLineShader);
bool Bone::isLoadDefaultResource = false;

Bone::Bone(const string & name) : Actor(name),
	sphereMeshRender(sphereMesh, outLineMaterial),
	coneMeshRender(coneMesh, outLineMaterial)
{
	loadDefaultResource();
	setupFlags = SetupFlags::Transform;
	sphereMeshRender.canCastShadow = false;
	coneMeshRender.canCastShadow = false;
	sphereMeshRender.hidden = true;
	coneMeshRender.hidden = true;
}

void Bone::prerender(SceneRenderData& sceneData)
{
	Actor::prerender(sceneData);
	if (sphereMeshRender.hidden && coneMeshRender.hidden)
		return;
	Matrix4f _transformMat = transformMat;
	_transformMat(0, 0) /= scale.x();
	_transformMat(1, 1) /= scale.y();
	_transformMat(2, 2) /= scale.z();
	unsigned int transID = sceneData.setMeshTransform(_transformMat);
	sphereMeshRender.instanceID = transID;
	for (int i = 0; i < children.size(); i++) {
		Bone* cb = dynamic_cast<Bone*>(children[i]);
		if (cb != NULL) {
			Matrix4f T = Matrix4f::Identity();
			T.block(0, 3, 3, 1) = cb->position.normalized() * 0.5;
			Matrix4f S = Matrix4f::Identity();
			S(0, 0) = cb->position.norm() - 0.5;
			Matrix4f R = Matrix4f::Identity();
			R.block(0, 0, 3, 3) = Quaternionf::FromTwoVectors(Vector3f(1, 0, 0), cb->position.normalized()).toRotationMatrix();
			unsigned int transID = sceneData.setMeshTransform(transformMat * T * R * S);
			coneMeshRender.instanceID = transID;
		}
	}
}

Render * Bone::getRender()
{
	return &coneMeshRender;
}

unsigned int Bone::getRenders(vector<Render*>& renders)
{
	renders.push_back(&coneMeshRender);
	renders.push_back(&sphereMeshRender);
	return 2;
}

void Bone::setHidden(bool value)
{
	sphereMeshRender.hidden = value;
	coneMeshRender.hidden = value;
}

bool Bone::isHidden()
{
	return sphereMeshRender.hidden && coneMeshRender.hidden;
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

void Bone::loadDefaultResource()
{
	if (isLoadDefaultResource)
		return;
	Importer imph = Importer("Engine/Shapes/UnitSphere.fbx");
	if (!imph.getMesh(sphereMesh))
		throw runtime_error("Load Engine/Shapes/UnitSphere.fbx failed");
	Importer impc = Importer("Engine/Shapes/Cone.fbx");
	if (!impc.getMesh(coneMesh))
		throw runtime_error("Load Engine/Shapes/Cone.fbx failed");
	if (!Material::MaterialLoader::loadMaterial(outLineMaterial, "Engine/Shaders/ClearColor.mat"))
		throw runtime_error("Load Engine/Shaders/ClearColor.mat failed");
	isLoadDefaultResource = true;
}
