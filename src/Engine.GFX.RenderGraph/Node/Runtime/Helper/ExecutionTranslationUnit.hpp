#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandTranslationUnit.hpp>

#include "../../../Symbol/SymbolizedResource.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffect;
    struct EffectCompileResult;
}

namespace hg::engine::gfx::material {
    class Material;
}

namespace hg::engine::gfx::render::graph {
    class ExecutionTranslationUnit :
        public cmd::LinearRenderCommandTranslationUnit {
    public:
        using this_type = ExecutionTranslationUnit;

        class Resolver;

    protected:
        constexpr ExecutionTranslationUnit(mref<uptr<Resolver>> resolver_) noexcept :
            LinearRenderCommandTranslationUnit(),
            _resolver(_STD move(resolver_)) {}

    public:
        constexpr ~ExecutionTranslationUnit() noexcept override = default;

    public:
        class __declspec(novtable) Resolver {
        public:
            using this_type = Resolver;

            using compiled_result = ::hg::engine::gfx::acc::EffectCompileResult;

        public:
            virtual nmpt<SymbolizedResource> resolveSymbol(
                mref<smr<const acc::Symbol>> symbol_
            ) const noexcept = 0;

            virtual nmpt<compiled_result> resolveCompiledMaterial(
                const ptr<const material::Material> material_
            ) const noexcept = 0;

            virtual nmpt<compiled_result> resolvedCompiledEffect(
                mref<smr<const acc::AccelerationEffect>> effect_
            ) const noexcept = 0;
        };

    protected:
        uptr<Resolver> _resolver;
    };
}
