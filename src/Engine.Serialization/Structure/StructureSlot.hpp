#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>

#include "SlotOpResult.hpp"
#include "StructurePath.hpp"
#include "StructureSlotState.hpp"
#include "StructureSlotType.hpp"
#include "__fwd.hpp"

namespace hg::engine::serialization {
	class macro_novtable StructureSlotBase :
		public std::enable_shared_from_this<StructureSlotBase> {
	public:
		using this_type = StructureSlotBase;

	protected:
		explicit StructureSlotBase(cref<StructureSlotState> state_) noexcept :
			_state(state_) {}

		explicit StructureSlotBase(mref<StructureSlotState> state_) noexcept :
			_state(std::move(state_)) {}

	public:
		virtual ~StructureSlotBase() = default;

	protected:
		StructureSlotState _state;

	public:
		[[nodiscard]] ref<StructureSlotState> getState() noexcept {
			return _state;
		}

		[[nodiscard]] cref<StructureSlotHeader> getSlotHeader() const noexcept;

		[[nodiscard]] ref<StructureSlotHeader> getSlotHeader() noexcept;

		[[nodiscard]] virtual StructureSlotType getSlotType() const noexcept;

	public:
		[[nodiscard]] virtual bool validateType() const noexcept = 0;

	public:
		void writeHeader();

		void readHeader();

	public:
		virtual void enter();

		void leave();

		virtual void feedback(const non_owning_rptr<const StructureSlotBase> other_);
	};

	template <typename ValueType_>
	class macro_novtable TypedStructureSlotBase :
		public StructureSlotBase {
	public:
		using this_type = TypedStructureSlotBase<ValueType_>;
		using value_type = std::decay_t<ValueType_>;

	protected:
		explicit TypedStructureSlotBase(cref<StructureSlotState> state_) :
			StructureSlotBase(state_) {}

		explicit TypedStructureSlotBase(mref<StructureSlotState> state_) :
			StructureSlotBase(std::move(state_)) {}

	public:
		virtual void operator<<(cref<value_type> value_) = 0;

		virtual SlotOpResult operator>>(ref<value_type> value_) = 0;
	};
}
