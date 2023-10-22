#include "IGCommandBuffer.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

void IGCommandBuffer::draw(u32 instanceCount_, u32 vertexCount_) {}

void IGCommandBuffer::drawIndexed(u32 instanceCount_, u32 indexCount_) {}

void IGCommandBuffer::bindMaterial(mref<const MaterialIdentifier> identifier_, ptr<const material::Material> material_) {}

void IGCommandBuffer::bindStaticMesh(ptr<const Mesh> mesh_) {}

void IGCommandBuffer::bindStaticInstances(ptr<const MeshInstanceView> view_) {}

void IGCommandBuffer::bindSkeletalMesh(ptr<const Mesh> mesh_) {}

void IGCommandBuffer::bindSkeletalInstances(ptr<const MeshInstanceView> view_, ptr<const SkeletalBoneView> boneView_) {}

void IGCommandBuffer::bindVertexBuffer(ptr<const VertexBufferView> vertexView_) {}

void IGCommandBuffer::bindIndexBuffer(ptr<const IndexBufferView> indexView_) {}

void IGCommandBuffer::bindTexture(ptr<const VirtualTextureView> textureView_) {}

void IGCommandBuffer::bindStorage(ptr<const StorageBufferView> storageView_) {}
