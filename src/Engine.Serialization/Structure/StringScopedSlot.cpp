#include "StringScopedSlot.hpp"

#include <span>

#include "../Archive/Archive.hpp"
#include "StructureSlotState.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#include "StructureSlotTypeTraits.hpp"
#endif

using namespace ember::engine::serialization;
using namespace ember;

StringScopedSlot::StringScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlot(state_) {}

StringScopedSlot::StringScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlot(_STD move(state_)) {}

StringScopedSlot::~StringScopedSlot() = default;

void StringScopedSlot::operator<<(cref<data_type> object_) {

    auto* archive = _state.rootState->getArchive();

    const auto src = _STD span<u8, _STD dynamic_extent>(
        reinterpret_cast<ptr<u8>>(const_cast<ref<data_type>>(object_).data()),
        object_.size()
    );

    (*archive) << StructureSlotType::eString;
    (*archive) << static_cast<u64>(src.size());
    archive->serializeBytes(src.data(), src.size(), ArchiveStreamMode::eIn);
}

void StringScopedSlot::operator>>(ref<data_type> object_) const {

    auto* archive = _state.rootState->getArchive();

    StructureSlotType storedType { StructureSlotType::eUndefined };
    u64 storedSize { ~0ui64 };

    (*archive) >> storedType;
    (*archive) >> storedSize;

    /**/

    #ifdef _DEBUG
    if (storedType != StructureSlotType::eString) {
        IM_CORE_WARNF(
            "Tried to deserialize a `{}` into a `{}`",
            StructureSlotTypeTrait::canonical(storedType),
            StructureSlotTypeTraits<StructureSlotType::eString>::canonical
        );
    }
    #endif

    /**/

    object_.resize(storedSize);

    const auto dst = _STD span<u8, _STD dynamic_extent>(
        reinterpret_cast<ptr<u8>>(object_.data()),
        storedSize
    );

    archive->serializeBytes(dst.data(), dst.size(), ArchiveStreamMode::eOut);
}
