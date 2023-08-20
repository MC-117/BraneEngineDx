#include "AssimpImporter.h"
#include "../IVendor.h"
#include "../Console.h"
#include "../Utility/MathUtility.h"
#include "../Asset.h"

ImporterRegister<AssimpImporter> objImporter(".obj");
ImporterRegister<AssimpImporter> fbxImporter(".fbx");
ImporterRegister<AssimpImporter> pmxImporter(".pmx");

AssimpImporter::AssimpImporter()
{
}

AssimpImporter::AssimpImporter(const string & file, unsigned int flag)
{
	load(file, flag);
}

AssimpImporter::~AssimpImporter()
{
	if (scene != NULL)
		aiReleaseImport(scene);
	scene = NULL;
}

bool AssimpImporter::isLoad()
{
	return scene != NULL && pre;
}

bool AssimpImporter::isSkeletonMesh()
{
	return hasBone;
}

bool AssimpImporter::hasAnimation()
{
	return scene != NULL && scene->HasAnimations();
}

bool AssimpImporter::load(const string & file, unsigned int flag)
{
	if (scene != NULL)
		aiReleaseImport(scene);
	aiPropertyStore* props = aiCreatePropertyStore();
	aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
	aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 1);
	filesystem::path path = filesystem::u8path(file);
	scene = aiImportFileExWithProperties(path.generic_u8string().c_str(), flag ^ aiProcess_RemoveRedundantMaterials, NULL, props);
	aiReleasePropertyStore(props);
	if (scene == NULL) {
		errorString = aiGetErrorString();
		Console::error(errorString);
		return false;
	}
	return preprocess();
}

bool AssimpImporter::getMesh(Mesh & mesh)
{
	if (!isLoad())
		return false;
	if (!totalMesh.isValid())
		if (!toMeshParts())
			return false;
	mesh.meshParts.clear();
	for (int i = 0; i < meshParts.size(); i++) {
		mesh.addMeshPart(meshParts[i].first, meshParts[i].second);
	}
	mesh.resize(3, totalMesh.elementCount, totalMesh.vertexCount);
	mesh.setTotalMeshPart(totalMesh);
	mesh.bound = bound;
	return true;
}

bool AssimpImporter::getSkeletonMesh(SkeletonMesh & mesh)
{
	if (!isLoad())
		return false;
	if (!totalMesh.isValid())
		if (!toSkeletonMeshParts())
			return false;
	mesh.meshParts.clear();
	mesh.partHasMorph.resize(nMesh);
	for (int i = 0; i < meshParts.size(); i++) {
		mesh.addMeshPart(meshParts[i].first, meshParts[i].second);
		mesh.partHasMorph[i] = meshSortedList[i]->mNumAnimMeshes > 0;
	}
	mesh.resize(3, totalMesh.elementCount, totalMesh.vertexCount);
	mesh.setTotalMeshPart(totalMesh);
	mesh.bound = bound;
	mesh.skeletonData.boneName = boneName;
	mesh.skeletonData.boneList.resize(boneList.size());
	for (int i = 0; i < boneList.size(); i++) {
		mesh.skeletonData.boneList[i].index = i;
		mesh.skeletonData.boneList[i].name = boneList[i].first->mName.C_Str();
		mesh.skeletonData.boneList[i].offsetMatrix = boneList[i].second;
		toMatrix4f(mesh.skeletonData.boneList[i].transformMatrix, boneList[i].first->mTransformation);
		mesh.skeletonData.boneList[i].skeletonData = &mesh.skeletonData;
		if (boneList[i].first->mParent != NULL) {
			mesh.skeletonData.boneList[i].setParent(&mesh.skeletonData.boneList[boneName[boneList[i].first->mParent->mName.C_Str()]]);
		}
	}
	mesh.skeletonData.rootBone = &mesh.skeletonData.boneList[0];
	mesh.morphName = morphName;
	return true;
}

bool AssimpImporter::getAnimation(vector<AnimationClipData*> & clips, bool doMerge, bool swapYZ)
{
	if (!isLoad())
		return false;
	if (!scene->HasAnimations())
		return false;
	struct Tag {
		aiNodeAnim *pos = 0, *rot = 0, *sca = 0;
	};
	for (int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* amin = scene->mAnimations[i];
		double dur = amin->mDuration / amin->mTicksPerSecond;
		AnimationClipData* animData = new AnimationClipData(amin->mName.C_Str());
		if (amin->mNumMorphMeshChannels > 0) {
			aiMeshMorphAnim* manim = amin->mMorphMeshChannels[0];
			for (int k = 0; k < manim->mNumKeys; k++) {
				aiMeshMorphKey& key = manim->mKeys[k];
				for (int v = 0; v < key.mNumValuesAndWeights; v++) {
					Curve<float, float>& curve = animData->addMorphAnimationData(key.mValues[v]);
					curve.duration = dur;
					curve.insert(key.mTime / amin->mTicksPerSecond,
						CurveValue<float>(CurveValue<float>::Linear, key.mWeights[v]));
				}
			}
		}
		map<string, Tag> merge;
		for (int c = 0; c < amin->mNumChannels; c++) {
			aiNodeAnim* nanim = amin->mChannels[c];
			if (doMerge) {
				string tempName = nanim->mNodeName.C_Str();
				string str = "_$AssimpFbx$_";
				size_t pos = tempName.find(str);
				if (pos != -1) {
					string name = tempName.substr(0, pos);
					string tag = tempName.substr(pos + str.size());
					auto iter = merge.find(name);
					Tag *d;
					if (iter == merge.end())
						d = &merge.insert(pair<string, Tag>(name, Tag())).first->second;
					else
						d = &iter->second;
					if (tag == "Translation")
						d->pos = nanim;
					if (tag == "Rotation")
						d->rot = nanim;
					if (tag == "Scaling")
						d->sca = nanim;
					continue;
				}
			}
			TransformAnimationData& tad = animData->addAnimatinData(nanim->mNodeName.C_Str());
			tad.positionCurve.duration = dur;
			for (int k = 0; k < nanim->mNumPositionKeys; k++) {
				aiVector3D& pos = nanim->mPositionKeys[k].mValue;
				if (swapYZ)
					tad.positionCurve.insert(nanim->mPositionKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(pos.x, -pos.z, pos.y)));
				else
					tad.positionCurve.insert(nanim->mPositionKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(pos.x, pos.y, pos.z)));
			}
			tad.rotationCurve.duration = dur;
			for (int k = 0; k < nanim->mNumRotationKeys; k++) {
				aiQuaternion& rot = nanim->mRotationKeys[k].mValue;
				if (swapYZ)
					tad.rotationCurve.insert(nanim->mRotationKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Quaternionf>(CurveValue<Quaternionf>::Linear, Quaternionf::FromTwoVectors(Vector3f(0, -1, 0), Vector3f(1, 0, 0)) * Quaternionf(rot.w, rot.x, -rot.z, rot.y)));
				else
					tad.rotationCurve.insert(nanim->mRotationKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Quaternionf>(CurveValue<Quaternionf>::Linear, Quaternionf(rot.w, rot.x, rot.y, rot.z)));
			}
			tad.scaleCurve.duration = dur;
			for (int k = 0; k < nanim->mNumScalingKeys; k++) {
				aiVector3D& sca = nanim->mScalingKeys[k].mValue;
				if (swapYZ)
					tad.scaleCurve.insert(nanim->mScalingKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(sca.x, sca.z, sca.y)));
				else
					tad.scaleCurve.insert(nanim->mScalingKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(sca.x, sca.y, sca.z)));
			}
		}
		for (auto b = merge.begin(), e = merge.end(); b != e; b++) {
			aiNodeAnim* main = NULL;
			if (b->second.pos != NULL) {
				main = b->second.pos;
			}
			else if (b->second.rot != NULL) {
				main = b->second.rot;
			}
			else if (b->second.sca != NULL) {
				main = b->second.sca;
			}
			else
				continue;
			TransformAnimationData& tad = animData->addAnimatinData(b->first);
			aiNodeAnim* nanim = b->second.pos == NULL ? main : b->second.pos;
			tad.positionCurve.duration = dur;
			for (int k = 0; k < nanim->mNumPositionKeys; k++) {
				aiVector3D& pos = nanim->mPositionKeys[k].mValue;
				if (swapYZ)
					tad.positionCurve.insert(nanim->mPositionKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(pos.x, -pos.z, pos.y)));
				else
					tad.positionCurve.insert(nanim->mPositionKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(pos.x, pos.y, pos.z)));
			}
			nanim = b->second.rot == NULL ? main : b->second.rot;
			tad.rotationCurve.duration = dur;
			for (int k = 0; k < nanim->mNumRotationKeys; k++) {
				aiQuaternion& rot = nanim->mRotationKeys[k].mValue;
				if (swapYZ)
					tad.rotationCurve.insert(nanim->mRotationKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Quaternionf>(CurveValue<Quaternionf>::Linear, Quaternionf::FromTwoVectors(Vector3f(0, -1, 0), Vector3f(1, 0, 0)) * Quaternionf(rot.w, rot.x, -rot.z, rot.y)));
				else
					tad.rotationCurve.insert(nanim->mRotationKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Quaternionf>(CurveValue<Quaternionf>::Linear, Quaternionf(rot.w, rot.x, rot.y, rot.z)));
			}
			nanim = b->second.sca == NULL ? main : b->second.sca;
			tad.scaleCurve.duration = dur;
			for (int k = 0; k < nanim->mNumScalingKeys; k++) {
				aiVector3D& sca = nanim->mScalingKeys[k].mValue;
				if (swapYZ)
					tad.scaleCurve.insert(nanim->mScalingKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(sca.x, sca.z, sca.y)));
				else
					tad.scaleCurve.insert(nanim->mScalingKeys[k].mTime / amin->mTicksPerSecond,
						CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, Vector3f(sca.x, sca.y, sca.z)));
			}
		}
		animData->pack();
		clips.push_back(animData);
	}
	return !clips.empty();
}

string AssimpImporter::getError()
{
	return errorString;
}

Mesh * AssimpImporter::loadMesh(const string & file, unsigned int flag)
{
	AssimpImporter imp = AssimpImporter(file, flag);
	if (imp.isSkeletonMesh()) {
		SkeletonMesh* m = new SkeletonMesh();
		if (imp.getSkeletonMesh(*m))
			return m;
		else {
			delete m;
			return NULL;
		}
	}
	else {
		Mesh* m = new Mesh();
		if (imp.getMesh(*m))
			return m;
		else {
			delete m;
			return NULL;
		}
	}
}

void AssimpImporter::toMatrix4f(Matrix4f & a, aiMatrix4x4 & b)
{
	a(0, 0) = b.a1;
	a(0, 1) = b.a2;
	a(0, 2) = b.a3;
	a(0, 3) = b.a4;
	a(1, 0) = b.b1;
	a(1, 1) = b.b2;
	a(1, 2) = b.b3;
	a(1, 3) = b.b4;
	a(2, 0) = b.c1;
	a(2, 1) = b.c2;
	a(2, 2) = b.c3;
	a(2, 3) = b.c4;
	a(3, 0) = b.d1;
	a(3, 1) = b.d2;
	a(3, 2) = b.d3;
	a(3, 3) = b.d4;
}

bool AssimpImporter::processNodes(aiNode* node, const Matrix4f& gTransform)
{
	if (node == NULL)
		return false;
	try {
		boneName.insert(pair<string, unsigned int>(node->mName.C_Str(), boneList.size()));
	}
	catch(exception e) {
		return false;
	}
	Matrix4f t = Matrix4f::Identity();
	toMatrix4f(t, node->mTransformation);
	t = gTransform * t;
	boneList.push_back(pair<aiNode*, Matrix4f>(node, t.inverse()));
	for (int i = 0; i < node->mNumChildren; i++) {
		if (!processNodes(node->mChildren[i], t))
			return false;
	}
	return true;
}

bool AssimpImporter::preprocess()
{
	if (scene == NULL)
		return false;
	if (!processNodes(scene->mRootNode))
		return false;
	nMorph = 0;
	nMorphVert = 0;
	vector<aiMesh*> normMeshes;
	for (int i = 0; i < scene->mNumMeshes; i++) {
		if (scene->mMeshes[i]->mFaces->mNumIndices != 3)
			continue;
		if (scene->mMeshes[i]->mNumAnimMeshes > 0) {
			if (nMorph == 0) {
				nMorph = scene->mMeshes[i]->mNumAnimMeshes;
				morphName.resize(nMorph);
				for (int n = 0; n < nMorph; n++)
					morphName[n] = scene->mMeshes[i]->mAnimMeshes[n]->mName.C_Str();
			}
			else if (nMorph != scene->mMeshes[i]->mNumAnimMeshes)
				return false;
			meshSortedList.push_back(scene->mMeshes[i]);
			nMorphVert += scene->mMeshes[i]->mNumVertices;
		}
		else
			normMeshes.push_back(scene->mMeshes[i]);
	}
	for (int i = 0; i < normMeshes.size(); i++)
		meshSortedList.push_back(normMeshes[i]);
	nMesh = meshSortedList.size();
	vertexWeights.resize(nMesh);
	for (int i = 0; i < meshSortedList.size(); i++) {
		nFace += meshSortedList[i]->mNumFaces;
		nVert += meshSortedList[i]->mNumVertices;
		vertexWeights[i].resize(meshSortedList[i]->mNumVertices);
		if (meshSortedList[i]->mNumBones > 0)
			hasBone = true;
		for (int b = 0; b < meshSortedList[i]->mNumBones; b++) {
			aiBone* bone = meshSortedList[i]->mBones[b];
			auto bt = boneName.find(bone->mName.C_Str());
			if (bt == boneName.end())
				return false;
			unsigned int bi = bt->second;
			toMatrix4f(boneList[bi].second, bone->mOffsetMatrix);
			for (int w = 0; w < bone->mNumWeights; w++) {
				if (vertexWeights[i][bone->mWeights[w].mVertexId].size() > VERTEX_MAX_BONE)
					return false;
				vertexWeights[i][bone->mWeights[w].mVertexId].push_back(pair<unsigned int, float>(bi, bone->mWeights[w].mWeight));
			}
		}
	}
	pre = true;
	return true;
}

bool AssimpImporter::toMeshParts()
{
	bound = BoundBox::none;
	meshParts.resize(nMesh);
	totalMesh = VendorManager::getInstance().getVendor().newMeshPart(nVert, nFace * 3);
	unsigned int baseVertex = totalMesh.vertexFirst, baseElement = totalMesh.elementFirst;
	for (int i = 0; i < meshSortedList.size(); i++) {
		aiMesh* mesh = meshSortedList[i];
		MeshPart part = MeshPart { totalMesh.meshData, baseVertex, mesh->mNumVertices, baseElement, mesh->mNumFaces * 3 };
		part.bound = BoundBox::none;
		for (int v = 0; v < mesh->mNumVertices; v++) {
			part.vertex(v) = Vector3f(
				mesh->mVertices[v].x,
				mesh->mVertices[v].y,
				mesh->mVertices[v].z
			);
			part.normal(v) = Vector3f(
				mesh->mNormals[v].x,
				mesh->mNormals[v].y,
				mesh->mNormals[v].z
			).normalize();
			if (mesh->mTextureCoords[0] == NULL)
				part.uv(v) = Vector2f(0, 0);
			else
				part.uv(v) = Vector2f(
					mesh->mTextureCoords[0][v].x,
					mesh->mTextureCoords[0][v].y
				);

			part.bound.encapsulate(part.vertex(v));
		}
		for (int f = 0; f < mesh->mNumFaces; f++) {
			part.element(f * 3) = mesh->mFaces[f].mIndices[0];
			part.element(f * 3 + 1) = mesh->mFaces[f].mIndices[1];
			part.element(f * 3 + 2) = mesh->mFaces[f].mIndices[2];
		}
		bound.encapsulate(part.bound);
		meshParts[i].first = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
		meshParts[i].second = part;
		baseVertex += part.vertexCount;
		baseElement += part.elementCount;
	}
	return !meshParts.empty();
}

bool AssimpImporter::toSkeletonMeshParts()
{
	bound = BoundBox::none;
	meshParts.resize(nMesh);
	SkeletonMeshPart _totalMesh = VendorManager::getInstance().getVendor().newSkeletonMeshPart(nVert, nFace * 3, boneList.size(), nMorphVert, nMorph);
	totalMesh = _totalMesh;
	unsigned int baseVertex = _totalMesh.vertexFirst, baseElement = _totalMesh.elementFirst;
	for (int i = 0; i < meshSortedList.size(); i++) {
		aiMesh* mesh = meshSortedList[i];
		SkeletonMeshPart part = { (SkeletonMeshData*)_totalMesh.meshData, baseVertex, mesh->mNumVertices, baseElement, mesh->mNumFaces * 3 };
		part.bound = BoundBox::none;
		if (mesh->mNumAnimMeshes > 0)
			part.morphMeshData = _totalMesh.morphMeshData;
		for (int v = 0; v < mesh->mNumVertices; v++) {
			part.vertex(v) = Vector3f(
				mesh->mVertices[v].x,
				mesh->mVertices[v].y,
				mesh->mVertices[v].z
			);
			part.normal(v) = Vector3f(
				mesh->mNormals[v].x,
				mesh->mNormals[v].y,
				mesh->mNormals[v].z
			).normalize();
			if (mesh->mTextureCoords[0] != NULL) {
				part.uv(v) = Vector2f(
					mesh->mTextureCoords[0][v].x,
					mesh->mTextureCoords[0][v].y
				);
			}
			for (int w = 0; w < vertexWeights[i][v].size(); w++) {
				part.boneIndex(v)[w] = vertexWeights[i][v][w].first;
				part.weight(v)[w] = vertexWeights[i][v][w].second;
			}
			for (int m = 0; m < mesh->mNumAnimMeshes; m++) {
				part.morphVertex(v, m) = Vector4f(
					mesh->mAnimMeshes[m]->mVertices[v].x - mesh->mVertices[v].x,
					mesh->mAnimMeshes[m]->mVertices[v].y - mesh->mVertices[v].y,
					mesh->mAnimMeshes[m]->mVertices[v].z - mesh->mVertices[v].z, 1.0f
				);
				part.morphNormal(v, m) = Vector4f(
					mesh->mAnimMeshes[m]->mNormals[v].x - mesh->mNormals[v].x,
					mesh->mAnimMeshes[m]->mNormals[v].y - mesh->mNormals[v].y,
					mesh->mAnimMeshes[m]->mNormals[v].z - mesh->mNormals[v].z, 1.0f
				);
			}

			part.bound.encapsulate(part.vertex(v));
		}
		for (int f = 0; f < mesh->mNumFaces; f++) {
			part.element(f * 3) = mesh->mFaces[f].mIndices[0];
			part.element(f * 3 + 1) = mesh->mFaces[f].mIndices[1];
			part.element(f * 3 + 2) = mesh->mFaces[f].mIndices[2];
		}
		bound.encapsulate(part.bound);
		meshParts[i].first = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
		meshParts[i].second = part;
		baseVertex += part.vertexCount;
		baseElement += part.elementCount;
	}
	return !meshParts.empty();
}

bool AssimpImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	const unsigned int flag = aiProcessPreset_TargetRealtime_MaxQuality;
	bool animationOnly = false;
	string bstr;
	info.config.get("animationOnly", bstr);
	animationOnly = bstr == "true";
	if (load(info.path, flag)) {
		if (!animationOnly) {
			if (isSkeletonMesh()) {
				SkeletonMesh* mesh = new SkeletonMesh();
				if (getSkeletonMesh(*mesh)) {
					Asset* asset = new Asset(&SkeletonMeshAssetInfo::assetInfo, info.filename, info.path);
					asset->asset[0] = mesh;
					if (AssetManager::registAsset(*asset)) {
						result.assets.push_back(asset);
					}
					else {
						delete asset;
						delete mesh;
						result.status = ImportResult::RegisterFailed;
					}
				}
				else {
					delete mesh;
					result.status = ImportResult::LoadFailed;
				}
			}
			else {
				Mesh* mesh = new Mesh();
				if (getMesh(*mesh)) {
					Asset* asset = new Asset(&MeshAssetInfo::assetInfo, info.filename, info.path);
					asset->asset[0] = mesh;
					if (AssetManager::registAsset(*asset)) {
						result.assets.push_back(asset);
					}
					else {
						delete asset;
						delete mesh;
						result.status = ImportResult::RegisterFailed;
					}
				}
				else {
					delete mesh;
					result.status = ImportResult::LoadFailed;
				}
			}
		}
		if (hasAnimation()) {
			vector<AnimationClipData*> anims;
			if (getAnimation(anims, true)) {
				for (int i = 0; i < anims.size(); i++) {
					Asset* asset = new Asset(&AnimationClipDataAssetInfo::assetInfo, info.filename + ":" + anims[i]->name, info.path + ":Animation" + to_string(i));
					asset->asset[0] = anims[i];
					if (AssetManager::registAsset(*asset)) {
						result.assets.push_back(asset);
						result.status = ImportResult::Successed;
					}
					else {
						delete asset;
						if (result.status == ImportResult::Successed)
							result.status = ImportResult::PartSuccessed;
						else
							result.status = ImportResult::RegisterFailed;
					}
				}
			}
			else {
				if (result.status == ImportResult::Successed)
					result.status = ImportResult::PartSuccessed;
				else
					result.status = ImportResult::LoadFailed;
			}
		}
	}
	else {
		result.status = ImportResult::LoadFailed;
	}
	return result.status == ImportResult::Successed || result.status == ImportResult::PartSuccessed;
}
