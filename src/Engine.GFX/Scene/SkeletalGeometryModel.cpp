#include "SkeletalGeometryModel.hpp"

#include <Engine.GFX.RenderPipeline/Invocation/IGCommandBuffer.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

SkeletalGeometryModel::SkeletalGeometryModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _skeletalGeometry(),
    _skeletalGeometryResource(),
    _streamable() {}

SkeletalGeometryModel::~SkeletalGeometryModel() = default;

const ptr<StaticGeometryComponent> SkeletalGeometryModel::owner() const noexcept {
    return static_cast<const ptr<StaticGeometryComponent>>(_owner);
}

void SkeletalGeometryModel::create(const ptr<scene::Scene> scene_) {

    auto* const origin = owner();

}

void SkeletalGeometryModel::update(const ptr<scene::Scene> scene_) {

    auto* const origin = owner();

}

void SkeletalGeometryModel::destroy(const ptr<scene::Scene> scene_) {

    auto* const origin = owner();

}

void SkeletalGeometryModel::render(mref<nmpt<IGCommandBuffer>> cmd_) const {

    cmd_->bindVertexBuffer(nullptr);
    cmd_->bindIndexBuffer(nullptr);

    cmd_->bindStorage(nullptr);

    cmd_->drawIndexed(0uL, 0uL);
}
