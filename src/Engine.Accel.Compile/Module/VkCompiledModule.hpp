#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics/API/__vkFwd.hpp>

#include "CompiledModule.hpp"
#include "VkModuleSource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
    class VkCompiledModule final :
        public CompiledModule {
    public:
        using this_type = VkCompiledModule;

    public:
        constexpr VkCompiledModule() noexcept = default;

        VkCompiledModule(
            mref<_::VkShaderModule> shaderModule_,
            mref<MappedBindings> bindings_
        ) noexcept;

        VkCompiledModule(mref<this_type> other_) noexcept = default;

        VkCompiledModule(cref<this_type> other_) = delete;

        ~VkCompiledModule() override;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

        ref<this_type> operator=(cref<this_type> other_) = delete;

    public:
        MappedBindings bindings;
        _::VkShaderModule shaderModule { nullptr };
    };
}
