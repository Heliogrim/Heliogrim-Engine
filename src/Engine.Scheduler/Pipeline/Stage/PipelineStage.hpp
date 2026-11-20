#pragma once

#include <Engine.Common/__macro.hpp>

#include "StageDispatcher.hpp"
#include "StageIdentifier.hpp"
#include "../StagePipeline.hpp"
#include "../../Graph/StageDecl.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler {
	class macro_novtable PipelineStage {
	public:
		using this_type = PipelineStage;

		using identifier_type = AssocKey<StageIdentifier>;
		using pipeline_handle_type = const non_owning_rptr<const StagePipeline>;

	protected:
		PipelineStage(
			cref<identifier_type> identifier_,
			pipeline_handle_type pipeline_
		) noexcept;

	public:
		virtual ~PipelineStage() noexcept;

	protected:
		const identifier_type _stageIdentifier;

	public:
		[[nodiscard]] cref<identifier_type> stageIdentifier() const noexcept;

	protected:
		pipeline_handle_type _stagePipeline;

	public:
		[[nodiscard]] pipeline_handle_type stagePipeline() const noexcept;

	public:
		virtual void staticDispatch(cref<StaticStageDispatcher> dispatcher_) = 0;

		virtual void dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) = 0;

		[[nodiscard]] virtual graph::StageDecl decl() const noexcept = 0;
	};
}
