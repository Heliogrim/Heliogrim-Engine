#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Collection/PointerKey.hpp>
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Acc.Compile/EffectCompileResult.hpp>
#include <Engine.GFX.Acc.Compile/Profile/EffectProfile.hpp>
#include <Engine.GFX.Acc.Compile/Spec/EffectSpecification.hpp>
#include <Engine.GFX.Render.Command/RenderCommandTranslationUnit.hpp>
#include <tl/optional.hpp>
#include <Engine.GFX.Material/Material.hpp>

namespace hg::engine::gfx::render::graph {
    class EffectCompileTranslationUnit :
        public cmd::LinearRenderCommandTranslationUnit {
    public:
        using this_type = EffectCompileTranslationUnit;

        class Resolver;
        class Aggregator;

    public:
        EffectCompileTranslationUnit(mref<nmpt<Resolver>> resolver_, mref<nmpt<Aggregator>> aggregator_);

        constexpr ~EffectCompileTranslationUnit() noexcept override = default;

    public:
        class __declspec(novtable) Resolver {
        public:
            using this_type = Resolver;

        public:
            constexpr virtual ~Resolver() noexcept = default;

        public:
            [[nodiscard]] virtual cref<acc::EffectSpecification> resolveSpecification() const noexcept = 0;

            [[nodiscard]] virtual smr<const acc::EffectProfile> resolveProfile() const noexcept = 0;
        };

    public:
        class __declspec(novtable) Aggregator {
        public:
            using this_type = Aggregator;

            using material_identifier = Guid;
            using effect_identifier = PointerKey<engine::gfx::acc::AccelerationEffect>;

            using compiled_type = uptr<engine::gfx::acc::EffectCompileResult>;
            using packed_compiled_type = Vector<compiled_type>;

            template <typename Type_>
            //using expected = concurrent::future<Type_>;
            using expected = _STD type_identity_t<Type_>;

        public:
            constexpr virtual ~Aggregator() noexcept = default;

        public:
            [[nodiscard]] virtual bool containsEffect(effect_identifier identifier_) const noexcept = 0;

            [[nodiscard]] virtual bool containsMaterial(material_identifier identifier_) const noexcept = 0;

            [[nodiscard]] virtual tl::optional<cref<compiled_type>> queryEffect(
                effect_identifier identifier_
            ) const noexcept = 0;

            [[nodiscard]] virtual tl::optional<cref<expected<compiled_type>>> queryExpectedEffect(
                effect_identifier identifier_
            ) const noexcept = 0;

            [[nodiscard]] virtual tl::optional<cref<packed_compiled_type>> queryMaterial(
                material_identifier identifier_
            ) const noexcept = 0;

            [[nodiscard]] virtual tl::optional<cref<expected<packed_compiled_type>>> queryExpectedMaterial(
                material_identifier identifier_
            ) const noexcept = 0;

        public:
            virtual void store(
                effect_identifier identifier_,
                mref<expected<compiled_type>> request_
            ) noexcept = 0;

            virtual void store(
                material_identifier identifier_,
                mref<expected<packed_compiled_type>> request_
            ) noexcept = 0;
        };

    protected:
        nmpt<Resolver> _resolver;
        nmpt<Aggregator> _aggregator;

    private:
        [[nodiscard]] Aggregator::expected<Aggregator::compiled_type> request(
            mref<smr<const engine::gfx::acc::AccelerationEffect>> effect_
        ) const noexcept;

        [[nodiscard]] Aggregator::expected<Aggregator::packed_compiled_type> request(
            mref<const nmpt<const material::Material>> material_
        ) const noexcept;

    public:
        void translate(ptr<const cmd::BindMaterialRenderCommand>) noexcept override;
    };
}
