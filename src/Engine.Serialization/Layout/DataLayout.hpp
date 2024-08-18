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
		template <layout::LayoutDefineValueType ValueType_>
		void defineValue(const u64 offset_) {
			define(make_ptr<layout::LayoutDefineValue<ValueType_>>(offset_));
		}

		void defineObject(const u64 offset_, cref<sptr<DataLayoutBase>> layout_) {
			define(make_ptr<layout::LayoutDefineObject>(offset_, layout_));
		}

		template <typename ContainerType_>
		void defineSlice(const u64 offset_, cref<sptr<DataLayoutBase>> layout_) {
			define(&make_ptr<layout::LayoutDefineSlice>(offset_, layout_)->withType<ContainerType_>());
		}

		void defineSpan(const u64 offset_, cref<sptr<DataLayoutBase>> layout_, const u64 count_) {
			define(make_ptr<layout::LayoutDefineSpan>(offset_, layout_, count_));
		}

	public:
		void describe() override;
	};
}
