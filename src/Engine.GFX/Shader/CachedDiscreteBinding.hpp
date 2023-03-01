#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "DiscreteBindingGroup.hpp"

namespace hg::engine::gfx {
    class CachedDiscreteBinding {
    public:
        using this_type = CachedDiscreteBinding;

    public:
        CachedDiscreteBinding(mref<shader::DiscreteBindingGroup> binding_);

        CachedDiscreteBinding(cref<CachedDiscreteBinding>) = delete;

        CachedDiscreteBinding(mref<CachedDiscreteBinding> other_) noexcept;

        ~CachedDiscreteBinding();

    public:
        ref<CachedDiscreteBinding> operator=(cref<CachedDiscreteBinding>) = delete;

        ref<CachedDiscreteBinding> operator=(mref<CachedDiscreteBinding> other_) noexcept;

    public:
        void release();

    private:
        shader::DiscreteBindingGroup _binding;

    public:
        ref<shader::DiscreteBindingGroup> binding() noexcept;

        ref<CachedDiscreteBinding> operator=(mref<shader::DiscreteBindingGroup> dbg_);
    };
}
