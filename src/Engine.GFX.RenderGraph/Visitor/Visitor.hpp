#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../__fwd.hpp"

namespace hg::engine::render::graph {
	class macro_novtable Visitor {
	public:
		using this_type = Visitor;

	protected:
		Visitor() noexcept = default;

	public:
		virtual ~Visitor() noexcept = default;

	public:
		virtual void operator()(cref<Node> node_) = 0;

	public:
		virtual void operator()(cref<AnchorNode> node_);

		virtual void operator()(cref<BarrierNode> node_);

		virtual void operator()(cref<ConvergeNode> node_);

		virtual void operator()(cref<DivergeNode> node_);

		virtual void operator()(cref<SelectorNode> node_);

		virtual void operator()(cref<ProviderNode> node_);

		virtual void operator()(cref<SLNode> node_);

		virtual void operator()(cref<SubPassNodeBase> node_);

	public:
		virtual void operator()(cref<CompileNode> node_) = 0;

	public:
		virtual void operator()(cref<CompileSubPassNode> node_);
	};
}
