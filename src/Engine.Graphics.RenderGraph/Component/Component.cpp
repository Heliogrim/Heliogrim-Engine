#include "Component.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

void Component::mount(nmpt<const Node> target_) {}

void Component::mounted(nmpt<const Component> mounted_) {}

void Component::unmount(nmpt<const Node> target_) {}

void Component::unmounted(nmpt<const Component> unmounted_) {}
