#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::engine::reflow {
	class ReflowClassList final :
		protected Vector<Variant<StringView, String>> {
	public:
		using this_type = ReflowClassList;
		using value_type = Variant<StringView, String>;

	public:
		constexpr ReflowClassList() noexcept = default;

		explicit ReflowClassList(mref<value_type> initialValue_) noexcept :
			Vector({ ::hg::move(initialValue_) }) {}

		constexpr ReflowClassList(cref<ReflowClassList> other_) noexcept = default;

		constexpr ReflowClassList(mref<ReflowClassList> other_) noexcept = default;

		~ReflowClassList() = default;

	public:
		ref<this_type> operator=(cref<this_type>) noexcept = default;

		ref<this_type> operator=(mref<this_type>) noexcept = default;

	public:
		void add(StringView className_);

		void drop(StringView className_);

	public:
		using Vector::begin;
		using Vector::cbegin;
		using Vector::end;
		using Vector::cend;
	};
}
