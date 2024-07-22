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
		constexpr Stage() noexcept = default;

		constexpr explicit Stage(mref<StageFlagBits> flagBits_) noexcept :
			_flagBits(std::move(flagBits_)),
			_intermediate() {}

		virtual ~Stage() noexcept = default;

	protected:
		StageFlagBits _flagBits;

	public:
		[[nodiscard]] constexpr StageFlagBits getFlagBits() const noexcept {
			return _flagBits;
		}

	public:
		void enumerateStageInputs(_Out_ ref<Vector<StageInput>> inputs_) const noexcept;

		void enumerateStageOutputs(_Out_ ref<Vector<StageOutput>> outputs_) const noexcept;

	protected:
		smr<lang::Intermediate> _intermediate;

	public:
		[[nodiscard]] smr<lang::Intermediate> getIntermediate() const noexcept {
			return _intermediate;
		}

		void setIntermediate(mref<smr<lang::Intermediate>> intermediate_) {
			_intermediate = std::move(intermediate_);
		}
	};
}
