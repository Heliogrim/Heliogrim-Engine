#include "PooledRenderCommandBuffer.hpp"

#include <Engine.GFX.RenderPipeline.Factory/StaticMeshLayout.hpp>

#include "Commands/Begin.hpp"
#include "Commands/BindIndexBuffer.hpp"
#include "Commands/BindMaterial.hpp"
#include "Commands/BindSkeletalMesh.hpp"
#include "Commands/BindStaticMesh.hpp"
#include "Commands/BindStorageBuffer.hpp"
#include "Commands/BindTexture.hpp"
#include "Commands/BindVertexBuffer.hpp"
#include "Commands/DrawMesh.hpp"
#include "Commands/DrawSkeletalMesh.hpp"
#include "Commands/End.hpp"
#include "Commands/NextSubpass.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void PooledRenderCommandBuffer::begin() noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BeginRenderCommand>();
}

void PooledRenderCommandBuffer::nextSubpass() noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<NextSubpassRenderCommand>();
}

void PooledRenderCommandBuffer::end() noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<EndRenderCommand>();
}

void PooledRenderCommandBuffer::bindMaterial(
    mref<MaterialIdentifier> identifier_,
    const ptr<const material::Material> material_
) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindMaterialRenderCommand>(
        _STD move(identifier_),
        _STD move(material_)
    );
}

void PooledRenderCommandBuffer::bindStaticMesh(const ptr<const Mesh> mesh_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindStaticMeshRenderCommand>(_STD move(mesh_));
}

void PooledRenderCommandBuffer::bindStaticMeshInstance(const ptr<const MeshInstanceView> view_) noexcept {

    if constexpr (true) {

        const StorageBufferView* const sbv {};
        [[maybe_unused]] const auto result = _allocator.allocateCommand<BindStorageBufferRenderCommand>(_STD move(sbv));

    } else {

        const VertexBufferView* const vbv {};
        [[maybe_unused]] const auto result = _allocator.allocateCommand<BindVertexBufferRenderCommand>(_STD move(vbv));
    }

}

void PooledRenderCommandBuffer::bindSkeletalMesh(const ptr<const Mesh> mesh_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindSkeletalMeshRenderCommand>(_STD move(mesh_));
}

void PooledRenderCommandBuffer::bindSkeletalMeshInstance(
    const ptr<const MeshInstanceView> meshView_,
    const ptr<const SkeletalBoneView> boneView_
) noexcept {

    if constexpr (true) {

        const StorageBufferView* const isbv {};
        const StorageBufferView* const bsbv {};

        [[maybe_unused]] const auto instanceResult = _allocator.allocateCommand<BindStorageBufferRenderCommand>(
            _STD move(isbv)
        );
        [[maybe_unused]] const auto boneResult = _allocator.allocateCommand<BindStorageBufferRenderCommand>(
            _STD move(bsbv)
        );

    } else {

        const VertexBufferView* const ivbv {};
        const StorageBufferView* const bsbv {};

        [[maybe_unused]] const auto instanceResult = _allocator.allocateCommand<BindVertexBufferRenderCommand>(
            _STD move(ivbv)
        );
        [[maybe_unused]] const auto boneResult = _allocator.allocateCommand<BindStorageBufferRenderCommand>(
            _STD move(bsbv)
        );
    }

}

void PooledRenderCommandBuffer::bindIndexBuffer(const ptr<const IndexBufferView> indexView_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindIndexBufferRenderCommand>(
        _STD move(indexView_)
    );
}

void PooledRenderCommandBuffer::bindVertexBuffer(const ptr<const VertexBufferView> vertexView_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindVertexBufferRenderCommand>(
        _STD move(vertexView_)
    );
}

void PooledRenderCommandBuffer::bindStorage(const ptr<const StorageBufferView> storageView_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindStorageBufferRenderCommand>(
        _STD move(storageView_)
    );
}

void PooledRenderCommandBuffer::bindTexture(const ptr<const TextureView> textureView_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindTextureRenderCommand>(
        _STD move(textureView_)
    );
}

void PooledRenderCommandBuffer::bindTexture(const ptr<const VirtualTextureView> textureView_) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<BindTextureRenderCommand>(
        _STD move(textureView_)
    );
}

void PooledRenderCommandBuffer::drawMesh(
    const ptr<const MeshDescription> meshDescription_,
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 primitiveCount_,
    u32 primitiveOffset_
) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<DrawMeshRenderCommand>(
        meshDescription_,
        instanceCount_,
        instanceOffset_,
        primitiveCount_,
        primitiveOffset_,
        false
    );
}

void PooledRenderCommandBuffer::drawMeshIdx(
    const ptr<const MeshDescription> meshDescription_,
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 primitiveCount_,
    u32 primitiveOffset_
) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<DrawMeshRenderCommand>(
        meshDescription_,
        instanceCount_,
        instanceOffset_,
        primitiveCount_,
        primitiveOffset_,
        true
    );
}

void PooledRenderCommandBuffer::drawSkeletalMesh(
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 primitiveCount_,
    u32 primitiveOffset_
) noexcept {
    [[maybe_unused]] const auto result = _allocator.allocateCommand<DrawSkeletalMeshRenderCommand>(
        instanceCount_,
        instanceOffset_,
        primitiveCount_,
        primitiveOffset_,
        false
    );
}

void PooledRenderCommandBuffer::drawSkeletalMeshIdx(
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 primitiveCount_,
    u32 primitiveOffset_
) noexcept {
    [[nodiscard]] const auto result = _allocator.allocateCommand<DrawSkeletalMeshRenderCommand>(
        instanceCount_,
        instanceOffset_,
        primitiveCount_,
        primitiveOffset_,
        true
    );
}

void PooledRenderCommandBuffer::drawStaticMesh(
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 primitiveCount_,
    u32 primitiveOffset_
) noexcept {
    this_type::drawMesh(
        GetStaticMeshLayout().get(),
        instanceCount_,
        instanceOffset_,
        primitiveCount_,
        primitiveOffset_
    );
}

void PooledRenderCommandBuffer::drawStaticMeshIdx(
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 primitiveCount_,
    u32 primitiveOffset_
) noexcept {
    this_type::drawMeshIdx(
        GetStaticMeshLayout().get(),
        instanceCount_,
        instanceOffset_,
        primitiveCount_,
        primitiveOffset_
    );
}
