#include "Spine2DMeshData.h"

Spine2DMeshData::Spine2DMeshData()
{
}

Spine2DMeshData::~Spine2DMeshData()
{
    release();
}

void Spine2DMeshData::release()
{
    vertexBuffer.resize(0);
    uvBuffer.resize(0);
    elementBuffer.resize(0);
    meshParts.clear();
    model = NULL;
    inited = false;
}

void Spine2DMeshData::setModel(spine::Skeleton* model)
{
    if (this->model == model)
        return;
    release();
    if (model == NULL)
        return;
    this->model = model;
    int count = model->getSlots().size();
    meshParts.resize(count);
    int vertexCount = 0;
    int indexCount = 0;
    for (int i = 0; i < count; i++) {
        spine::Slot* slot = model->getSlots()[i];
        spine::Attachment* attachment = slot->getAttachment();
        if (!attachment) continue;

        int vcount, icount;

        if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
            vcount = 4;
            icount = 6;
        }
        else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
            vcount = mesh->getWorldVerticesLength() / 2;
            icount = mesh->getTriangles().size();
        }
        else {
            vcount = 0;
            icount = 0;
        }

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

    updateBounds();

    inited = true;
}

void Spine2DMeshData::updateElement(int index)
{
    spine::Slot* slot = model->getSlots()[index];

    spine::Attachment* attachment = slot->getAttachment();
    if (!attachment) return;

    MeshPart& part = meshParts[index];

    if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
        const unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };
        memcpy(&part.element(0), quadIndices, sizeof(quadIndices));
    }
    else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
        spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
        for (int i = 0; i < part.elementCount; i += 3) {
            part.element(i) = mesh->getTriangles()[i];
            part.element(i + 1) = mesh->getTriangles()[i + 1];
            part.element(i + 2) = mesh->getTriangles()[i + 2];
        }
    }
    if (part.elementCount > 0)
        elementBuffer.uploadSubData(part.elementFirst, part.elementCount, &part.element(0));
}

void Spine2DMeshData::updateVertex(int index)
{
    spine::Slot* slot = model->getSlots()[index];

    spine::Attachment* attachment = slot->getAttachment();
    if (!attachment) return;

    MeshPart& part = meshParts[index];

    if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
        spine::RegionAttachment* regionAttachment = (spine::RegionAttachment*)attachment;

        regionAttachment->computeWorldVertices(slot->getBone(), part.vertex(0).data(), 0, 3);

        for (size_t j = 0, l = 0; j < 4; j++, l += 2) {
            Vector2f& uv = part.uv(j);
            uv.x() = regionAttachment->getUVs()[l];
            uv.y() = regionAttachment->getUVs()[l + 1];
        }
    }
    else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
        spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;

        size_t numFloats = mesh->getWorldVerticesLength();
        size_t numVertices = numFloats / 2;
        mesh->computeWorldVertices(*slot, 0, numFloats, part.vertex(0).data(), 0, 3);
        memcpy(part.uv(0).data(), mesh->getUVs().buffer(), numVertices * sizeof(Vector2f));
    }
    if (part.vertexCount > 0) {
        vertexBuffer.uploadSubData(part.vertexFirst, part.vertexCount, part.vertex(0).data());
        uvBuffer.uploadSubData(part.vertexFirst, part.vertexCount, part.uv(0).data());
    }
}

void Spine2DMeshData::updateBounds()
{
    bounds.minVal = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    bounds.maxVal = Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for each (auto pos in vertices) {
        bounds.minVal[0] = min(bounds.minVal[0], pos.x());
        bounds.minVal[1] = min(bounds.minVal[1], pos.y());
        bounds.minVal[2] = min(bounds.minVal[2], pos.z());

        bounds.maxVal[0] = max(bounds.maxVal[0], pos.x());
        bounds.maxVal[1] = max(bounds.maxVal[1], pos.y());
        bounds.maxVal[2] = max(bounds.maxVal[2], pos.z());
    }
}

const MeshPart* Spine2DMeshData::getMeshPart(int index) const
{
    return &meshParts[index];
}

bool Spine2DMeshData::isValid() const
{
    return inited;
}

bool Spine2DMeshData::isGenerated() const
{
    return inited;
}

void Spine2DMeshData::init()
{
}

void Spine2DMeshData::bindShape()
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

void Spine2DMeshData::bindShapeWithContext(IRenderContext& context)
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
