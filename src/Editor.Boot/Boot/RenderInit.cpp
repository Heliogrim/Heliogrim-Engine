import Editor.Boot;
import Editor.Render;
import Engine.Core;
import Engine.Graphics.Core;

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

void editor::boot::initRender() {

	const auto engine = Engine::getEngine();
	auto graphics = Graphics::get();

	::hg::assertrt(not graphics->hasRenderer("Editor-Renderer"));

	auto renderer = make_smr<editor::render::EditorRenderer>(
		graphics->cacheCtrl(),
		graphics->getCurrentDevice()->allocator()
	);
	graphics->addRenderer("Editor-Renderer"sv, std::move(renderer));
}
