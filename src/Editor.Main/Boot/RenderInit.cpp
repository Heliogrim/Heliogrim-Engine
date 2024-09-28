#include "RenderInit.hpp"

#include <Editor.Render/Renderer/EditorRenderer.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

void editor::boot::initRender() {

	const auto engine = Engine::getEngine();
	const auto graphics = engine->getGraphics();

	::hg::assertrt(not graphics->hasRenderer("Editor-Renderer"));

	auto renderer = make_smr<editor::render::EditorRenderer>(
		graphics->cacheCtrl(),
		graphics->getCurrentDevice()->allocator()
	);
	graphics->addRenderer("Editor-Renderer"sv, std::move(renderer));
}
