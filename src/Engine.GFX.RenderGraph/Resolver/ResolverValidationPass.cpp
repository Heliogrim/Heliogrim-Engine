#include "ResolverValidationPass.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

#include "ResolverValidationVisitor.hpp"
#include "Engine.GFX.RenderGraph/RenderGraph.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render;
using namespace hg;

ResolverValidationPass::ResolverValidationPass() noexcept :
    ResolverPass(ResolverPassFlagBits::eValidate) {}

ResolverValidationPass::~ResolverValidationPass() = default;

uptr<RenderGraph> ResolverValidationPass::operator()(mref<uptr<RenderGraph>> graph_) const {

    if (not graph_->begin().empty()) {
        ResolverValidationVisitor visitor {};
        graph_->begin()->accept(visitor);
    }

    return graph_;
}
