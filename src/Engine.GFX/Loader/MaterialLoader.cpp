#include "MaterialLoader.hpp"

#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Session/Session.hpp>

#include "../Resource/MaterialResource.hpp"
#include "../Resource/TextureResource.hpp"
#include "Engine.GFX/Buffer/Buffer.hpp"
#include "Engine.GFX/Cache/GlobalCacheCtrl.hpp"
#include "Engine.GFX/Cache/GlobalResourceCache.hpp"
#include "Engine.GFX/Material/MaterialMetaDto.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "../Command/CommandBuffer.hpp"
#include "../Command/CommandQueue.hpp"

using namespace ember::engine::gfx;
using namespace ember;

MaterialLoader::MaterialLoader(const ptr<cache::GlobalCacheCtrl> cache_) :
    _cacheCtrl(cache_) {}

[[nodiscard]] ptr<TextureResource> resolveTextureResource(
    cref<asset_guid> guid_,
    const ptr<const engine::assets::AssetDatabase> database_,
    const ptr<engine::res::LoaderManager> loader_
) {
    #ifdef _DEBUG
    assert(not not guid_);
    #endif

    /**
     * Resolve texture assets
     */
    const auto query { database_->query(guid_) };
    assert(query.exists());

    auto* const textureAsset { query.get() };

    #ifdef _DEBUG
    if (!textureAsset->getClass()->isExactType<engine::assets::Texture>()) {
        __debugbreak();
    }
    #endif

    /**
     * Load texture assets -> Get resource handler
     */
    auto* const texture { static_cast<const ptr<TextureResource>>(loader_->loadImmediately(textureAsset)) };

    /**
     *
     */
    return texture;
}

void MaterialLoader::makeStageBuffer(const u64 size_, ref<Buffer> stageBuffer_) {

    vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(),
        size_,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    stageBuffer_.buffer = _cacheCtrl->cache()->device()->vkDevice().createBuffer(bci);
    stageBuffer_.device = _cacheCtrl->cache()->device()->vkDevice();
    stageBuffer_.size = size_;

    const auto allocResult {
        memory::allocate(
            _cacheCtrl->cache()->device()->allocator(),
            _cacheCtrl->cache()->device(),
            MemoryProperty::eHostVisible,
            stageBuffer_
        )
    };

    assert(stageBuffer_.buffer);
    assert(stageBuffer_.memory);

    stageBuffer_.bind();
}

void MaterialLoader::destroyStageBuffer(mref<Buffer> stageBuffer_) {
    stageBuffer_.destroy();
}

MaterialLoader::result_type MaterialLoader::operator()(const ptr<assets::GfxMaterial> asset_, options_type options_) {

    const auto* const db { Session::get()->modules().assetDatabase() };

    /**
     *
     */
    auto* res { _cacheCtrl->cache()->request(asset_) };

    /**
     *
     */
    if (!res->_payload.__pseudo_stored) {

        auto& loader { Session::get()->modules().resourceManager()->loader() };

        res->_payload.diffuse = resolveTextureResource(asset_->diffuse(), db, &loader);
        res->_payload.normal = resolveTextureResource(asset_->normal(), db, &loader);
        res->_payload.roughness = resolveTextureResource(asset_->roughness(), db, &loader);
        res->_payload.ao = resolveTextureResource(asset_->ao(), db, &loader);

        // TODO: Make meta data stored to virtual buffer view of material aggregate buffer

        experimental::MaterialMetaDto data {
            .diffuse = static_cast<u16>(res->_payload.diffuse->_payload.view->baseLayer()),
            .normal = static_cast<u16>(res->_payload.normal->_payload.view->baseLayer()),
            .roughness = static_cast<u16>(res->_payload.roughness->_payload.view->baseLayer()),
            .ao = static_cast<u16>(res->_payload.ao->_payload.view->baseLayer())
        };

        Buffer stage {};
        makeStageBuffer(sizeof(experimental::MaterialMetaDto), stage);

        stage.mapAligned();
        stage.write<experimental::MaterialMetaDto>(&data, 1ui32);
        stage.flushAligned();

        {
            /**
             * Copy data from stage to cached buffer
             */
            auto pool = _cacheCtrl->cache()->device()->transferQueue()->pool();
            pool->lck().acquire();
            CommandBuffer cmd = pool->make();
            cmd.begin();

            const vk::BufferMemoryBarrier sbmb {
                vk::AccessFlags(),
                vk::AccessFlags(),
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                stage.buffer,
                0ui32,
                stage.size
            };

            const vk::BufferCopy region {
                0ui32,
                res->_payload.view->offset(),
                stage.size
            };

            cmd.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags(),
                0, nullptr,
                1, &sbmb,
                0, nullptr
            );

            cmd.vkCommandBuffer().copyBuffer(
                stage.buffer,
                res->_payload.view->owner()->vkBuffer(),
                1ui32,
                &region
            );

            /**
             * Restore Layout
             */
            const vk::BufferMemoryBarrier ebmb {
                vk::AccessFlags(),
                vk::AccessFlags(),
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                stage.buffer,
                0ui32,
                stage.size
            };

            cmd.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags(),
                0, nullptr,
                1, &ebmb,
                0, nullptr
            );

            cmd.end();
            cmd.submitWait();
            cmd.release();

            pool->lck().release();
        }

        destroyStageBuffer(_STD move(stage));

        res->_payload.__pseudo_stored = true;
    }

    /**
     *
     */

    return res;
}
