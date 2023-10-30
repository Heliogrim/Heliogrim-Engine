#include "StaticGeometryModel.hpp"

#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Engine.Core/Engine.hpp>

#include "ModelDataTokens.hpp"
#include "StaticGeometryBatch.hpp"
#include "../Cache/CacheResult.hpp"
#include "Engine.Assets/Types/Material/GfxMaterial.hpp"
#include "Engine.GFX.Loader/Geometry/Traits.hpp"
#include "Engine.GFX.Loader/Material/Traits.hpp"
#include "../Buffer/Buffer.hpp"
#include "Engine.GFX.Glow.3D/Renderer/State/RevSfMtt.hpp"
#include "Engine.Common/Math/Convertion.hpp"
#include <Engine.GFX.Render.Command/Commands/BindMaterial.hpp>

using namespace hg::engine::gfx;
using namespace hg;

StaticGeometryModel::StaticGeometryModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_) {}

StaticGeometryModel::~StaticGeometryModel() {
    tidy();
}

void StaticGeometryModel::tidy() {}

void StaticGeometryModel::create(const ptr<::hg::engine::scene::Scene> scene_) {

    auto* origin { static_cast<ptr<StaticGeometryComponent>>(_owner) };

    /**
     *
     */
    _boundary = origin->getBoundaries();
    _staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getStaticGeometryAsset().internal());
    _staticGeometryResource = Engine::getEngine()->getResources()->loader().
                                                   load(_staticGeometryAsset, nullptr).
                                                   into<StaticGeometryResource>();

    /**
     *
     */
    for (const auto& material : origin->overrideMaterials()) {
        auto* wrapped { static_cast<ptr<assets::GfxMaterial>>(material.internal()) };
        auto resource = Engine::getEngine()->getResources()->loader().load<assets::GfxMaterial, MaterialResource>(
            _STD move(wrapped),
            {}
        );

        _overrideMaterials.push_back(resource);
    }
}

void StaticGeometryModel::update(const ptr<::hg::engine::scene::Scene> scene_) {

    auto* origin = static_cast<ptr<StaticGeometryComponent>>(_owner);

    /* Geometry Changes */

    {
        const auto outer = origin->getStaticGeometryGuid();
        const auto inner = _staticGeometryAsset->get_guid();

        if (outer != inner) {

            _staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(
                origin->getStaticGeometryAsset().internal()
            );
            _staticGeometryResource = Engine::getEngine()->getResources()->loader().load(
                _staticGeometryAsset,
                nullptr
            ).into<StaticGeometryResource>();

        }
    }

    /* Material Changes */

    const auto count = origin->overrideMaterials().size();
    for (u32 matIdx = 0; matIdx < count; ++matIdx) {

        auto* outer = static_cast<ptr<assets::GfxMaterial>>(origin->overrideMaterials()[matIdx].internal());
        const auto& inner = _overrideMaterials[matIdx];

        if (inner->getAssociation() == nullptr) {
            continue;
        }

        // TODO: Check whether pointer comparison is actually safe, or whether should use guid compare
        if (inner->getAssociation() != outer) {

            _overrideMaterials[matIdx] = Engine::getEngine()->getResources()->loader().load<
                assets::GfxMaterial, MaterialResource
            >(
                _STD move(outer),
                {}
            );

        }
    }

}

void StaticGeometryModel::destroy(const ptr<::hg::engine::scene::Scene> scene_) {}

Vector<render::RenderDataToken> StaticGeometryModel::providedToken() const noexcept {
    using namespace ::hg::engine::gfx::render;
    return { StaticModelGeometry, StaticModelTransform };
}

ptr<cache::ModelBatch> StaticGeometryModel::batch(const ptr<render::RenderPassState> state_) {

    /*
    // TODO: Change getting transform information to resolve via render graph
    auto* origin { static_cast<ptr<StaticGeometryComponent>>(_owner) };
    auto* res { static_cast<ptr<StaticGeometryResource>>(_staticGeometryResource) };

    return ModelBatch {
        origin->getWorldTransform(),
        res->_vertexData.buffer,
        res->_indexData.buffer,
        static_cast<u32>(res->_indexData.buffer.size / sizeof(u32)),
        0ui32
    };
     */

    /**
     * Acquire ModelBatch
     */
    ptr<cache::ModelBatch> result { nullptr };
    const auto cacheResult { state_->cacheCtrl.cache()->fetch(reinterpret_cast<ptrdiff_t>(owner()), result) };
    if (cacheResult != cache::CacheResult::eHit) {
        /**
         * On cache miss create new instance and store back for further usage
         */
        result = new StaticGeometryBatch();
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
    assert(result->getMetaClass()->exact<StaticGeometryBatch>());
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
        const auto rotation = math::as<math::quaternion, math::mat4>(
            _owner->getWorldTransform().rotator().quaternion()
        );
        const auto scale { math::mat4::make_identity().unchecked_scale(_owner->getWorldTransform().scale()) };

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

void StaticGeometryModel::render(mref<nmpt<render::cmd::RenderCommandBuffer>> cmd_) const {

    const auto worldTransform = _owner->getWorldTransform();
    const auto trans { math::mat4::make_identity().translate(worldTransform.location().operator math::fvec3()) };
    const auto rotation = math::as<math::quaternion, math::mat4>(worldTransform.rotator().quaternion());
    const auto scale { math::mat4::make_identity().unchecked_scale(worldTransform.scale()) };

    /**/

    for (const auto& material : _overrideMaterials) {

        const auto guard = material->acquire(resource::ResourceUsageFlag::eRead);
        cmd_->bindMaterial(MaterialIdentifier {}, guard.imm());
    }

    const auto meshGuard = _staticGeometryResource->acquire(resource::ResourceUsageFlag::eRead);
    cmd_->bindStaticMesh(meshGuard.imm());

    cmd_->bindStaticMeshInstance(nullptr/*{ trans * rotation * scale }*/);
    cmd_->drawStaticMeshIdx(1uL, 0uL, meshGuard->indices()->size(), 0uL);
}

const ptr<engine::assets::StaticGeometry> StaticGeometryModel::geometryAsset() const noexcept {
    return _staticGeometryAsset;
}

cref<smr<StaticGeometryResource>> StaticGeometryModel::geometryResource() const noexcept {
    return _staticGeometryResource;
}

bool StaticGeometryModel::streamable() const noexcept {
    return _streamable;
}
