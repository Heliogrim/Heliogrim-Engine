#include "WoodenTable.hpp"

#include <Ember/StaticMeshComponent.hpp>

#include "Ember/ActorInitializer.hpp"

using namespace ember::game::actors;
using namespace ember;

WoodenTable::WoodenTable() :
    Actor() {

    auto& initializer { ActorInitializer::get() };

    initializer.createComponent<ActorComponent>(this);
    initializer.createSubComponent<StaticMeshComponent>(this, _rootComponent);
}
