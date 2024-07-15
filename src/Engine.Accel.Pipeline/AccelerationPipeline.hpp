#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "__fwd.hpp"
#include "Bind/BindLayout.hpp"

namespace hg::engine::accel {
	class AccelerationPipeline :
		public InheritBase<AccelerationPipeline> {
	public:
		using this_type = AccelerationPipeline;

	protected:
		AccelerationPipeline(mref<smr<const AccelerationEffect>> effect_) noexcept;

	public:
		AccelerationPipeline(mref<this_type> other_) noexcept = delete;

		AccelerationPipeline(cref<this_type> other_) = delete;

		~AccelerationPipeline() override;

	protected:
		smr<const AccelerationEffect> _effect;
		Vector<smr<StageDerivat>> _stages;

	public:
		[[nodiscard]] smr<const AccelerationEffect> getEffect() const noexcept;

		[[nodiscard]] cref<Vector<smr<StageDerivat>>> getStageDerivates() const noexcept;

	protected:
		BindLayout _bindLayout;

	public:
		[[nodiscard]] cref<BindLayout> getBindingLayout() const noexcept;

		void setBindingLayout(mref<BindLayout> layout_) noexcept;
	};
}
