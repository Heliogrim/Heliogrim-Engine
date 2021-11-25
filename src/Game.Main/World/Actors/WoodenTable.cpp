#include "WoodenTable.hpp"

#include <Ember/StaticMeshComponent.hpp>

using namespace ember::game::actors;
using namespace ember;

WoodenTable::WoodenTable(const ptr<ActorInitializer> initializer_) :
    Actor(initializer_) {

    auto* rootComp = initializer_->createComponent<ActorComponent>(this);
    auto* meshComp = initializer_->createSubComponent<StaticMeshComponent>(rootComp);
}
