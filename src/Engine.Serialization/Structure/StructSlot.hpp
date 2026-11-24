#pragma once

#include "__fwd.hpp"
#include "StructureSlot.hpp"
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	class StructSlot final :
		public StructureSlotBase {
	public:
		using this_type = StructSlot;

		using record_key_type = StringView;

	public:
		explicit StructSlot(cref<StructureSlotState> state_) noexcept;

		explicit StructSlot(mref<StructureSlotState> state_) noexcept;

		~StructSlot() override;

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override;

		[[nodiscard]] bool validateType() const noexcept override;

	public:
		void feedback(non_owning_rptr<const StructureSlotBase> other_) override;

	private:
		[[nodiscard]] s64 findRecord(cref<record_key_type> key_) const;

	public:
		[[nodiscard]] sptr<RecordSlot> insertRecord(cref<record_key_type> key_);

		[[nodiscard]] bool hasRecord(cref<record_key_type> key_);

		[[nodiscard]] sptr<RecordSlot> getRecord(cref<record_key_type> key_);

	public:
		/* Stored keys in order; empty on a malformed payload (failure latched). */
		[[nodiscard]] Vector<String> keys() const;

		void warn(ReadError::Kind kind_, String detail_) const;

		/* Warns `unknown-field` per stored key no alias of this struct read (`@type`, `@rev` aside); true if any. */
		[[nodiscard]] bool reportUnreadKeys() const;
	};
}
