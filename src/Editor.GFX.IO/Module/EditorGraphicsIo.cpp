#include "EditorGraphicsIo.hpp"

#include <Editor.GFX.Event/EditorGraphicsRegisterTransformerEvent.hpp>
#include <Editor.GFX.Loader/Texture/EditorTextureLoader.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Bmp.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Exr.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Hdr.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Jpeg.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Png.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Tga.hpp>
#include <Editor.GFX.Loader/Texture/Transformer/Tiff.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.GFX.Event/GraphicRegisterImporterEvent.hpp>
#include <Engine.GFX.Event/GraphicRegisterLoaderEvent.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.Resource/LoaderManager.hpp>

#include "EditorGraphicsIoDepKey.hpp"

using namespace ::hg::editor;
using namespace ::hg;

EditorGraphicsIo::EditorGraphicsIo(nmpt<engine::Engine> engine_) :
	SubModule(engine_.get()),
	_dependencies(
		{
			engine::core::SubModuleDependency {
				engine::core::GraphicsDepKey,
				engine::core::SubModuleOrder::eSuccessor,
				true
			}
		}
	),
	_coreLoaderHookHandle(~0uLL),
	_coreImporterHookHandle(~0uLL) {
	/**/
	hookGraphicsEvents();
}

EditorGraphicsIo::~EditorGraphicsIo() {
	unhookGraphicsEvents();
}

engine::core::DependencyKey EditorGraphicsIo::moduleKey() const noexcept {
	return EditorGraphicsIoDepKey;
}

cref<CompactSet<engine::core::SubModuleDependency>> EditorGraphicsIo::dependencies() const noexcept {
	return _dependencies;
}

void EditorGraphicsIo::hookGraphicsEvents() noexcept {

	/* Editor Graphics Events */

	_editorTransformerHookHandle = _engine->getEmitter().on<EditorGraphicsRegisterTransformerEvent>(
		[this](cref<EditorGraphicsRegisterTransformerEvent> event_) {

			using namespace ::hg::editor::gfx::loader;

			auto& pool = *_engine->getGraphics()->pool();

			event_.getProxy()
			      .put(make_uptr<EditorBmpTextureTransformer>(pool))
			      .put(make_uptr<EditorExrTextureTransformer>())
			      .put(make_uptr<EditorHdrTextureTransformer>(pool))
			      .put(make_uptr<EditorJpegTextureTransformer>(pool))
			      .put(make_uptr<EditorPngTextureTransformer>(pool))
			      .put(make_uptr<EditorTgaTextureTransformer>(pool))
			      .put(make_uptr<EditorTiffTextureTransformer>());
		}
	);

	/* Core Graphics Events */

	_coreLoaderHookHandle = _engine->getEmitter().on<GraphicRegisterLoaderEvent>(
		[this](cref<GraphicRegisterLoaderEvent> event_) {

			using namespace ::hg::engine::gfx;

			if (not event_.getLoaderManager().hasLoader<engine::assets::TextureAsset, TextureResource>()) {
				auto loader = make_sptr<gfx::loader::EditorTextureLoader>(
					event_.getGraphicsGlobalCacheCtrl(),
					event_.getGraphicsGlobalResourcePool(),
					event_.getLoaderManager().sharedSourceLoader()
				);
				event_.getLoaderManager().registerLoader<engine::assets::TextureAsset, TextureResource>(loader);

				/* Warning: Temporary */
				auto ev = EditorGraphicsRegisterTransformerEvent { loader->transformer };
				_engine->getEmitter().emit<EditorGraphicsRegisterTransformerEvent>(ev);
			}

		}
	);

	_coreImporterHookHandle = _engine->getEmitter().on<GraphicRegisterImporterEvent>(
		[this](cref<GraphicRegisterImporterEvent> event_) {}
	);

}

void EditorGraphicsIo::unhookGraphicsEvents() noexcept {

	/* Core Graphics Events */

	_engine->getEmitter().remove<GraphicRegisterLoaderEvent>(std::exchange(_coreLoaderHookHandle, ~0uLL));
	_engine->getEmitter().remove<GraphicRegisterImporterEvent>(std::exchange(_coreImporterHookHandle, ~0uLL));

	/* Editor Graphics Events */

	_engine->getEmitter().remove<EditorGraphicsRegisterTransformerEvent>(
		std::exchange(_editorTransformerHookHandle, ~0uLL)
	);
}

void EditorGraphicsIo::setup() {}

void EditorGraphicsIo::start() {}

void EditorGraphicsIo::stop() {}

void EditorGraphicsIo::destroy() {}
