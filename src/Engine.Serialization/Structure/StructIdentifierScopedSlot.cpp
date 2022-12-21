#include "StructIdentifierScopedSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

StructIdentifierScopedSlot::StructIdentifierScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {}

StructIdentifierScopedSlot::StructIdentifierScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {}

StructIdentifierScopedSlot::~StructIdentifierScopedSlot() = default;

StructureSlotType StructIdentifierScopedSlot::getSlotType() const noexcept {
    return StructureSlotType::eUndefined;
}

void StructIdentifierScopedSlot::enter(const bool mutating_) {}

void StructIdentifierScopedSlot::leave(const bool mutating_) {}

const StringScopedSlot StructIdentifierScopedSlot::intoString() const {
    return StringScopedSlot { _STD move(_state) };
}

StringScopedSlot StructIdentifierScopedSlot::intoString() {
    return StringScopedSlot { _STD move(_state) };
}

const IntegralScopedSlot<u64> StructIdentifierScopedSlot::intoU64() const {
    return IntegralScopedSlot<u64> { _STD move(_state) };
}

IntegralScopedSlot<u64> StructIdentifierScopedSlot::intoU64() {
    return IntegralScopedSlot<u64> { _STD move(_state) };
}

void StructIdentifierScopedSlot::operator<<(cref<string> identifier_) {
    (this->intoString()) << identifier_;
}

void StructIdentifierScopedSlot::operator<<(cref<u64> identifier_) {
    throw NotImplementedException();
    //(this->intoU64()) << identifier_;
}

void StructIdentifierScopedSlot::operator>>(ref<string> identifier_) {
    // Enforce const behaviour to prevent dirty writes or unnecessary reports 
    const auto* const self = this;
    self->intoString() >> identifier_;
}

void StructIdentifierScopedSlot::operator>>(ref<u64> identifier_) {
    throw NotImplementedException();
    //(this->intoU64()) >> identifier_;
}
