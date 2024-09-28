#include "EditorRenderer.hpp"

#include <utility>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Subpass/Impl/DummyProvider.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TmpEndPass.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubpassNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Pass/CompilePassContext.hpp>
#include <Engine.Reflow.Render/ReflowPass.hpp>

using namespace hg::editor::render;
using namespace hg::engine::render;
using namespace hg;

EditorRenderer::EditorRenderer(
	mref<nmpt<engine::gfx::cache::GlobalCacheCtrl>> globalCache_,
	mref<nmpt<engine::gfx::memory::GlobalPooledAllocator>> globalGfxAllocator_
) noexcept :
	Renderer(
		Guid { static_cast<u32>("__Static__Proxy"_typeId.data), 0u, 0u, "Editor-Renderer"_typeId.data },
		"Editor-Renderer",
		makeCompileGraph(),
		nullptr,
		std::move(globalCache_),
		std::move(globalGfxAllocator_)
	) {}

EditorRenderer::~EditorRenderer() noexcept = default;

uptr<engine::render::graph::CompileGraph> EditorRenderer::makeCompileGraph() noexcept {

	auto graph = make_uptr<graph::CompileGraph>();
	graph->addExpectedProvision(graph::Provision { makeSceneColorSymbol() });

	/**/

	const nmpt<graph::Node> begin = graph->begin().get();
	const nmpt<graph::Node> end = graph->end().get();

	nmpt<graph::Node> cursor = begin;

	/**/

	auto beforeBarrier = make_smr<graph::BarrierNode>();
	auto beginPass = make_smr<graph::CompileSubPassNode>();
	auto uiPass = make_smr<graph::CompileSubPassNode>();
	auto endPass = make_smr<graph::CompileSubPassNode>();
	auto afterBarrier = make_smr<graph::BarrierNode>();

	/**/

	beginPass->setSubPassBuilder(
		[](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
			auto node = ctx_.getGraphNodeAllocator()
				->allocate<graph::SubPassNode<DummyProvider>>();
			return node;
		}
	);

	uiPass->setSubPassBuilder(
		[](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
			auto node = ctx_.getGraphNodeAllocator()
				->allocate<graph::SubPassNode<engine::reflow::render::ReflowPass>>();
			return node;
		}
	);

	endPass->setSubPassBuilder(
		[](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
			auto node = ctx_.getGraphNodeAllocator()
				->allocate<graph::SubPassNode<TmpEndPass>>(vk::ImageLayout::ePresentSrcKHR);
			return node;
		}
	);

	/**/

	nmpt<graph::Node> nextCursor = beforeBarrier.get();
	graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(beforeBarrier));

	cursor = std::exchange(nextCursor, beginPass.get());
	graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(beginPass));

	cursor = std::exchange(nextCursor, uiPass.get());
	graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(uiPass));

	cursor = std::exchange(nextCursor, endPass.get());
	graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(endPass));

	cursor = std::exchange(nextCursor, afterBarrier.get());
	graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(afterBarrier));

	/**/

	return graph;
}
