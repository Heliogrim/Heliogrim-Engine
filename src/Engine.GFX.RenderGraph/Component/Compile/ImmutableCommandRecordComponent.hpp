#pragma once

#include <variant>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>

#include "CommandRecordComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileImmutableCommandRecordComponent final :
        public InheritMeta<CompileImmutableCommandRecordComponent, CompileCommandRecordComponent> {
    public:
        using this_type = CompileImmutableCommandRecordComponent;

        using ini_stg_type = smr<StaticGeometryResource>;
        using ini_fn_type = _STD function<void(mref<nmpt<cmd::RenderCommandBuffer>> cmd_)>;

    private:
        _STD variant<ini_stg_type, ini_fn_type> _initializer;

    public:
        void setInitializer(mref<ini_stg_type> staticGeometryInitializer_);

        void setInitializer(mref<ini_fn_type> function_);

    public:
        [[nodiscard]] uptr<CommandRecordComponent> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
