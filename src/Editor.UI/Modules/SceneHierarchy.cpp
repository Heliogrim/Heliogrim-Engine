#include "SceneHierarchy.hpp"

#include "../Panel/SceneHierarchyPanel.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

SceneHierarchy::SceneHierarchy() :
    _panels(),
    _resolver(),
    _generator() {}

SceneHierarchy::~SceneHierarchy() = default;

sptr<SceneHierarchyPanel> SceneHierarchy::makePanel() {
    auto panel { SceneHierarchyPanel::make(this) };
    _panels.push_back(panel);
    return panel;
}

bool SceneHierarchy::storeResolver(const type_id typeId_, mref<uptr<HierarchyResolverBase>> resolver_) {

    if (_resolver.find(typeId_) != _resolver.end()) {
        return false;
    }

    _resolver.insert_or_assign(typeId_, _STD move(resolver_));
    return true;
}

ptr<HierarchyResolverBase> SceneHierarchy::getResolver(const type_id typeId_) {

    const auto it { _resolver.find(typeId_) };
    if (it != _resolver.end()) {
        return it->second.get();
    }

    return nullptr;
}

bool SceneHierarchy::storeGenerator(const type_id typeId_, mref<uptr<HierarchyGeneratorBase>> generator_) {

    if (_generator.find(typeId_) != _generator.end()) {
        return false;
    }

    _generator.insert_or_assign(typeId_, _STD move(generator_));
    return true;

}

ptr<HierarchyGeneratorBase> SceneHierarchy::getGenerator(const type_id typeId_) {

    const auto it { _generator.find(typeId_) };
    if (it != _generator.end()) {
        return it->second.get();
    }

    return nullptr;

}
