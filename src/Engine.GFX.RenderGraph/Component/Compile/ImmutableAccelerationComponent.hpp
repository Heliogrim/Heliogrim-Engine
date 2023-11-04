#pragma once
#include "AccelerationComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileImmutableAccelerationComponent final :
        public InheritMeta<CompileImmutableAccelerationComponent, CompileAccelerationComponent> {
    public:
        using this_type = CompileImmutableAccelerationComponent;

    private:
        Vector<smr<const acc::Symbol>> _passOutputSymbols;
        Vector<smr<const acc::Symbol>> _passInputSymbols;

    protected:
        [[nodiscard]] bool validateOutputSymbol(nmpt<const acc::Symbol> symbol_) const noexcept;

        [[nodiscard]] bool validateTransformSymbol(
            nmpt<const acc::Symbol> inputSymbol_,
            nmpt<const acc::Symbol> outputSymbol_
        ) const noexcept;

        [[nodiscard]] bool validateInputSymbol(nmpt<const acc::Symbol> symbol_) const noexcept;

    public:
        void addOutput(mref<smr<const acc::Symbol>> symbol_);

        void addTransform(mref<smr<const acc::Symbol>> symbol_);

        void addInput(mref<smr<const acc::Symbol>> symbol_);

    public:
        [[nodiscard]] uptr<AccelerationComponent> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
