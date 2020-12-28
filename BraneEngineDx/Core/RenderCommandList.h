#pragma once
#ifndef _RENDERCOMMANDLIST_H_
#define _RENDERCOMMANDLIST_H_

#include "Utility.h"
#include "SkeletonMesh.h"
#include "RenderTarget.h"
#include "IRenderExecution.h"

class Camera;
class Render;
class Light;

struct RenderCommand
{
	Material* material;
	Camera* camera;
	MeshPart* mesh;
	list<Particle>* particles;
};

struct TransDataTag
{
	Render* render;
	Mesh* mesh;

	bool operator<(const TransDataTag& tag) const;
};

struct TransTag
{
	Material* mat;
	MeshPart* meshPart;

	bool operator<(const TransTag& tag) const;
};

class RenderCommandList
{
public:
	void setLight(Render* lightRender);
	static unsigned int setMeshTransform(const Matrix4f& transformMat);
	static unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	static void* getMeshPartTransform(MeshPart* meshPart, Material* material);
	static void* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	static void* setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);

	static unsigned int setStaticMeshTransform(const Matrix4f& transformMat);
	static unsigned int setStaticMeshTransform(const vector<Matrix4f>& transformMats);
	static void* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	static void* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	static void* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);
	static void cleanStaticMeshTransform(unsigned int base, unsigned int count);
	static void cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	bool setRenderCommand(const RenderCommand& cmd, bool isStatic = false);

	static void setUpdateStatic();
	static bool willUpdateStatic();

	void excuteCommand();
	void resetCommand();
protected:
	bool setRenderCommand(const RenderCommand& cmd, bool isStatic, bool autoFill);
protected:
	struct shader_order
	{
		bool operator()(const ShaderProgram* s0, const ShaderProgram* s1) const
		{
			if (s0->renderOrder < s1->renderOrder)
				return true;
			if (s0->renderOrder == s1->renderOrder && s0 < s1)
				return true;
			return false;
		}
	};
	struct camera_order
	{
		bool operator()(const Camera* s0, const Camera* s1) const;
	};
	struct MeshTransformData
	{
		vector<Matrix4f> transforms;
		unsigned int batchCount = 0;

		unsigned int setMeshTransform(const Matrix4f& transformMat);
		unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
		bool updataMeshTransform(const Matrix4f& transformMat, unsigned int base);
		bool updataMeshTransform(const vector<Matrix4f>& transformMats, unsigned int base);
		void clean();
		bool clean(unsigned int base, unsigned int count);
	};
	struct MeshTransformIndex
	{
		vector<unsigned int> indices;
		unsigned int batchCount = 0;
		unsigned int indexBase = 0;
	};
	struct IRenderPack
	{
		IRenderExecution* vendorRenderExecution = NULL;
		virtual ~IRenderPack();
		virtual void excute() = 0;
		virtual void newVendorRenderExecution();
	};
	struct ParticleData;
	struct MeshTransformDataPack
	{
		unsigned int totalTransformIndexCount = 0;
		unsigned int staticTotalTransformIndexCount = 0;
		bool willStaticUpdate = false;
		bool staticUpdate = true;

		MeshTransformData meshTransformData;
		map<TransTag, MeshTransformIndex> meshTransformIndex;

		MeshTransformData staticMeshTransformData;
		map<TransTag, MeshTransformIndex> staticMeshTransformIndex;

		GPUBuffer transformBuffer = GPUBuffer(GB_Storage, 16 * sizeof(float));
		GPUBuffer transformIndexBuffer = GPUBuffer(GB_Storage, sizeof(unsigned int));

		void setUpdateStatic();

		unsigned int setMeshTransform(const Matrix4f& transformMat);
		unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
		MeshTransformIndex* getMeshPartTransform(MeshPart* meshPart, Material* material);
		MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
		MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex);
		
		unsigned int setStaticMeshTransform(const Matrix4f& transformMat);
		unsigned int setStaticMeshTransform(const vector<Matrix4f>& transformMats);
		MeshTransformIndex* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
		MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
		MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex);
		void uploadTransforms();
		void bindTransforms();
		void clean();
		void cleanStatic();
		void cleanStatic(unsigned int base, unsigned int count); // TO-FIX
		void cleanPartStatic(MeshPart* meshPart = NULL, Material* material = NULL);
	};
	struct MeshDataRenderPack : public IRenderPack
	{
		map<MeshPart*, MeshTransformIndex*> meshParts;
		vector<DrawElementsIndirectCommand> cmds;
		unsigned int totaltransformCount = 0;
		GPUBuffer transformBuffer = GPUBuffer(GB_Storage, 16 * sizeof(float));

		void setRenderData(MeshPart* part, MeshTransformIndex* data);
		virtual void excute();
	};
	struct ParticleData
	{
		vector<Particle> particles;
		unsigned int batchCount = 0;
		unsigned int particleBase = 0;
	};
	struct ParticleDataPack
	{
		unsigned int totalParticleCount = 0;
		map<Material*, ParticleData> particles;
		GPUBuffer particleBuffer = GPUBuffer(GB_Storage, sizeof(Particle));

		ParticleData* setParticles(Material* material, const list<Particle>& particles);
		ParticleData* setParticles(Material* material, const vector<Particle>& particles);
		void uploadParticles();
		void bindParticles();
		void clean();
	};
	struct ParticleRenderPack : public IRenderPack
	{
		ParticleData* particleData;
		DrawArraysIndirectCommand cmd;
		GPUBuffer cmdBuffer = GPUBuffer(GB_Command, sizeof(DrawElementsIndirectCommand));

		virtual void excute();
	};
	
	struct DirectLightData
	{
		Vector3f direction;
		float intensity = 0;
		Matrix4f lightSpaceMat;
		Vector3f color;
		unsigned int pointLightCount = 0;
	};
	struct PointLightData
	{
		Vector3f position;
		float intensity = 0;
		Vector3f color;
		float attenuation;
	};
	struct LightDataPack
	{
		DirectLightData directLightData;
		vector<PointLightData> pointLightDatas;
		GPUBuffer lightBuffer = GPUBuffer(GB_Storage, sizeof(PointLightData));

		RenderTarget* shadowTarget = NULL;

		void setLight(Light* light);
		void uploadLight();
		void bindLight();
		void clean();
	};
public:
	set<Render*> lightRenders;
	list<IRenderPack*> renderPacks;
	map<Camera*, map<ShaderProgram*, map<Material*, map<MeshData*, IRenderPack*>>, shader_order>, camera_order> commandList;
	//map<Camera*, map<Shader*, map<Material*, ParticleRenderPack>, shader_order>, camera_order> particleCommandList;
	static MeshTransformDataPack meshTransformDataPack;
	static ParticleDataPack particleDataPack;
	static LightDataPack lightDataPack;
};

#endif // !_RENDERCOMMANDLIST_H_