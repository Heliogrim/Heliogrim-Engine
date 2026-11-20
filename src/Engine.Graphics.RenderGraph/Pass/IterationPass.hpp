#pragma once

#include <cassert>

#include "../Node/RuntimeNode.hpp"
#include "../Visitor/Visitor.hpp"
#include "IterationPassContext.hpp"
#include "RenderGraphPass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
	class IterationPass :
		public RenderGraphPass {
	public:
		friend class VolatileIterationVisitor;

	public:
		using this_type = IterationPass;

	public:
		IterationPass(ref<SymbolContext> symbolContext_) noexcept :
			RenderGraphPass(),
			_context(symbolContext_) {}

		~IterationPass() noexcept = default;

	private:
		IterationPassContext _context;

	public:
		class VolatileIterationVisitor final :
			public graph::Visitor {
		private:
			const ptr<IterationPass> _owner;

		public:
			VolatileIterationVisitor(const ptr<IterationPass> owner_) noexcept;

			~VolatileIterationVisitor() noexcept override;

		private:
			Queue<nmpt<Node>> _backlog;

			void step();

			void unroll();

		public:
			void operator()(cref<Node> node_) override;

			void operator()(cref<CompileNode> node_) override {
				assert(false);
			}
		};

	public:
		[[nodiscard]] VolatileIterationVisitor visitor() noexcept {
			return VolatileIterationVisitor { this };
		}
	};
}
