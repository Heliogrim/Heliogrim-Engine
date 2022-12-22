#include "StringScopedSlot.hpp"

#include <span>

#include "../Archive/Archive.hpp"
#include "StructureSlotState.hpp"
#include "ScopedSlotGuard.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#include "StructureSlotTypeTraits.hpp"
#endif

using namespace ember::engine::serialization;
using namespace ember;

StringScopedSlot::StringScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlot(state_) {
    ensureEntered(not _state.isScopedImmutable());
}

StringScopedSlot::StringScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlot(_STD move(state_)) {
    ensureEntered(not _state.isScopedImmutable());
}

StringScopedSlot::~StringScopedSlot() = default;

void StringScopedSlot::enter(const bool mutating_) {

    ScopedStructureSlot<string>::enter(mutating_);

    /**/

    if (not mutating_) {

        auto* archive = _state.rootState->getArchive();
        archive->seek(_state.offset);

        StructureSlotType storedType = StructureSlotType::eUndefined;
        (*archive) >> storedType;

        #ifdef _DEBUG
        if (storedType != StructureSlotType::eString) {
            IM_CORE_WARNF(
                "Tried to deserialize a `{}` into a `{}`",
                StructureSlotTypeTrait::canonical(storedType),
                StructureSlotTypeTraits<StructureSlotType::eString>::canonical
            );
        }
        #endif

        (*archive) >> _state.size;
        return;
    }

    /**/

    if (mutating_ && not _state.size) {
        auto* archive = _state.rootState->getArchive();
        (*archive) << StructureSlotType::eString;
        (*archive) << 0ui64;
    }
}

void StringScopedSlot::leave(const bool mutating_) {

    ScopedStructureSlot<string>::leave(mutating_);

    /**/

    if (not mutating_) {
        return;
    }

    /**/

    auto* archive = _state.rootState->getArchive();
    archive->seek(_state.offset);

    (*archive) << StructureSlotType::eString;
    (*archive) << (_state.size - sizeof(StructureSlotType::eString) - sizeof(u64));

    archive->seek(_state.offset + _state.size);
}

void StringScopedSlot::operator<<(cref<data_type> object_) {

    const ScopedSlotGuard guard { this, true };
    auto* archive = _state.rootState->getArchive();

    const auto src = _STD span<u8, _STD dynamic_extent>(
        reinterpret_cast<ptr<u8>>(const_cast<ref<data_type>>(object_).data()),
        object_.size()
    );

    /*
    (*archive) << StructureSlotType::eString;
    (*archive) << static_cast<u64>(src.size());
     */
    archive->serializeBytes(src.data(), src.size(), ArchiveStreamMode::eIn);
}

void StringScopedSlot::operator>>(ref<data_type> object_) const {

    const ScopedSlotGuard guard { this, false };

    /**/

    auto* archive = _state.rootState->getArchive();
    object_.resize(_state.size);

    const auto dst = _STD span<u8, _STD dynamic_extent>(
        reinterpret_cast<ptr<u8>>(object_.data()),
        _state.size
    );

    archive->serializeBytes(dst.data(), dst.size(), ArchiveStreamMode::eOut);
}
