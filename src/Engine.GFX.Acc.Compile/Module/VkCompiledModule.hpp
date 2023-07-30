#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX.Acc/Stage/TransferToken.hpp>

#include "CompiledModule.hpp"
#include "VkBindingLocation.hpp"

namespace hg::engine::gfx::acc {
    class VkCompiledModule final :
        public CompiledModule {
    public:
        using this_type = VkCompiledModule;

    public:
        constexpr VkCompiledModule() noexcept = default;

        VkCompiledModule(
            mref<_::VkShaderModule> shaderModule_,
            mref<DenseMap<TransferToken, VkBindingLocation>> mapping_
        ) noexcept;

        VkCompiledModule(mref<this_type> other_) noexcept = default;

        VkCompiledModule(cref<this_type> other_) = delete;

        ~VkCompiledModule() override;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

        ref<this_type> operator=(cref<this_type> other_) = delete;

    public:
        _::VkShaderModule shaderModule { nullptr };

    private:
        DenseMap<TransferToken, VkBindingLocation> _mappedLocations;

    public:
        [[nodiscard]] VkBindingLocation queryBindLocation(cref<TransferToken> token_) const noexcept;
    };
}
