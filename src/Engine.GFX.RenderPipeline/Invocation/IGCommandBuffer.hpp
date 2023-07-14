#pragma once

namespace hg::engine::gfx::render::pipeline {
    class IGCommandBuffer {
    public:
        void draw();
        void drawIndexed();

        /**/

        void bindMaterial();
        void bindMesh();
        void bindInstances();

        /**/

        void bindVertexBuffer();
        void bindIndexBuffer();
        void bindTexture();
        void bindStorage();
    };
}
