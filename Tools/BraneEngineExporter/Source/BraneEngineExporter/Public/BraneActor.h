#pragma once

#include "BraneMaterial.h"
#include <vector>
#include <set>

namespace Brane
{
	using namespace std;
	
	class Mesh : public IWritable
	{
	public:
		BESerialize(AssetSearch);
		
		string path;

		Mesh() = default;
		virtual ~Mesh() = default;

		virtual void write(ostream& os);
	};

	typedef shared_ptr<Mesh> MeshRef;

	class Transform;
	typedef shared_ptr<Transform> TransformRef;

	class Transform : public IWritable
	{
	public:
		BESerialize(Transform);
		
		string name;
		Vector3f position;
		Quaternionf rotation;
		Vector3f scale = Vector3f { 1.0f, 1.0f, 1.0f };

		Transform* parent = nullptr;
		set<TransformRef> children;

		Transform() = default;
		virtual ~Transform() = default;

		virtual void addChild(TransformRef child);
		virtual void unparent();

		virtual void write(ostream& os);

		virtual bool writeToFile(const string& path);
	};

	class MeshActor : public Transform
	{
	public:
		BESerialize(MeshActor);
		
		MeshRef mesh;

		MeshActor() = default;
		virtual ~MeshActor() = default;
		
		vector<pair<string, MaterialRef>> materials;

		void setMaterial(const string& slotName, MaterialRef material);

		virtual void write(ostream& os);
	};

	typedef shared_ptr<MeshActor> MeshActorRef;

	class Camera : public Transform
	{
	public:
		BESerialize(Camera);

		float fov = 90;
		float distance = 0;
		Color clearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);

		Material ssaoMat;
		Material dofMat;
		Material bloomMat;
		Material volumetricLightMat;
		Material toneMapMat;

		Camera();
		virtual ~Camera() = default;

		virtual void write(ostream& os);
	};

	typedef shared_ptr<Camera> CameraRef;

	class Light : public Transform
	{
	public:
		BESerialize(Light);

		float intensity = 1;
		float attenuation = 1;
		Color color = { 255, 255, 255, 255 };

		Light() = default;
		virtual ~Light() = default;

		virtual void write(ostream& os);
	};

	class DirectLight : public Light
	{
	public:
		BESerialize(DirectLight);
	};

	class PointLight : public Light
	{
	public:
		BESerialize(PointLight);

		float radius = 10;
		
		virtual void write(ostream& os);
	};

	class DebugCamera : public Camera
	{
	public:
		BESerialize(DebugCamera);

		DebugCamera();

		virtual void write(ostream& os);
	};

	class SkySphere : public Transform
	{
	public:
		BESerialize(SkySphere);

		SkySphere();

		virtual void write(ostream& os);
	};

	class CaptureProbe : public Transform
	{
	public:
		BESerialize(CaptureProbe);

		float radius = 0;

		CaptureProbe() = default;

		virtual void write(ostream& os);
	};

	class World : public Transform
	{
	public:
		BESerialize(World);
		
		World() = default;
		virtual ~World() = default;
	};
}
