#include "StaticGeometryBatch.hpp"

using namespace ember::engine::gfx;
using namespace ember;

StaticGeometryBatch::~StaticGeometryBatch() {
    if (cdb) {
        delete cdb;
        cdb = nullptr;
    }

    if (instance.buffer || instance.memory) {
        instance.destroy();
    }
}

ptr<cache::ModelBatch> StaticGeometryBatch::clone() {
    throw NotImplementedException();
}
