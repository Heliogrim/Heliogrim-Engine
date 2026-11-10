#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Graphics/Cache/__fwd.hpp>
#include <Engine.Graphics/Memory/__fwd.hpp>
#include <Engine.Graphics.RenderGraph/__fwd.hpp>

#include "InjectGraphRegistry.hpp"

namespace hg::engine::render {
	class Renderer {
	public:
		using this_type = Renderer;

	protected:
		Renderer() noexcept;

	public:
		Renderer(
			mref<Guid> guid_,
			mref<string> name_,
			mref<uptr<graph::CompileGraph>> compileGraph_,
			mref<smr<graph::InjectGraphRegistry>> injectionRegistry_,
			/**/
			mref<nmpt<graphics::cache::GlobalCacheCtrl>> globalCache_,
			mref<nmpt<graphics::memory::GlobalPooledAllocator>> globalGfxAllocator_
		) noexcept;

		virtual ~Renderer();

	private:
		Guid _guid;
		string _name;

		smr<graph::CompileGraph> _compileGraph;
		smr<graph::InjectGraphRegistry> _injectReg;

	private:
		nmpt<graphics::cache::GlobalCacheCtrl> _globalCache;
		nmpt<graphics::memory::GlobalPooledAllocator> _globalGfxAlloc;

	private:
		[[nodiscard]] virtual uptr<graph::RuntimeGraph> makeDefaultRuntimeGraph() const;

	public:
		[[nodiscard]] _Success_(return != nullptr) virtual uptr<class RenderPass> allocate() const;

		[[nodiscard]] virtual std::pair<uptr<RenderPass>, bool> reallocate(
			mref<uptr<RenderPass>> pass_
		) const;

		virtual void free(mref<uptr<RenderPass>> pass_) const;

	private:
		[[nodiscard]] virtual uptr<RenderPass> updateIncremental(
			mref<uptr<RenderPass>> pass_
		) const;

	public:
		// TODO: Check whether we need this member function, cause we only delegate to internal incremental update
		[[nodiscard]] virtual uptr<RenderPass> update(mref<uptr<RenderPass>> pass_) const;
	};
}
