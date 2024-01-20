#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Scene/SceneSystem.hpp>

#include "RenderSceneRegistry.hpp"

namespace hg::engine::gfx {
    class SceneResourcePool;
}

namespace hg::engine::render {
    class RenderSceneSystem :
        public scene::SceneSystem<RenderSceneSystem> {
    public:
        using this_type = RenderSceneSystem;

    public:
        RenderSceneSystem();

        ~RenderSceneSystem() noexcept override;

    public:
        void prepare() override;

        void cleanup() override;

    private:
        uptr<hg::engine::gfx::SceneResourcePool> _sceneResourcePool;

    public:
        [[nodiscard]] nmpt<hg::engine::gfx::SceneResourcePool> getSceneResourcePool() const noexcept;

    public:
        void broadcast(scene::SceneBroadcastFlags flags_) override;

        void update(scene::SceneUpdateFlags flags_) override;

        void postprocess(scene::ScenePostProcessFlags flags_) override;

    private:
        RenderSceneRegistry _storage;

    public:
        [[nodiscard]] ref<RenderSceneRegistry> getRegistry() noexcept;

    public:
        void add(const ptr<const MetaClass> meta_, std::span<const ptr<SceneComponent>> batch_) override;

        void remove(const ptr<const MetaClass> meta_, std::span<const ptr<const SceneComponent>> batch_) override;

        void clear() override;
    };
}
