#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX/Material/Material.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx {
    // Error: Breaking forward declaration
    struct MaterialIdentifier {};
}

namespace hg::engine::gfx::render::cmd {
    class BindMaterialRenderCommand :
        public RenderCommand {
    public:
        using this_type = BindMaterialRenderCommand;

    public:
        constexpr BindMaterialRenderCommand(
            mref<MaterialIdentifier> identifier_,
            mref<const ptr<const material::Material>> material_
        ) noexcept :
            RenderCommand(),
            _identifier(_STD move(identifier_)),
            _material(_STD move(material_)) { }

        constexpr ~BindMaterialRenderCommand() noexcept = default;

    private:
        const MaterialIdentifier _identifier;
        const ptr<const material::Material> _material;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) noexcept override;
    };
}
