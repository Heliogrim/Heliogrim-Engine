#include "StaticGeometryBatch.hpp"

using namespace hg::engine::gfx;
using namespace hg;

StaticGeometryBatch::~StaticGeometryBatch() {
    if (cdb) {
        delete cdb;
        cdb = nullptr;
    }

    if (mttCdb) {
        delete mttCdb;
        mttCdb = nullptr;
    }

    if (instance.buffer || instance.memory) {
        instance.destroy();
    }

    if (mtt.buffer || mtt.memory) {
        mtt.destroy();
    }
}

ptr<cache::ModelBatch> StaticGeometryBatch::clone() {
    throw NotImplementedException();
}
