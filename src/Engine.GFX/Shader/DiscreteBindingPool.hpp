#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "DiscreteBindingGroup.hpp"
#include "ShaderBindingGroupLayout.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class Device;
}

namespace ember::engine::gfx {

    class DiscreteBindingPool {
    public:
        using this_type = DiscreteBindingPool;

    public:
        DiscreteBindingPool(
            cref<sptr<Device>> device_,
            cref<shader::ShaderBindingGroupLayout> layout_,
            const u32 sets_
        ) noexcept;

        DiscreteBindingPool(cref<DiscreteBindingPool>) = delete;

        DiscreteBindingPool(mref<DiscreteBindingPool> other_) noexcept = default;

        ~DiscreteBindingPool();

    public:
        ref<DiscreteBindingPool> operator=(cref<DiscreteBindingPool>) = delete;

        ref<DiscreteBindingPool> operator=(mref<DiscreteBindingPool> other_) noexcept = default;

    private:
        void tidy();

        void setup();

    private:
        sptr<Device> _device;

    private:
        shader::ShaderBindingGroupLayout _layout;

    public:
        [[nodiscard]] cref<shader::ShaderBindingGroupLayout> layout() const noexcept;

    private:
        u32 _maxSets;
        u32 _currentSets;

        volatile bool _exceeded;

    public:
        [[nodiscard]] u32 maxSets() const noexcept;

        [[nodiscard]] u32 leftSets() const noexcept;

        [[nodiscard]] bool exceeded() const noexcept;

    public:
        void markAsExceeded() noexcept;

    private:
        vk::DescriptorPool _vkPool;

    public:
        [[nodiscard]] cref<vk::DescriptorPool> vkPool() const noexcept;

    public:
        [[nodiscard]] shader::DiscreteBindingGroup allocate(cref<shader::ShaderBindingGroup> group_);

        void free(mref<shader::DiscreteBindingGroup> group_);
    };
}
