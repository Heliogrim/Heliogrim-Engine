#include "RenderGraphCompiler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>

#include "CompileVisitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

RenderGraphCompiler::RenderGraphCompiler() = default;

RenderGraphCompiler::~RenderGraphCompiler() = default;

uptr<RuntimeGraph> RenderGraphCompiler::operator()(mref<CompileRequest> request_) {

    if (request_.targetSymbols.empty()) {
        return nullptr;
    }

    for (const auto& target : request_.targetSymbols) {

        bool exists = false;
        for (const auto& provision : request_.graph->expectedProvision()) {
            if (provision.symbol == target) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            __debugbreak();
            return nullptr;
        }
    }

    /**/

    auto runtime = make_uptr<RuntimeGraph>();
    auto visitor = CompileVisitor(runtime.get());

    request_.graph->reverse(visitor);

    return runtime;
}
