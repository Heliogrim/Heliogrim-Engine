#pragma once

#include "ScopedSlotState.hpp"
#include "StructureSlot.hpp"

namespace hg::engine::serialization {
	class macro_novtable ScopedSlot {
	public:
		using this_type = ScopedSlot;

		using slot_type = StructureSlotBase;

	protected:
		ScopedSlot(
			mref<ScopedSlotState> state_,
			mref<sptr<StructureSlotBase>> slot_
		);

		ScopedSlot(cref<this_type>) = default;

		ScopedSlot(mref<this_type>) noexcept = default;

	public:
		virtual ~ScopedSlot() = default;

	protected:
		mutable ScopedSlotState _state;

	public:
		[[nodiscard]] cref<ScopedSlotState> getScopedState() const noexcept;

		[[nodiscard]] ref<ScopedSlotState> getScopedState() noexcept;

	protected:
		mutable sptr<StructureSlotBase> _slot;

	public:
		// ReSharper disable once CppHiddenFunction
		[[nodiscard]] const non_owning_rptr<StructureSlotBase> slot() const noexcept;

	public:
		void enterSlot() const;

		void leaveSlot() const;
	};

	template <typename ValueType_>
	class macro_novtable TypeScopedSlot :
		public ScopedSlot {
	public:
		using this_type = TypeScopedSlot<ValueType_>;

		using slot_type = TypedStructureSlotBase<ValueType_>;
		using value_type = typename slot_type::value_type;

	protected:
		TypeScopedSlot(
			mref<ScopedSlotState> state_,
			mref<sptr<StructureSlotBase>> slot_
		) :
			ScopedSlot(std::move(state_), std::move(slot_)) {}

	public:
		~TypeScopedSlot() override = default;

	public:
		[[nodiscard]] const non_owning_rptr<slot_type> slot() const noexcept {
			return static_cast<const non_owning_rptr<slot_type>>(ScopedSlot::slot());
		}

	public:
		virtual void operator<<(cref<value_type> value_) = 0;

		virtual void operator>>(ref<value_type> value_) const = 0;
	};
}
