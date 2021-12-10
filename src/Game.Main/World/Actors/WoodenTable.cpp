#include "WoodenTable.hpp"

#include <Ember/StaticGeometryComponent.hpp>

#include "Ember/ActorInitializer.hpp"

using namespace ember::game::actors;
using namespace ember;

WoodenTable::WoodenTable() :
    Actor() {

    auto& initializer { ActorInitializer::get() };

    initializer.createComponent<ActorComponent>(this);
    initializer.createSubComponent<StaticGeometryComponent>(this, _rootComponent);
}
