#include "MaterialTransformer.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/VirtualBufferView.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Command/CommandPool.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>
#include <Engine.GFX/Material/MaterialMetaDto.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>

#include "../Texture/TextureResource.hpp"
#include "../Texture/Traits.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

[[nodiscard]] static smr<engine::gfx::TextureResource> resolveTexture(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::AssetDatabase> database_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
);

static void makeStageBuffer(cref<sptr<Device>> device_, const u64 size_, _Out_ ref<Buffer> buffer_);

static void destroyStageBuffer(mref<Buffer> buffer_);

MaterialTransformer::MaterialTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_) :
    Transformer(),
    _pool(pool_) {}

MaterialTransformer::response_type::type MaterialTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    // Material does not require any additional binary data
    // auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});

    const auto& loader = engine::Engine::getEngine()->getResources()->loader();
    const auto* const database = engine::Engine::getEngine()->getAssets()->getDatabase();

    using derived_type = ::ember::engine::resource::UniqueResource<MaterialResource::value_type>;
    auto dst = make_smr<MaterialResource, derived_type>(
        new derived_type()
    );

    auto materialGuard = dst->acquire(resource::ResourceUsageFlag::eAll);
    auto& material = *materialGuard.mut();

    /**/

    if (request_->diffuse()) {
        material._diffuse = resolveTexture(request_->diffuse(), database, &loader);
    }

    if (request_->normal()) {
        material._normal = resolveTexture(request_->normal(), database, &loader);
    }

    if (request_->metalness()) {
        material._metalness = resolveTexture(request_->metalness(), database, &loader);
    }

    if (request_->roughness()) {
        material._roughness = resolveTexture(request_->roughness(), database, &loader);
    }

    if (request_->ao()) {
        material._ao = resolveTexture(request_->ao(), database, &loader);
    }

    if (request_->alpha()) {
        material._alpha = resolveTexture(request_->alpha(), database, &loader);
    }

    /**/

    material._view = _pool->allocateVirtualBuffer(pool::MaterialBufferAllocation {});

    /**/

    experimental::MaterialMetaDto meta {};

    {
        auto guard = material.diffuse()->acquire(resource::ResourceUsageFlag::eRead);
        if (guard.empty() || not guard->is<VirtualTextureView>()) {
            meta.diffuse = ~0;

        } else {
            meta.diffuse = guard->as<VirtualTextureView>()->baseLayer();
        }
    }

    {
        auto guard = material.normal()->acquire(resource::ResourceUsageFlag::eRead);
        if (guard.empty() || not guard->is<VirtualTextureView>()) {
            meta.normal = ~0;

        } else {
            meta.normal = guard->as<VirtualTextureView>()->baseLayer();
        }
    }

    {
        auto guard = material.roughness()->acquire(resource::ResourceUsageFlag::eRead);
        if (guard.empty() || not guard->is<VirtualTextureView>()) {
            meta.roughness = ~0;

        } else {
            meta.roughness = guard->as<VirtualTextureView>()->baseLayer();
        }
    }

    {
        auto guard = material.metalness()->acquire(resource::ResourceUsageFlag::eRead);
        if (guard.empty() || not guard->is<VirtualTextureView>()) {
            meta.metalness = ~0;

        } else {
            meta.metalness = guard->as<VirtualTextureView>()->baseLayer();
        }
    }

    {
        auto guard = material.ao()->acquire(resource::ResourceUsageFlag::eRead);
        if (guard.empty() || not guard->is<VirtualTextureView>()) {
            meta.ao = ~0;

        } else {
            meta.ao = guard->as<VirtualTextureView>()->baseLayer();
        }
    }

    {
        auto guard = material.alpha()->acquire(resource::ResourceUsageFlag::eRead);
        if (guard.empty() || not guard->is<VirtualTextureView>()) {
            meta.alpha = ~0;

        } else {
            meta.alpha = guard->as<VirtualTextureView>()->baseLayer();
        }
    }

    /**/

    const u64 alignedSize = (sizeof(experimental::MaterialMetaDto) / 128ui64) * 128ui64 + (
        (sizeof(experimental::MaterialMetaDto) % 128ui64) ? 128ui64 : 0ui64);

    Buffer stage {};
    makeStageBuffer(_pool->device(), alignedSize, stage);

    stage.mapAligned(sizeof(experimental::MaterialMetaDto));
    stage.write<experimental::MaterialMetaDto>(&meta, 1ui32);
    stage.flushAligned(sizeof(experimental::MaterialMetaDto));

    /**/

    {
        /**
         * Copy meta data from stage to cached buffer
         */
        auto* const cmdPool = _pool->device()->transferQueue()->pool();
        cmdPool->lck().acquire();

        CommandBuffer cmd = cmdPool->make();
        cmd.begin();

        const vk::BufferMemoryBarrier sbmb {
            vk::AccessFlags(), vk::AccessFlags(), VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, stage.buffer, 0ui32,
            sizeof(experimental::MaterialMetaDto)
        };

        const vk::BufferCopy region { 0ui32, material.view()->offset(), sizeof(experimental::MaterialMetaDto) };

        cmd.vkCommandBuffer().pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(),
            0,
            nullptr,
            1,
            &sbmb,
            0,
            nullptr
        );

        cmd.vkCommandBuffer().copyBuffer(stage.buffer, material.view()->owner()->vkBuffer(), 1ui32, &region);

        /**
         * Restore Buffer Layout
         */
        const vk::BufferMemoryBarrier ebmb {
            vk::AccessFlags(), vk::AccessFlags(), VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, stage.buffer, 0ui32,
            sizeof(experimental::MaterialMetaDto)
        };

        cmd.vkCommandBuffer().pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(),
            0,
            nullptr,
            1,
            &ebmb,
            0,
            nullptr
        );

        /**
         * Dispatch Commands
         */
        cmd.end();
        cmd.submitWait();
        cmd.release();

        cmdPool->lck().release();
    }

    /**
     * Cleanup and Return
     */

    destroyStageBuffer(_STD move(stage));
    dst->setAssociation(request_);

    return dst;
}

smr<engine::gfx::TextureResource> resolveTexture(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::AssetDatabase> database_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

    /**
     * Resolve texture asset from database
     */
    const auto query = database_->query(guid_);
    assert(query.exists());

    auto* asset = static_cast<ptr<engine::assets::Texture>>(query.get());

    #ifdef _DEBUG
    if (!asset->getClass()->isExactType<engine::assets::Texture>()) {
        __debugbreak();
    }
    #endif

    /**
     * Load texture asset to get the internal resource handle
     *  // TODO: This should be a non-residential load operation, cause we only need the handle
     */
    auto texture = loader_->loadImmediately<engine::assets::Texture, TextureResource>(
        _STD move(asset),
        TextureLoadOptions {}
    );

    /**/

    return texture;
}

void makeStageBuffer(cref<sptr<Device>> device_, const u64 size_, ref<Buffer> buffer_) {

    /**/

    vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(), size_, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, 0, nullptr
    };

    buffer_.buffer = device_->vkDevice().createBuffer(bci);
    buffer_.device = device_->vkDevice();
    buffer_.size = size_;

    /**/

    const auto allocated = memory::allocate(device_->allocator(), device_, MemoryProperty::eHostVisible, buffer_);

    assert(buffer_.buffer);
    assert(buffer_.memory);

    buffer_.bind();
}

void destroyStageBuffer(mref<Buffer> buffer_) {
    buffer_.destroy();
}
