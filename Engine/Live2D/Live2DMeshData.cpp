#include "Live2DMeshData.h"
#include "../Core/IRenderContext.h"

Live2DMeshData::Live2DMeshData()
{
}

Live2DMeshData::~Live2DMeshData()
{
    release();
}

void Live2DMeshData::release()
{
    vertexBuffer.resize(0);
    uvBuffer.resize(0);
    elementBuffer.resize(0);
    meshParts.clear();
    inited = false;
}

void Live2DMeshData::setModel(Csm::CubismModel* model)
{
    if (this->model == model)
        return;
    release();
    if (model == NULL)
        return;
    this->model = model;
    int count = model->GetDrawableCount();
    meshParts.resize(count);
    int vertexCount = 0;
    int indexCount = 0;
    for (int i = 0; i < count; i++) {
        int vcount = model->GetDrawableVertexCount(i);
        int icount = model->GetDrawableVertexIndexCount(i);

        MeshPart& part = meshParts[i];
        part.vertexPerFace = 3;
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
    vertices.resize(vertexCount);
    uvs.resize(vertexCount);
    elements.resize(indexCount);
    vertexBuffer.resize(vertexCount);
    uvBuffer.resize(vertexCount);
    elementBuffer.resize(indexCount);
    for (int i = 0; i < count; i++) {
        updateElement(i);
        updateVertex(i);
    }

    inited = true;
}

void Live2DMeshData::updateElement(int index)
{
    int count = model->GetDrawableVertexIndexCount(index);
    if (count == 0)
        return;
    const unsigned short* indices = model->GetDrawableVertexIndices(index);
    MeshPart& part = meshParts[index];
    for (int i = 0; i < count; i += 3) {
        part.element(i) = indices[i];
        part.element(i + 1) = indices[i + 1];
        part.element(i + 2) = indices[i + 2];
    }
    elementBuffer.uploadSubData(part.elementFirst, part.elementCount, &part.element(0));
}

void Live2DMeshData::updateVertex(int index)
{
    int count = model->GetDrawableVertexCount(index);
    if (count == 0)
        return;
    const float* vertices = model->GetDrawableVertices(index);
    const float* uvs = (const float*)model->GetDrawableVertexUvs(index);
    MeshPart& part = meshParts[index];
    for (int i = 0; i < count * 2; i += 2) {
        part.vertex(i / 2) = Vector3f(vertices[i], vertices[i + 1]);
        part.uv(i / 2) = Vector2f(uvs[i], uvs[i + 1]);
    }
    vertexBuffer.uploadSubData(part.vertexFirst, part.vertexCount, part.vertex(0).data());
    uvBuffer.uploadSubData(part.vertexFirst, part.vertexCount, part.uv(0).data());
}

const MeshPart* Live2DMeshData::getMeshPart(int index) const
{
    return &meshParts[index];
}

bool Live2DMeshData::isValid() const
{
    return inited;
}

bool Live2DMeshData::isGenerated() const
{
    return inited;
}

void Live2DMeshData::init()
{
}

void Live2DMeshData::bindShape()
{
    if (!inited)
        return;
    if (currentMeshData == this)
        return;
    VendorManager::getInstance().getVendor().setMeshDrawContext();
    vertexBuffer.bindBase(1);
    uvBuffer.bindBase(2);
    elementBuffer.bindBase(0);
    currentMeshData = this;
}

void Live2DMeshData::bindShapeWithContext(IRenderContext& context)
{
    if (!inited)
        return;
    if (context.currentMeshData == this)
        return;
    context.setMeshDrawContext();
    context.bindBufferBase(vertexBuffer.getVendorGPUBuffer(), 1);
    context.bindBufferBase(uvBuffer.getVendorGPUBuffer(), 2);
    context.bindBufferBase(elementBuffer.getVendorGPUBuffer(), 0);
    context.currentMeshData = this;
}
