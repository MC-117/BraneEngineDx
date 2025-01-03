#include "ClothMeshData.h"

ClothMeshData::ClothMeshData()
{
}

ClothMeshData::~ClothMeshData()
{
    release();
}

void ClothMeshData::release()
{
    vertexBuffer.resize(0);
    uvBuffer.resize(0);
    elementBuffer.resize(0);
    sourceMeshParts.clear();
    clothParts.clear();
    inited = false;
}

void ClothMeshData::addMeshPart(const string& name, const MeshPart& meshPart)
{
    updated = true;
    sourceMeshParts.push_back(meshPart);
    mesh.addMeshPart(name, MeshPart());
    partNames.push_back(name);
}

void ClothMeshData::apply(const Matrix4f& transformMat)
{
    if (inited)
        release();
    int count = sourceMeshParts.size();
    clothParts.resize(count);
    int vertexCount = 0;
    int indexCount = 0;
    for (int i = 0; i < count; i++) {
        int vcount = sourceMeshParts[i].vertexCount;
        int icount = sourceMeshParts[i].elementCount;

        MeshPart& part = clothParts[i];
        part.vertexPerFace = sourceMeshParts[i].vertexPerFace;
        part.vertexFirst = vertexCount;
        part.elementFirst = indexCount;
        part.vertexCount = vcount;
        part.elementCount = icount;
        part.meshData = this;
        part.mesh = NULL;
        part.partIndex = i;

        vertexCount += vcount;
        indexCount += icount;
    }
    invMass.resize(vertexCount, 1);
    vertices.resize(vertexCount);
    normals.resize(vertexCount);
    uvs.resize(vertexCount);
    elements.resize(indexCount);

    vertexBuffer.resize(vertexCount);
    normalBuffer.resize(vertexCount);
    uvBuffer.resize(vertexCount);
    elementBuffer.resize(indexCount);

    totalPart.elementFirst = 0;
    totalPart.elementCount = indexCount;
    totalPart.vertexFirst = 0;
    totalPart.vertexCount = vertexCount;
    totalPart.meshData = this;
    totalPart.mesh = NULL;

    for (int i = 0; i < count; i++) {
        MeshPart& spart = sourceMeshParts[i];
        MeshPart& cpart = clothParts[i];
        memcpy(cpart.vertex(0).data(), spart.vertex(0).data(), sizeof(Vector3f) * spart.vertexCount);
        memcpy(cpart.normal(0).data(), spart.normal(0).data(), sizeof(Vector3f) * spart.vertexCount);
        memcpy(cpart.uv(0).data(), spart.uv(0).data(), sizeof(Vector2f) * spart.vertexCount);
        memcpy(&cpart.element(0), &spart.element(0), sizeof(float) * spart.elementCount);
    }

    if (transformMat != Matrix4f::Identity())
        for (int i = 0; i < vertexCount; i++) {
            Vector3f& v = vertices[i];
            v = (transformMat * Vector4f(v.x(), v.y(), v.z(), 1)).block(0, 0, 3, 1);
            Vector3f& n = normals[i];
            n = (transformMat * Vector4f(n.x(), n.y(), n.z(), 0)).block(0, 0, 3, 1);
            n.normalize();
        }

    updateElement();
    updateVertex();
    updateNormal();
    updateUV();

    mesh.meshPartNameMap.clear();
    mesh.partNames.clear();
    mesh.meshParts.clear();
    mesh.faceCount = indexCount;
    mesh.vertCount = vertexCount;
    mesh.vertexPerFace = 3;
    mesh.renderMode = ShapeTopologyType::STT_Triangle;
    mesh.bound = totalPart.bound;
    mesh.setTotalMeshPart(totalPart);
    for (int i = 0; i < clothParts.size(); i++) {
        MeshPart part = clothParts[i];
        mesh.addMeshPart(partNames[i], part);
    }

    inited = true;
}

float& ClothMeshData::getMass(const MeshPart& part, int index)
{
    return invMass[part.vertexFirst + index];
}

void ClothMeshData::updateElement()
{
    elementBuffer.uploadData(elements.size(), elements.data());
}

void ClothMeshData::updateVertex()
{
    totalPart.bound = BoundBox::none;
    for (int i = 0; i < clothParts.size(); i++) {
        MeshPart& part = clothParts[i];
        part.bound = BoundBox::none;
        for (int v = 0; v < part.vertexCount; v++) {
            Vector3f& p = part.vertex(v);
            part.bound.encapsulate(p);
        }

        mesh.meshParts[i].bound = part.bound;
        totalPart.bound.encapsulate(part.bound);
    }
    mesh.bound = totalPart.bound;
    mesh.totalMeshPart.bound = totalPart.bound;
    vertexBuffer.uploadData(vertices.size(), vertices.data());
}

void ClothMeshData::updateNormal()
{
    normalBuffer.uploadData(normals.size(), normals.data());
}

void ClothMeshData::updateUV()
{
    uvBuffer.uploadData(uvs.size(), uvs.data());
}

const MeshPart* ClothMeshData::getMeshPart(int index) const
{
    return &clothParts[index];
}

bool ClothMeshData::isValid() const
{
    return inited;
}

bool ClothMeshData::isGenerated() const
{
    return inited;
}

void ClothMeshData::init()
{
}

void ClothMeshData::bindShape()
{
    if (!inited)
        return;
    if (currentMeshData == this)
        return;
    VendorManager::getInstance().getVendor().setMeshDrawContext();
    vertexBuffer.bindBase(1);
    uvBuffer.bindBase(2);
    normalBuffer.bindBase(3);
    elementBuffer.bindBase(0);
    currentMeshData = this;
}

void ClothMeshData::bindShapeWithContext(IRenderContext& context)
{
    context.setMeshDrawContext();
    context.bindBufferBase(vertexBuffer.getVendorGPUBuffer(), 1);
    context.bindBufferBase(uvBuffer.getVendorGPUBuffer(), 2);
    context.bindBufferBase(normalBuffer.getVendorGPUBuffer(), 3);
    context.bindBufferBase(elementBuffer.getVendorGPUBuffer(), 0);
    context.currentMeshData = this;
}
