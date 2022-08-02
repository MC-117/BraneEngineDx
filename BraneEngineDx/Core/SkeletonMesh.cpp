#include "SkeletonMesh.h"
#include "Shader.h"
#include "Utility/Utility.h"

SkeletonMeshPart::SkeletonMeshPart() : MeshPart()
{
}

SkeletonMeshPart::SkeletonMeshPart(const SkeletonMeshPartDesc& desc)
{
	meshData = desc.meshData;
	vertexFirst = desc.vertexFirst;
	vertexCount = desc.vertexCount;
	elementFirst = desc.elementFirst;
	elementCount = desc.elementCount;
	morphMeshData = desc.morphMeshData;
}

SkeletonMeshPart::SkeletonMeshPart(SkeletonMeshData * meshData, unsigned int vertexFirst, unsigned int vertexCount, unsigned int indexFirst, unsigned int indexCount, MorphMeshData* morphData) :
	MeshPart(meshData, vertexFirst, vertexCount, indexFirst, indexCount), morphMeshData(morphData)
{
}

SkeletonMeshPart::operator MeshPart()
{
	MeshPart p = { meshData, vertexFirst, vertexCount, elementFirst, elementCount };
	p.mesh = mesh;
	p.partIndex = partIndex;
	p.bound = bound;
	return p;
}

unsigned int BoneData::addChild(const string & name, const Matrix4f & offsetMatrix, const Matrix4f & transformMatrix)
{
	if (skeletonData == NULL)
		return -1;
	skeletonData->boneName.insert(pair<string, unsigned int>(name, skeletonData->boneList.size()));
	skeletonData->boneList.push_back({
		name, (unsigned int)skeletonData->boneList.size(),
		(unsigned int)children.size(), offsetMatrix,
		transformMatrix, skeletonData, this
	});
	children.push_back(&skeletonData->boneList.back());
	return skeletonData->boneList.size() - 1;
}

BoneData * BoneData::getParent()
{
	return parent;
}

void BoneData::setParent(BoneData * parent)
{
	this->parent = parent;
	if (parent == NULL)
		return;
	siblingIndex = parent->children.size();
	parent->children.push_back(this);
}

BoneData * BoneData::getChild(unsigned int index)
{
	return index < children.size() ? children[index] : NULL;
}

BoneData * BoneData::getSibling(unsigned int index)
{
	unsigned int i = siblingIndex + index + 1;
	return parent == NULL ? NULL : (i < parent->children.size() ? parent->children[i] : NULL);
}

BoneData * BoneData::getNext(BoneData* limitBone)
{
	BoneData* b = getChild();
	if (b == NULL) {
		b = getSibling();
		if (b == NULL) {
			b = parent;
			while (b != limitBone) {
				BoneData* s = b->getSibling();
				if (s == NULL) {
					b = b->parent;
				}
				else {
					b = s;
					return b;
				}
			}
			return NULL;
		}
	}
	return b;
}

BoneData * SkeletonData::getBoneData(const string & name)
{
	auto iter = boneName.find(name);
	if (iter == boneName.end())
		return NULL;
	return &boneList[iter->second];
}

BoneData * SkeletonData::getBoneData(unsigned int index)
{
	return index < boneList.size() ? &boneList[index] : NULL;
}

//void SkeletonMesh::updateMorphWeights(vector<float>& weights)
//{
//	((SkeletonMeshData*)totalMeshPart.meshData)->updateMorphWeights(weights);
//}

bool SkeletonMesh::writeObjStream(ostream& os, const vector<int>& partIndex) const
{
	vector<int> offsets;
	offsets.resize(partIndex.size());
	int count = 0;
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		 SkeletonMeshPart part = (const SkeletonMeshPart&)meshParts[i];
		 part.morphMeshData = NULL;
		 if (partHasMorph[i])
			 part.morphMeshData = &((SkeletonMeshData*)part.meshData)->morphMeshData;
		for (int v = 0; v < part.vertexCount; v++) {
			string line = "v ";
			Vector3f vec = part.vertex(v);
			line += to_string(vec.x()) + " " + to_string(vec.y()) + " " + to_string(vec.z()) + "\n";
			os.write(line.c_str(), line.length());
		}
		offsets[index] = count;
		count += part.vertexCount;

		if (part.morphMeshData != NULL) {
			int mcount = morphName.size();
			for (int m = 0; m < mcount; m++) {
				for (int v = 0; v < part.vertexCount; v++) {
					string line = "v ";
					Vector3f vec = part.morphVertex(v, m);
					//vec += part.vertex(v);
					line += to_string(vec.x()) + " " + to_string(vec.y()) + " " + to_string(vec.z()) + "\n";
					os.write(line.c_str(), line.length());
				}
			}
			count += part.vertexCount * mcount;
		}
	}
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		const SkeletonMeshPart& part = (const SkeletonMeshPart&)meshParts[i];
		for (int v = 0; v < part.vertexCount; v++) {
			string line = "vt ";
			Vector2f vec = part.uv(v);
			line += to_string(vec.x()) + " " + to_string(vec.y()) + "\n";
			os.write(line.c_str(), line.length());
		}
	}
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		SkeletonMeshPart part = (const SkeletonMeshPart&)meshParts[i];
		part.morphMeshData = NULL;
		if (partHasMorph[i])
			part.morphMeshData = &((SkeletonMeshData*)part.meshData)->morphMeshData;
		for (int v = 0; v < part.vertexCount; v++) {
			string line = "n ";
			Vector3f vec = part.normal(v);
			line += to_string(vec.x()) + " " + to_string(vec.y()) + " " + to_string(vec.z()) + "\n";
			os.write(line.c_str(), line.length());
		}

		if (part.morphMeshData != NULL) {
			int mcount = morphName.size();
			for (int m = 0; m < mcount; m++) {
				for (int n = 0; n < part.vertexCount; n++) {
					string line = "n ";
					Vector3f vec = part.morphNormal(n, m);
					//vec += part.normal(n);
					line += to_string(vec.x()) + " " + to_string(vec.y()) + " " + to_string(vec.z()) + "\n";
					os.write(line.c_str(), line.length());
				}
			}
		}
	}
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		SkeletonMeshPart part = (const SkeletonMeshPart&)meshParts[i];
		part.morphMeshData = NULL;
		if (partHasMorph[i])
			part.morphMeshData = &((SkeletonMeshData*)part.meshData)->morphMeshData;
		string line = "g part" + to_string(i) + "\n";
		os.write(line.c_str(), line.length());
		for (int e = 0; e < part.elementCount; e += 3) {
			string line = "f ";
			unsigned int vec0 = part.element(e);
			unsigned int vec1 = part.element(e + 1);
			unsigned int vec2 = part.element(e + 2);
			int offset = offsets[index] + 1;
			vec0 += offset;
			vec1 += offset;
			vec2 += offset;
			line += to_string(vec0) + "/" + to_string(vec0) + "/" + to_string(vec0) + " " +
				to_string(vec1) + "/" + to_string(vec1) + "/" + to_string(vec1) + " " +
				to_string(vec2) + "/" + to_string(vec2) + "/" + to_string(vec2) + "\n";
			os.write(line.c_str(), line.length());
		}
		if (part.morphMeshData != NULL) {
			for (int m = 0; m < morphName.size(); m++) {
				line = "g part" + to_string(i) + "_morph" + to_string(m) + "\n";
				os.write(line.c_str(), line.length());

				int moffset = (m + 1) * part.vertexCount;

				for (int e = 0; e < part.elementCount; e++) {
					string line = "f ";
					unsigned int vec0 = part.element(e);
					unsigned int vec1 = part.element(e + 1);
					unsigned int vec2 = part.element(e + 2);
					int offset = offsets[index] + 1;
					vec0 += offset + moffset;
					vec1 += offset + moffset;
					vec2 += offset + moffset;
					line += to_string(vec0) + "/" + to_string(vec0) + "/" + to_string(vec0) + " " +
						to_string(vec1) + "/" + to_string(vec1) + "/" + to_string(vec1) + " " +
						to_string(vec2) + "/" + to_string(vec2) + "/" + to_string(vec2) + "\n";
					os.write(line.c_str(), line.length());
				}
			}
		}
	}
	return true;
}
