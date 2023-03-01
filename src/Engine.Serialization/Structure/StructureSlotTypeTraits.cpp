#include "StructureSlotTypeTraits.hpp"

using namespace hg::engine::serialization;
using namespace hg;

#define RESOLVE(type_, iv_) case type_: iv_ StructureSlotTypeTraits<type_>
#define RESOLVE_R(type_) RESOLVE(type_, return)

string_view StructureSlotTypeTrait::canonical(StructureSlotType type_) noexcept {
    switch (type_) {
        RESOLVE_R(StructureSlotType::eUndefined)::canonical;
        RESOLVE_R(StructureSlotType::eBool)::canonical;
        RESOLVE_R(StructureSlotType::eU8)::canonical;
        RESOLVE_R(StructureSlotType::eU16)::canonical;
        RESOLVE_R(StructureSlotType::eU32)::canonical;
        RESOLVE_R(StructureSlotType::eU64)::canonical;
        RESOLVE_R(StructureSlotType::eS8)::canonical;
        RESOLVE_R(StructureSlotType::eS16)::canonical;
        RESOLVE_R(StructureSlotType::eS32)::canonical;
        RESOLVE_R(StructureSlotType::eS64)::canonical;
        RESOLVE_R(StructureSlotType::eFloat)::canonical;
        RESOLVE_R(StructureSlotType::eDouble)::canonical;
        RESOLVE_R(StructureSlotType::eString)::canonical;
        RESOLVE_R(StructureSlotType::eSlice)::canonical;
        RESOLVE_R(StructureSlotType::eMap)::canonical;
        RESOLVE_R(StructureSlotType::eMapEntry)::canonical;
        RESOLVE_R(StructureSlotType::eRecord)::canonical;
        RESOLVE_R(StructureSlotType::eStruct)::canonical;
        RESOLVE_R(StructureSlotType::eSeq)::canonical;
        default: return StructureSlotTypeTraits<StructureSlotType::eUndefined>::canonical;
    }
}

s64 StructureSlotTypeTrait::const_size(StructureSlotType type_) noexcept {
    switch (type_) {
        RESOLVE_R(StructureSlotType::eUndefined)::const_size;
        RESOLVE_R(StructureSlotType::eBool)::const_size;
        RESOLVE_R(StructureSlotType::eU8)::const_size;
        RESOLVE_R(StructureSlotType::eU16)::const_size;
        RESOLVE_R(StructureSlotType::eU32)::const_size;
        RESOLVE_R(StructureSlotType::eU64)::const_size;
        RESOLVE_R(StructureSlotType::eS8)::const_size;
        RESOLVE_R(StructureSlotType::eS16)::const_size;
        RESOLVE_R(StructureSlotType::eS32)::const_size;
        RESOLVE_R(StructureSlotType::eS64)::const_size;
        RESOLVE_R(StructureSlotType::eFloat)::const_size;
        RESOLVE_R(StructureSlotType::eDouble)::const_size;
        RESOLVE_R(StructureSlotType::eString)::const_size;
        RESOLVE_R(StructureSlotType::eSlice)::const_size;
        RESOLVE_R(StructureSlotType::eMap)::const_size;
        RESOLVE_R(StructureSlotType::eMapEntry)::const_size;
        RESOLVE_R(StructureSlotType::eRecord)::const_size;
        RESOLVE_R(StructureSlotType::eStruct)::const_size;
        RESOLVE_R(StructureSlotType::eSeq)::const_size;
        default: return StructureSlotTypeTraits<StructureSlotType::eUndefined>::const_size;
    }
}

#undef RESOLVE_R
#undef RESOLVE
