#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "TextureLikeObject.hpp"
#include "__fwd.hpp"
#include "../TextureFormat.hpp"
#include "TextureType.hpp"
#include "../API/__vkFwd.hpp"

namespace hg::engine::gfx::loader {
    class TextureLoader;
}

namespace hg::engine::gfx {
    class VirtualTextureView final :
        public InheritMeta<VirtualTextureView, TextureLikeObject> {
    public:
        friend class VirtualTexture;
        friend class ::hg::engine::gfx::loader::TextureLoader;

    public:
        using this_type = VirtualTextureView;

    protected:
        VirtualTextureView(
            nmpt<VirtualTexture> owner_,
            mref<Vector<nmpt<VirtualTexturePage>>> pages_,
            math::uivec2 layers_,
            math::uivec3 extent_,
            TextureFormat format_,
            math::uivec2 mipLevels_,
            TextureType type_
        );

    public:
        VirtualTextureView(cref<this_type>) = delete;

        VirtualTextureView(mref<this_type>) noexcept = delete;

    public:
        ~VirtualTextureView() override;

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        nmpt<VirtualTexture> _owner;

    public:
        [[nodiscard]] const nmpt<const VirtualTexture> owner() const noexcept;

        [[nodiscard]] const nmpt<VirtualTexture> owner() noexcept;

    private:
        Vector<nmpt<VirtualTexturePage>> _pages;

    public:
        // TODO: Should be protected / private ...
        [[nodiscard]] cref<Vector<nmpt<VirtualTexturePage>>> pages() const noexcept;

    private:
        math::uivec2 _layers;

    public:
        [[nodiscard]] math::uivec2::value_type baseLayer() const noexcept;

        [[nodiscard]] math::uivec2::value_type layers() const noexcept;

    private:
        math::uivec3 _extent;

    public:
        [[nodiscard]] math::uivec3 extent() const noexcept;

        [[nodiscard]] math::uivec3::value_type width() const noexcept;

        [[nodiscard]] math::uivec3::value_type height() const noexcept;

        [[nodiscard]] math::uivec3::value_type depth() const noexcept;

    private:
        TextureFormat _format;

    public:
        [[nodiscard]] TextureFormat format() const noexcept;

    private:
        math::uivec2 _mipLevels;

    public:
        [[nodiscard]] math::uivec2::value_type minMipLevel() const noexcept;

        [[nodiscard]] math::uivec2::value_type mipLevels() const noexcept;

        [[nodiscard]] math::uivec2::value_type maxMipLevel() const noexcept;

    private:
        TextureType _type;

    public:
        [[nodiscard]] TextureType type() const noexcept;

    private:
        /**
         * Vulkan Virtual Texture (-View)
         */
        _::VkImageView _vkImageView;

    public:
        /**
         * Query the stored internal vkImageView
         */
        [[nodiscard]] _::VkImageView vkImageView() const noexcept;

        /**
         * Replaces the stored internal vkImageView
         *
         * @returns The previous vkImageView
         */
        [[nodiscard]] _::VkImageView vkImageView(mref<_::VkImageView> nextView_) noexcept;
    };
}
