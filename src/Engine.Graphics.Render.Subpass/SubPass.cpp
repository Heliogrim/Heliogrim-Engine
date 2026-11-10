#include "SubPass.hpp"

using namespace hg::engine::render;
using namespace hg;

void SubPass::setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    declareOutputs(symCtx_);
    declareTransforms(symCtx_);
    declareInputs(symCtx_);
}

void SubPass::destroy() noexcept {}

void SubPass::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {}

void SubPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {}

void SubPass::declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {}
