#include "WoodenTable.hpp"

#include <Heliogrim/StaticGeometryComponent.hpp>

#include "Heliogrim/ActorInitializer.hpp"

using namespace hg::game::actors;
using namespace hg;

WoodenTable::WoodenTable(cref<ActorInitializer> initializer_) :
    Actor(initializer_) {

    initializer_.createComponent<ActorComponent>(this);
    initializer_.createSubComponent<StaticGeometryComponent>(this, _rootComponent);
}
