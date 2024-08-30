#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "DataLayoutBase.hpp"
#include "LayoutDefine.hpp"
#include "LayoutDefineObject.hpp"
#include "LayoutDefineSlice.hpp"
#include "LayoutDefineSpan.hpp"
#include "LayoutDefineValue.hpp"

namespace hg::engine::serialization {
	template <typename SerializationType_>
	class DataLayout final :
		public DataLayoutBase {
	public:
		friend class ::hg::engine::serialization::layout::LayoutDefine;

	public:
		using this_type = DataLayout<SerializationType_>;
		using underlying_type = DataLayoutBase;

		using serialization_type = SerializationType_;

		inline static constexpr bool has_reflected_class = ClassHasMeta<serialization_type>;

	public:
		DataLayout() noexcept :
			DataLayoutBase(has_reflected_class) {}

		~DataLayout() override = default;

	protected:
		/**
		 * Will generate a layout definition to store and load data to and from a value.
		 *
		 * @example Let's assume we have a trivial type like `std::size_t` which typifies a value (e.g. a counter).
		 *  As we may not need some kind of nested or special behaviour to store and load from the given value at
		 *  `offset_`, we can forward the invocation to some kind of trivial memory operation, thus defining a value
		 *  definition will result in some kind of lightweight effect.
		 *
		 * @remark The consumed or generated layout is not guaranteed to be invoked, as some optimizations may reorder,
		 *	embed or inline operations.
		 *
		 * @tparam ValueType_ The value type to consider in-place at the given `offset_`.
		 * @param offset_ The byte offset into the object where to locate the value.
		 */
		template <layout::LayoutDefineValueType ValueType_>
		void defineValue(const u64 offset_) {
			define(make_ptr<layout::LayoutDefineValue<ValueType_>>(offset_));
		}

		/**
		 * Will generate a layout definition to store and load data to and from the given object.
		 *
		 * @example Let's assume we have a composite class `T` with a member struct or class of type `U`.
		 *	The generated layout will embed the data-layout derived from the member type `U` at offset `offset_`
		 *	into the data-layout derived from the owner type `T`. Therefore a load or store operation, will implicitly
		 *	call into the layout support functions for the corresponding effect.
		 *
		 * @note While it is possible to forward a data layout of a primitive or trivial type into this function,
		 *	you should consider the usage of `defineValue<...>` to target those inlined types.
		 *
		 * @remark The consumed or generated layout is not guaranteed to be invoked, as some optimizations may reorder,
		 *	embed or inline operations.
		 *
		 * @param offset_ The byte offset into the parent where the object is located.
		 * @param layout_ The serialization data layout of the object type to handle.
		 */
		void defineObject(const u64 offset_, cref<sptr<DataLayoutBase>> layout_) {
			define(make_ptr<layout::LayoutDefineObject>(offset_, layout_));
		}

		/**
		 * Will generate a layout definition to store and load data to and from the given container.
		 *
		 * @example Let's assume `T` is the type managed by a generic container `C<...>`.
		 *	A possible instance `obj` within the target could have a complete type definition of `C<T, alloc<T>>`.
		 *	Therefore the generated load and store operations will scale obj to hold N elements of T and
		 *	conditionally store or load the instances of type T in-place or move-construct them.
		 *
		 * @remark The consumed or generated layout is not guaranteed to be invoked, as some optimizations may reorder,
		 *	embed or inline operations.
		 *
		 * @tparam ContainerType_ The container type to use in-place at the given offset.
		 * @param offset_ The byte offset into the object where to work.
		 * @param layout_ The layout of the value-type, managed within the container of `ContainerType_`.
		 */
		template <typename ContainerType_>
		void defineSlice(const u64 offset_, cref<sptr<DataLayoutBase>> layout_) {
			define(&make_ptr<layout::LayoutDefineSlice>(offset_, layout_)->withType<ContainerType_>());
		}

		/**
		 * Will generate a layout definition to store and load data to and from a contiguous sequence of elements.
		 *
		 * @example Let's assume we want to store and load a three-dimensional vector build with floats for each dimension.
		 *  We may define a `struct v3d { float data[3]; };` providing space for each floating-point value while
		 *  spanning a contiguous memory region of `sizeof(float)... * 3`, thus we can access all the floats at ones
		 *  by using the address of the first element and the upper bound of `3` as a count.
		 *
		 * @note If you meant to handle a set of data, which is not explicitly inline of the current object layout,
		 *	you should use the `defineSlice<...>` function.
		 *
		 * @remark The consumed or generated layout is not guaranteed to be invoked, as some optimizations may reorder,
		 *	embed or inline operations.
		 *
		 * @param offset_ The byte offset into the object where to work.
		 * @param layout_ The layout of the value type to be expected at the given relative position.
		 * @param count_ The contiguous amount of elements described by `layout_` to be considered at `offset_`.
		 */
		void defineSpan(const u64 offset_, cref<sptr<DataLayoutBase>> layout_, const u64 count_) {
			define(make_ptr<layout::LayoutDefineSpan>(offset_, layout_, count_));
		}

	public:
		void describe() override;
	};
}
