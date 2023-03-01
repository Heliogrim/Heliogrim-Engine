#include "EdPostCompStage.hpp"

#include "EdGridNode.hpp"

using namespace hg::editor::gfx;
using namespace hg::engine::gfx::render;
using namespace hg;

void EdPostCompStage::setup(cref<sptr<engine::gfx::Device>> device_) {
    RenderStage::setup(device_);

    /**/
    multiplexer().push(new EdGridNode());
}

void EdPostCompStage::destroy() {
    RenderStage::destroy();
}
