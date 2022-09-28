#include "StaticGeometryBatch.hpp"

using namespace ember::engine::gfx;
using namespace ember;

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
