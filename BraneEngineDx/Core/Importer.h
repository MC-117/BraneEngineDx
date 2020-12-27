#pragma once
#ifndef _IMPORTER_H_
#define _IMPORTER_H_

#include <assimp\cimport.h>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "SkeletonMesh.h"
#include "Texture2D.h"
#include "AnimationClip.h"

class Importer
{
public:
	Importer();
	Importer(const string& file, unsigned int flag = aiProcessPreset_TargetRealtime_MaxQuality);
	~Importer();

	bool isLoad();
	bool isSkeletonMesh();
	bool hasAnimation();
	bool load(const string& file, unsigned int flag);
	bool getMesh(Mesh& mesh);
	bool getSkeletonMesh(SkeletonMesh& mesh);
	bool getAnimation(vector<AnimationClipData*>& datas, bool doMerge = false, bool swapYZ = false);

	string getError();

	static Mesh* loadMesh(const string& file, unsigned int flag);
protected:
	const aiScene* scene = NULL;
	vector<pair<string, MeshPart>> meshParts;
	MeshPart totalMesh;
	Range<Vector3f> bound;

	struct VertexData {

		aiNode* node;
		vector<pair<aiMesh*, aiVertexWeight*>> weight;
	};

	int nMesh = 0, nFace = 0, nVert = 0, nMorph, nMorphVert = 0;
	bool hasBone = false, pre = false;
	vector<vector<vector<pair<unsigned int, float>>>> vertexWeights;
	map<string, unsigned int> boneName;
	vector<aiMesh*> meshSortedList;
	vector<string> morphName;
	vector<pair<aiNode*, Matrix4f>> boneList;

	string errorString;

	void toMatrix4f(Matrix4f& a, aiMatrix4x4& b);

	bool processNodes(aiNode* node, const Matrix4f& gTransform = Matrix4f::Identity());
	bool preprocess();
	bool toMeshParts();
	bool toSkeletonMeshParts();
};

#endif // !_IMPORTER_H_
