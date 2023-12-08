#include "RenderGraphCompiler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>

#include "CompileVisitor.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

uptr<RuntimeGraph> RenderGraphCompiler::operator()(mref<CompileRequest> request_) const {

    if (request_.targetSymbols.empty()) {
        return nullptr;
    }

    for (const auto& target : request_.targetSymbols) {

        bool exists = false;
        for (const auto& provision : request_.graph->expectedProvision()) {

            if (
                provision.symbol->scope == target->scope &&
                provision.symbol->name == target->name
            ) {
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
    auto visitor = graph::CompileVisitor(runtime.get());

    request_.graph->reverse(visitor);

    return runtime;
}
