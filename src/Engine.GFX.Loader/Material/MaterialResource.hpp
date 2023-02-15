#pragma once

#include <Engine.GFX/Buffer/__fwd.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "../Texture/TextureResource.hpp"

namespace ember::engine::gfx::loader {
    class Material final {
    public:
        using this_type = Material;

    public:
        constexpr Material() noexcept;

        ~Material();

    private:
        smr<TextureResource> _diffuse;
        smr<TextureResource> _normal;
        smr<TextureResource> _roughness;
        smr<TextureResource> _metalness;
        smr<TextureResource> _ao;
        smr<TextureResource> _alpha;

    public:
        [[nodiscard]] cref<smr<TextureResource>> diffuse() const noexcept;

        [[nodiscard]] cref<smr<TextureResource>> normal() const noexcept;

        [[nodiscard]] cref<smr<TextureResource>> roughness() const noexcept;

        [[nodiscard]] cref<smr<TextureResource>> metalness() const noexcept;

        [[nodiscard]] cref<smr<TextureResource>> ao() const noexcept;

        [[nodiscard]] cref<smr<TextureResource>> alpha() const noexcept;

    private:
        uptr<VirtualBufferView> _view;

    public:
        [[nodiscard]] const non_owning_rptr<const VirtualBufferView> view() const noexcept;
    };
}

namespace ember::engine::gfx {
    typedef resource::Resource<loader::Material> MaterialResource;
}
