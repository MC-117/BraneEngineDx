#pragma once
#ifndef _RENDERCOMMANDLIST_H_
#define _RENDERCOMMANDLIST_H_

#include "Utility/Utility.h"
#include "SkeletonMesh.h"
#include "RenderTarget.h"
#include "GPUBuffer.h"
#include "IRenderExecution.h"
#include "MorphTargetWeight.h"

class Camera;
class Render;
class Light;

struct RenderCommand
{
	Material* material;
	Camera* camera;
	MeshPart* mesh;
	list<Particle>* particles;
	list<IBufferBinding*> bindings;
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
	unsigned int setMeshTransform(const Matrix4f& transformMat);
	unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	void* getMeshPartTransform(MeshPart* meshPart, Material* material);
	void* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	void* setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);

	unsigned int setStaticMeshTransform(const Matrix4f& transformMat);
	unsigned int setStaticMeshTransform(const vector<Matrix4f>& transformMats);
	void* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	void* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	void* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);
	void cleanStaticMeshTransform(unsigned int base, unsigned int count);
	void cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	bool setRenderCommand(const RenderCommand& cmd, bool isStatic = false);

	void setUpdateStatic();
	bool willUpdateStatic();

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

		GPUBuffer transformBuffer = GPUBuffer(GB_Struct, 16 * sizeof(float));
		GPUBuffer transformIndexBuffer = GPUBuffer(GB_Vertex, sizeof(unsigned int));

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
		void cleanStatic(unsigned int base, unsigned int count);
		void cleanPartStatic(MeshPart* meshPart = NULL, Material* material = NULL);
	};
	struct MeshDataRenderPack : public IRenderPack
	{
		map<MeshPart*, MeshTransformIndex*> meshParts;
		vector<DrawElementsIndirectCommand> cmds;
		list<IBufferBinding*> bindings;
		unsigned int totaltransformCount = 0;
		GPUBuffer transformBuffer = GPUBuffer(GB_Storage, 16 * sizeof(float));

		void setRenderData(MeshPart* part, MeshTransformIndex* data, const list<IBufferBinding*>& bindings);
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
		GPUBuffer particleBuffer = GPUBuffer(GB_Struct, sizeof(Particle));

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
		float radius;
	};
	struct LightDataPack
	{
		DirectLightData directLightData;
		vector<PointLightData> pointLightDatas;
		GPUBuffer directLightBuffer = GPUBuffer(GB_Constant, sizeof(DirectLightData));
		GPUBuffer pointLightBuffer = GPUBuffer(GB_Struct, sizeof(PointLightData));

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
	MeshTransformDataPack meshTransformDataPack;
	ParticleDataPack particleDataPack;
	LightDataPack lightDataPack;
};

#endif // !_RENDERCOMMANDLIST_H_