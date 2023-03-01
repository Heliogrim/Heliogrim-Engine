#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Extent.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx {
    class TextureView final {
    public:
        friend class Texture;

    public:
        using this_type = TextureView;

    protected:
        TextureView(
            non_owning_rptr<Texture> owner_,
            math::uivec2 layers_,
            math::uExtent3D extent_,
            math::uivec2 mipLevels_
        );

    public:
        TextureView(cref<this_type>) = delete;

        TextureView(mref<this_type>) noexcept = delete;

    public:
        ~TextureView();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        non_owning_rptr<Texture> _owner;

    public:
        [[nodiscard]] const non_owning_rptr<const Texture> owner() const noexcept;

        [[nodiscard]] const non_owning_rptr<Texture> owner() noexcept;

    private:
        math::uivec2 _layers;

    public:
        [[nodiscard]] math::uivec2::value_type baseLayer() const noexcept;

        [[nodiscard]] math::uivec2::value_type layers() const noexcept;

    private:
        math::uExtent3D _extent;

    public:
        [[nodiscard]] math::uExtent3D::off_type offsetX() const noexcept;

        [[nodiscard]] math::uExtent3D::off_type offsetY() const noexcept;

        [[nodiscard]] math::uExtent3D::off_type offsetZ() const noexcept;

        [[nodiscard]] math::uExtent3D::value_type width() const noexcept;

        [[nodiscard]] math::uExtent3D::value_type height() const noexcept;

        [[nodiscard]] math::uExtent3D::value_type depth() const noexcept;

    public:
        [[nodiscard]] math::vec2 minSt() const noexcept;

        [[nodiscard]] math::vec2 maxSt() const noexcept;

    private:
        math::uivec2 _mipLevels;

    public:
        [[nodiscard]] math::uivec2::value_type minMipLevel() const noexcept;

        [[nodiscard]] math::uivec2::value_type mipLevels() const noexcept;

        [[nodiscard]] math::uivec2::value_type maxMipLevel() const noexcept;
    };
}
