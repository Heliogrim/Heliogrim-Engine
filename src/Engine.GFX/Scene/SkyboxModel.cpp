#include "SkyboxModel.hpp"

#include <Heliogrim/SkyboxComponent.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Engine.Core/Engine.hpp>

#include "ModelDataTokens.hpp"
#include "StaticGeometryBatch.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Cache/CacheResult.hpp"
#include "Engine.Assets/Types/GfxMaterial.hpp"
#include "Engine.GFX.Glow.3D/Renderer/State/RevSfMtt.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SkyboxModel::SkyboxModel(const ptr<SceneComponent> owner_) :
    GeometryModel(owner_) {}

SkyboxModel::~SkyboxModel() {
    tidy();
}

void SkyboxModel::tidy() {}

void SkyboxModel::create(const ptr<scene::Scene> scene_) {

    auto* origin { static_cast<ptr<SkyboxComponent>>(_owner) };

    /**
     *
     */
    _boundary = origin->getBoundaries();
    _skyboxGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getSkyboxGeometryAsset().internal());
    _skyboxGeometryResource = Engine::getEngine()->getResources()->loader().load(_skyboxGeometryAsset, nullptr);

    /**
     *
     */
    auto materialAsset = static_cast<ptr<assets::GfxMaterial>>(origin->getSkyboxMaterialAsset().internal());
    auto materialResource = Engine::getEngine()->getResources()->loader().load(materialAsset, nullptr);

    _overrideMaterials.push_back(materialResource.into<MaterialResource>());
}

void SkyboxModel::update(const ptr<scene::Scene> scene_) {}

void SkyboxModel::destroy(const ptr<scene::Scene> scene_) {}

Vector<render::RenderDataToken> SkyboxModel::providedToken() const noexcept {
    using namespace ::hg::engine::gfx::render;
    return { StaticModelGeometry, StaticModelTransform };
}

ptr<cache::ModelBatch> SkyboxModel::batch(const ptr<render::RenderPassState> state_) {

    /**
     * Acquire ModelBatch
     */
    ptr<cache::ModelBatch> result { nullptr };
    const auto cacheResult { state_->cacheCtrl.cache()->fetch(reinterpret_cast<ptrdiff_t>(owner()), result) };
    if (cacheResult != cache::CacheResult::eHit) {
        /**
         * On cache miss create new instance and store back for further usage
         */
        result = HeliogrimObject::create<StaticGeometryBatch>();
        state_->cacheCtrl.cache()->store(reinterpret_cast<ptrdiff_t>(owner()), ptr<cache::ModelBatch> { result });
        // TODO: Check whether we can improve storing a copy of the pointer, cause value copy currently requires a explicit copy
    }

    /**
     * Update ModelBatch
     */
    auto* const batch { static_cast<ptr<StaticGeometryBatch>>(result) };

    // TODO: Transition of State
    #ifdef _DEBUG
    assert(result != nullptr);
    assert(result->getClass()->isExactType<StaticGeometryBatch>());
    #endif

    #pragma region Instance Data
    // TODO: Temporary
    if (!batch->instance.memory) [[unlikely]]
    {
        auto& buffer { batch->instance };
        buffer.device = state_->device->vkDevice();
        buffer.size = static_cast<u64>(sizeof(math::mat4));
        buffer.usageFlags = vk::BufferUsageFlagBits::eStorageBuffer;

        const vk::BufferCreateInfo ci { {}, buffer.size, buffer.usageFlags, vk::SharingMode::eExclusive, 0, nullptr };
        buffer.buffer = state_->device->vkDevice().createBuffer(ci);
        assert(buffer.buffer);

        const auto result {
            memory::allocate(
                &state_->alloc,
                state_->device,
                buffer.buffer,
                MemoryProperty::eHostVisible,
                buffer.memory
            )
        };// TODO: Handle failed allocation
        buffer.bind();
    }

    if (batch->instance.memory) {
        /**
         * Push Model Data
         */
        const auto trans {
            math::mat4::make_identity().translate(_owner->getWorldTransform().location().operator math::fvec3())
        };
        const auto scale { math::mat4::make_identity().unchecked_scale(_owner->getWorldTransform().scale()) };

        const auto euler { _owner->getWorldTransform().rotator().euler() };
        auto rotation { math::mat4::make_identity() };
        rotation.rotate(euler.x, math::vec3_pitch);
        rotation.rotate(euler.y, math::vec3_yaw);
        rotation.rotate(euler.z, math::vec3_roll);

        const auto mm { trans * rotation * scale };
        batch->instance.write<math::mat4>(&mm, 1ui32);
    }

    #pragma endregion

    #pragma region Material Translation Table

    // TODO: Temporary
    if (!batch->mtt.memory) [[unlikely]]
    {
        auto& buffer { batch->mtt };
        buffer.device = state_->device->vkDevice();
        buffer.size = static_cast<u64>(sizeof(u32) * _overrideMaterials.size());
        buffer.usageFlags = vk::BufferUsageFlagBits::eStorageBuffer;

        const vk::BufferCreateInfo ci { {}, buffer.size, buffer.usageFlags, vk::SharingMode::eExclusive, 0, nullptr };
        buffer.buffer = state_->device->vkDevice().createBuffer(ci);
        assert(buffer.buffer);

        const auto result {
            memory::allocate(
                &state_->alloc,
                state_->device,
                buffer.buffer,
                MemoryProperty::eHostVisible,
                buffer.memory
            )
        };// TODO: Handle failed allocation
        buffer.bind();
    }

    if (batch->mtt.memory) {
        /**
         * Push Material Translation
         */

        const auto& data { state_->data };
        const auto mttEntry { data.at("RevEarlySFNode::SfMtt"sv) };
        auto& mtt { *_STD static_pointer_cast<glow::render::RevSfMtt, void>(mttEntry) };

        const auto dataSize { sizeof(u32) * _overrideMaterials.size() };
        //batch->mtt.mapAligned(dataSize);
        batch->mtt.map(dataSize);

        for (u32 i { 0ui32 }; i < _overrideMaterials.size(); ++i) {
            const auto te { mtt.insert(_overrideMaterials[i].get()) };
            static_cast<ptr<u32>>(batch->mtt.memory->mapping)[i] = te;
        }

        //batch->mtt.flushAligned(dataSize);
        batch->mtt.unmap();
    }

    #pragma endregion

    /**
     * Bind Data
     */
    if (cacheResult != cache::eHit) {
        // TODO:
    }

    //
    return result;
}

const ptr<engine::assets::StaticGeometry> SkyboxModel::geometryAsset() const noexcept {
    return _skyboxGeometryAsset;
}

cref<smr<engine::resource::ResourceBase>> SkyboxModel::geometryResource() const noexcept {
    return _skyboxGeometryResource;
}

bool SkyboxModel::streamable() const noexcept {
    return _streamable;
}
