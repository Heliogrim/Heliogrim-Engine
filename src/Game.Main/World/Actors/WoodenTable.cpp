#include "WoodenTable.hpp"

#include <Ember/StaticGeometryComponent.hpp>

#include "Ember/ActorInitializer.hpp"

using namespace ember::game::actors;
using namespace ember;

WoodenTable::WoodenTable() :
    Actor() {}

WoodenTable::WoodenTable(cref<ActorInitializer> initializer_) :
    Actor() {

    initializer_.createComponent<ActorComponent>(this);
    initializer_.createSubComponent<StaticGeometryComponent>(this, _rootComponent);
}
