#pragma once

#include <concepts>

namespace ember::engine::serialization {
    template <typename>
    class StructureSlot;
    class StructureSlotBase;

    template <typename>
    class TypeScopedSlot;
    class ScopedSlot;

    class StructurePath;

    /**/

    class StringScopedSlot;

    template <_STD integral IntegralType_>
    class IntegralScopedSlot;

    template <typename ValueType_, template <typename...> typename SliceType_>
    class SliceScopedSlot;

    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
    class MapScopedSlot;

    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename PackType_>
    class MapEntryScopedSlot;

    /**/

    class RecordSlot;
    class RecordScopedSlot;
    class StructScopedSlot;
    class SeqScopedSlot;
}
