#include "pch.h"

/**/

#include <Heliogrim/Actor/Archetype/ArchetypeDefinition.inl.hpp>

#include "TestArchetypes.hpp"
#include "TestArchetypes.inl.hpp"

using namespace HeliogrimApi;

TestBody::TestBody(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

void TestBody::touchCamera() {
	++get<"camera">().ticks;
}

TestTank::TestTank(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

TestTicker::TestTicker(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

void TestTicker::tick() {
	++get<"camera">().ticks;
}

TestTickerChild::TestTickerChild(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

TestOverridable::TestOverridable(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

TestOverrider::TestOverrider(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

// Compile checks (spec 13): deducing this with explicit instantiation, next to InheritArchetype's.
template void TestOverridable::tick(this TestOverridable& self);

template void TestOverrider::tick(this TestOverrider& self);

template class hg::InheritArchetype<TestBody, Actor>;
template class hg::InheritArchetype<TestTank, TestBody>;
template class hg::InheritArchetype<TestTicker, Actor>;
template class hg::InheritArchetype<TestTickerChild, TestTicker>;
template class hg::InheritArchetype<TestOverridable, Actor>;
template class hg::InheritArchetype<TestOverrider, TestOverridable>;
