#include "CompileBinNode.hpp"

#include "../Runtime/BinNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

[[nodiscard]] uptr<RuntimeNode> CompileBinNode::compile(cref<CompilePassContext> ctx_) const noexcept {
    return make_uptr<BinNode>();
}
