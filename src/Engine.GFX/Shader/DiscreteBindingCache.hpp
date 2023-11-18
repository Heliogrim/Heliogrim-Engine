#pragma once

#include <unordered_map>
#include <Engine.Common/Wrapper.hpp>

#include "ShaderBindingGroupLayout.hpp"
#include "DiscreteBindingGroup.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration
     */
    class Device;
    class DiscreteBindingPool;
}

namespace hg::engine::gfx {
    // TODO: Think about, whether we want the DiscreteBindingCache as an global instance sharing every pool over multiple HORenderPasses, which would enable us to cache the descriptor set for any component resource, but would make the management a bit chunky
    class DiscreteBindingCache {
    public:
        using this_type = DiscreteBindingCache;

        constexpr static u32 initial_pool_size { 15ui32 };

    public:
        DiscreteBindingCache(cref<sptr<Device>> device_) noexcept;

        ~DiscreteBindingCache();

    public:
        void tidy();

        bool prepare(cref<shader::ShaderBindingGroupLayout> layout_, const u32 sets_);

    private:
        sptr<Device> _device;

    private:
        _STD unordered_map<shader::ShaderBindingGroupLayout, Vector<ptr<DiscreteBindingPool>>> _mappedPools;

    public:
        [[nodiscard]] shader::DiscreteBindingGroup allocate(cref<shader::ShaderBindingGroup> group_);

        ref<shader::DiscreteBindingGroup> reallocate(ref<shader::DiscreteBindingGroup> group_);

        void free(mref<shader::DiscreteBindingGroup> group_);
    };
}
