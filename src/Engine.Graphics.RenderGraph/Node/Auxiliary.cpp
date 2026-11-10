#include "Auxiliary.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

Auxiliary::~Auxiliary() {
    tidy();
}

void Auxiliary::tidy() {
    for (auto&& entry : _comps) {
        delete entry;
    }
    _comps.clear();
}

cref<CompactSet<ptr<Component>>> Auxiliary::components() const noexcept {
    return _comps;
}

void Auxiliary::add(mref<uptr<Component>> component_) noexcept {
    _comps.insert(component_.release());
}

void Auxiliary::remove(mref<ptr<Component>> component_) noexcept {
    _comps.erase(component_);
    delete component_;
}
