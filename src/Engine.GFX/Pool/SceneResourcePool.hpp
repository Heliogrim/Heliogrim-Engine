#pragma once
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Scene/Scene.hpp>

#include "InstancePooled.hpp"
#include "../Device/Device.hpp"
#include "../Aabb.hpp"
#include "../Light/Light.hpp"

namespace hg::engine::gfx {
    class SceneResourcePool {
    public:
        using this_type = SceneResourcePool;

    public:
        SceneResourcePool(cref<sptr<Device>> device_);

        SceneResourcePool(cref<this_type>) = delete;

        SceneResourcePool(mref<this_type>) = delete;

        ~SceneResourcePool();

    public:
        void setup();

        void destroy();

    private:
        sptr<Device> _device;
        nmpt<engine::scene::Scene> _scene;

    public:
        [[nodiscard]] sptr<Device> device() const noexcept;

        [[nodiscard]] nmpt<engine::scene::Scene> scene() const noexcept;

    private:
    public:
        InstancePooled<math::mat4> staticInstancePool;
        InstancePooled<Aabb3d> staticAabbPool;

    private:
    public:
        GlslSceneLightInfo sceneLightInfo;

        TypeBuffer<GlslSceneLightInfo> sceneLightInfoBuffer;
        InstancePooled<GlslLight> lightSourcePool;
    };
}
