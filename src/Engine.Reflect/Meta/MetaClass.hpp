#pragma once

#include <span>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "../Template/QueryTypeId.hpp"

namespace hg {
	class MetaClass {
	public:
		using this_type = MetaClass;

	protected:
		consteval MetaClass(const type_id typeId_) noexcept :
			_typeId(typeId_) {}

	public:
		constexpr virtual ~MetaClass() noexcept = default;

	protected:
		#pragma region Meta Information
		type_id _typeId;

	public:
		[[nodiscard]] constexpr cref<type_id> typeId() const noexcept {
			return _typeId;
		}

		[[nodiscard]] constexpr virtual bool inherits(const ptr<const MetaClass> type_) const noexcept = 0;

		[[nodiscard]] constexpr virtual bool inherits(const type_id typeId_) const noexcept = 0;

	public:
		[[nodiscard]] constexpr bool exact(const type_id typeId_) const noexcept {
			return _typeId == typeId_;
		}

		template <::hg::refl::TypeQueryable Check_>
		[[nodiscard]] constexpr bool exact() const noexcept {
			constexpr typename ::hg::refl::query_type_id<Check_>::result query {};
			return exact(query());
		}

		[[nodiscard]] constexpr bool is(const type_id typeId_) const noexcept {
			return exact(typeId_) || inherits(typeId_);
		}

		template <typename Check_>
		[[nodiscard]] constexpr bool is() const noexcept {
			constexpr typename ::hg::refl::query_type_id<Check_>::result query {};
			return is(query());
		}

		#pragma endregion

	protected:
		#pragma region Invocation Interface
		// TODO: Capture invocation interface
		#pragma endregion
	};
}
