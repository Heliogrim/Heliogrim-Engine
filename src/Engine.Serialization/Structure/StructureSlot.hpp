#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>

#include "SlotOpResult.hpp"
#include "StructurePath.hpp"
#include "StructureSlotState.hpp"
#include "StructureSlotType.hpp"
#include "__fwd.hpp"
#include "../Load/ReadError.hpp"

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
		virtual ~StructureSlotBase();

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

		/* Bounded by the parent payload; false (header reset, error reported) on any violation. */
		bool readHeader();

	public:
		/* Reports to the load context of the root, if any. */
		void fail(ReadError::Kind kind_, s64 offset_, s64 length_, ptr<const char> detail_) const;

		/* Warns through the load context of the root, if any. */
		void warn(ReadError::Kind kind_, s64 offset_, s64 length_, String detail_) const;

		[[nodiscard]] bool hasPayload() const noexcept;

		[[nodiscard]] s64 payloadBegin() const noexcept;

		[[nodiscard]] s64 payloadEnd() const noexcept;

		/* Bytes a slot with this header spans: tag, size field if any, payload. */
		[[nodiscard]] static s64 slotSpan(ref<const StructureSlotHeader> header_) noexcept;

	protected:
		[[nodiscard]] s64 slotLimit() const noexcept;

		[[nodiscard]] bool readHeaderAt(s64 at_, s64 limit_, _Out_ ref<StructureSlotHeader> out_) const;

		[[nodiscard]] bool checkArchive(s64 offset_, s64 length_) const;

		[[nodiscard]] bool charge(u64 bytes_) const;

		/* Reads the stored integral (lossless widening only) as raw bits of the target type. */
		[[nodiscard]] bool readIntegral(StructureSlotType target_, _Out_ ref<u64> raw_);

		[[nodiscard]] bool readFloat(StructureSlotType target_, _Out_ ref<double> value_);

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
