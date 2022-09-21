#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/__default.inl>

#include "__fwd.hpp"
#include "../TextureFormat.hpp"
#include "TextureType.hpp"

namespace ember::engine::gfx::loader {
    class TextureLoader;
}

namespace ember::engine::gfx {

    class VirtualTextureView final {
    public:
        friend class VirtualTexture;
        friend class ::ember::engine::gfx::loader::TextureLoader;

    public:
        using this_type = VirtualTextureView;

    protected:
        VirtualTextureView(non_owning_rptr<VirtualTexture> owner_,
            mref<Vector<non_owning_rptr<VirtualTexturePage>>> pages_, math::uivec2 layers_, math::uivec3 extent_,
            TextureFormat format_, math::uivec2 mipLevels_, TextureType type_);

    public:
        VirtualTextureView(cref<this_type>) = delete;

        VirtualTextureView(mref<this_type>) noexcept = delete;

    public:
        ~VirtualTextureView();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        non_owning_rptr<VirtualTexture> _owner;

    public:
        [[nodiscard]] const non_owning_rptr<const VirtualTexture> owner() const noexcept;

        [[nodiscard]] const non_owning_rptr<VirtualTexture> owner() noexcept;

    private:
        Vector<non_owning_rptr<VirtualTexturePage>> _pages;

    private:
        math::uivec2 _layers;

    public:
        [[nodiscard]] math::uivec2::value_type baseLayer() const noexcept;

        [[nodiscard]] math::uivec2::value_type layers() const noexcept;

    private:
        math::uivec3 _extent;

    public:
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
    };

}
