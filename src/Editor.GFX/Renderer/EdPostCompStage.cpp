#include "EdPostCompStage.hpp"

#include "EdGridNode.hpp"

using namespace ember::editor::gfx;
using namespace ember::engine::gfx::render;
using namespace ember;

void EdPostCompStage::setup(cref<sptr<engine::gfx::Device>> device_) {
    RenderStage::setup(device_);

    /**/
    multiplexer().push(new EdGridNode());
}

void EdPostCompStage::destroy() {
    RenderStage::destroy();
}
