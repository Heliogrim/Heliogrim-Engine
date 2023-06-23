#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX.Acc/AccelerationStageTransferToken.hpp>

#include "CompiledModule.hpp"

namespace hg::engine::gfx::acc {
    struct VkBindingLocation final {
        s32 set;
        s32 location;
    };

    class VkCompiledModule final :
        public CompiledModule {
    public:
        using this_type = VkCompiledModule;

    public:
        constexpr VkCompiledModule() noexcept = default;

        VkCompiledModule(mref<_::VkShaderModule> shaderModule_) noexcept;

        VkCompiledModule(mref<this_type> other_) noexcept = default;

        VkCompiledModule(cref<this_type> other_) = delete;

        ~VkCompiledModule() override;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

        ref<this_type> operator=(cref<this_type> other_) = delete;

    public:
        _::VkShaderModule shaderModule { nullptr };

    private:
        DenseMap<AccelerationStageTransferToken, VkBindingLocation> _mappedLocations;

    public:
        [[nodiscard]] VkBindingLocation queryBindLocation(cref<AccelerationStageTransferToken> token_) const noexcept;
    };
}
