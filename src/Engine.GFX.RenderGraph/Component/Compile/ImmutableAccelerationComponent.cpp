#include "ImmutableAccelerationComponent.hpp"

#include "../Runtime/ImmutableAccelerationComponent.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg;

uptr<AccelerationComponent> CompileImmutableAccelerationComponent::
compile(cref<CompilePassContext> ctx_) const noexcept {
    return ctx_.getGraphNodeAllocator()->allocate<ImmutableAccelerationComponent>(
        clone(_passOutputSymbols),
        clone(_passInputSymbols)
    );
}
