#pragma once

#include <Engine.Common/Collection/Queue.hpp>

#include "RenderGraphPass.hpp"
#include "ResolvePassContext.hpp"
#include "../Visitor/Visitor.hpp"
#include "../Node/RuntimeNode.hpp"

namespace hg::engine::render::graph {
	class ResolvePass final :
		public RenderGraphPass {
	public:
		class VolatileResolveVisitor;
		friend class VolatileResolveVisitor;

	public:
		using this_type = ResolvePass;

	public:
		constexpr ResolvePass(ref<SymbolContext> symbolContext_) noexcept :
			RenderGraphPass(),
			_context(symbolContext_) {}

		constexpr ~ResolvePass() noexcept = default;

	private:
		ResolvePassContext _context;

	public:
		class VolatileResolveVisitor final :
			public Visitor {
		public:
			using this_type = VolatileResolveVisitor;

		public:
			VolatileResolveVisitor(const ptr<ResolvePass> owner_) noexcept;

			~VolatileResolveVisitor() noexcept override;

		private:
			const ptr<ResolvePass> _owner;

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
		[[nodiscard]] VolatileResolveVisitor visitor() noexcept {
			return VolatileResolveVisitor { this };
		}
	};
}
