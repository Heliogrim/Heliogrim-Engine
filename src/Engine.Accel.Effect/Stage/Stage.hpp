#pragma once
#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "StageFlags.hpp"
#include "StageInput.hpp"
#include "StageOutput.hpp"
#include "../__fwd.hpp"

namespace hg::engine::accel {
	class Stage {
	public:
		using this_type = Stage;

	public:
		Stage() noexcept = default;

		Stage(mref<StageFlagBits> flagBits_);

		virtual ~Stage();

	protected:
		StageFlagBits _flagBits;

	public:
		[[nodiscard]] StageFlagBits getFlagBits() const noexcept;

	public:
		void enumerateStageInputs(_Out_ ref<Vector<StageInput>> inputs_) const noexcept;

		void enumerateStageOutputs(_Out_ ref<Vector<StageOutput>> outputs_) const noexcept;

	protected:
		smr<lang::Intermediate> _intermediate;

	public:
		[[nodiscard]] smr<lang::Intermediate> getIntermediate() const noexcept;

		void setIntermediate(mref<smr<lang::Intermediate>> intermediate_);
	};
}
