#pragma once

#include <concepts>
#include <type_traits>

namespace hg::engine::serialization {
	template <typename>
	class StructureSlot;
	class StructureSlotBase;

	template <typename>
	class TypeScopedSlot;
	class ScopedSlot;

	class StructurePath;

	/**/

	template <typename Type_>
	concept IsIntegralSlotType = std::is_integral_v<Type_> ||
		(std::is_enum_v<Type_> && std::is_integral_v<std::underlying_type_t<Type_>>);

	/**/

	class StringScopedSlot;

	template <IsIntegralSlotType IntegralType_>
	class IntegralScopedSlot;

	template <std::floating_point IntegralType_>
	class FloatScopedSlot;

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
