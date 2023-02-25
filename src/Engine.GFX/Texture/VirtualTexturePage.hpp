#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/__default.inl>

#include "VirtualTexturePageFlag.hpp"
#include "__fwd.hpp"
#include "../Device/Device.hpp"
#include "../Memory/VirtualMemoryPage.hpp"

namespace ember::engine::gfx {
    class VirtualTexturePage final {
    public:
        friend class VirtualTexture;

    public:
        using this_type = VirtualTexturePage;

    protected:
        VirtualTexturePage(
            non_owning_rptr<VirtualMemoryPage> memory_,
            u32 layer_,
            math::uivec3 offset_,
            math::uivec3 extent_,
            u32 mipLevel_,
            const VirtualTexturePageFlags flags_ = {}
        );

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        ~VirtualTexturePage() noexcept;

    private:
        VirtualTexturePageFlags _flags;

    public:
        [[nodiscard]] const VirtualTexturePageFlags flags() const noexcept;

    private:
        non_owning_rptr<VirtualMemoryPage> _memory;

    public:
        /**
         * Get the underlying paged virtual memory
         *
         * @author Julius
         * @date 19.06.2022
         *
         * @returns A non owning pointer to the const VirtualMemoryPage.
         */
        [[nodiscard]] const non_owning_rptr<VirtualMemoryPage> memory() const noexcept;

        /**
         * Check whether this is currently residential
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool residential() const noexcept;

        bool load();

        bool unload();

    private:
        u32 _layer;
        math::uivec3 _offset;
        math::uivec3 _extent;
        u32 _mipLevel;

    public:
        [[nodiscard]] u32 layer() const noexcept;

        [[nodiscard]] cref<math::uivec3> offset() const noexcept;

        [[nodiscard]] cref<math::uivec3> extent() const noexcept;

        [[nodiscard]] u32 mipLevel() const noexcept;

    protected:
        [[nodiscard]] vk::SparseImageMemoryBind vkSparseImageMemoryBind() const noexcept;

        [[nodiscard]] vk::SparseMemoryBind vkSparseMemoryBind() const noexcept;
    };
}
