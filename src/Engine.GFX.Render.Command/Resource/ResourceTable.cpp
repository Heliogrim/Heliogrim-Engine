#include "ResourceTable.hpp"

using namespace hg::engine::render;
using namespace hg;

void ResourceTable::bind(cref<SymbolId> symbolId_, mref<ResourceView> view_) {
    table.emplace(symbolId_, _STD move(view_));
}
