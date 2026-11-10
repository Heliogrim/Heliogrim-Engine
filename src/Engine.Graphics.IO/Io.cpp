#include "Io.hpp"

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Graphics.Event/GraphicRegisterImporterEvent.hpp>
#include <Engine.Graphics.Event/GraphicRegisterLoaderEvent.hpp>
#include <Engine.Graphics.Event/GraphicUnregisterImporterEvent.hpp>
#include <Engine.Graphics.Event/GraphicUnregisterLoaderEvent.hpp>
#include <Engine.Graphics.Loader/Font/FontLoader.hpp>
#include <Engine.Graphics.Loader/Geometry/StaticGeometryLoader.hpp>
#include <Engine.Graphics.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.Graphics.Loader/Material/MaterialLoader.hpp>
#include <Engine.Graphics.Loader/Material/MaterialPrototypeLoader.hpp>
#include <Engine.Graphics.Loader/Material/MaterialPrototypeResource.hpp>
#include <Engine.Graphics.Loader/Material/MaterialResource.hpp>
#include <Engine.Graphics.Loader/Texture/TextureLoader.hpp>
#include <Engine.Graphics.Loader/Texture/TextureResource.hpp>
#include <Engine.Graphics/Importer/FontFileTypes.hpp>
#include <Engine.Graphics/Importer/FontImporter.hpp>
#include <Engine.Graphics/Importer/ImageFileTypes.hpp>
#include <Engine.Graphics/Importer/ImageImporter.hpp>
#include <Engine.Graphics/Importer/ModelFileTypes.hpp>
#include <Engine.Graphics/Importer/ModelImporter.hpp>

using namespace ::hg::engine::graphics::loader;
using namespace ::hg;

template <typename LoaderType_, typename AssetType_, typename ResourceType_, typename... Args_>
static void registerIfNotPresent(ref<engine::resource::LoaderManager> manager_, Args_&&... args_) noexcept {

	if (manager_.hasLoader<AssetType_, ResourceType_>()) {
		return;
	}

	auto loader = ::hg::make_sptr<LoaderType_>(std::forward<Args_>(args_)...);
	manager_.registerLoader<AssetType_, ResourceType_>(std::move(loader));
}

template <typename ImporterType_, typename... Args_>
static void registerIfNotPresent(
	ref<engine::res::ImporterManager> manager_,
	cref<engine::res::FileTypeId> fileTypeId_,
	Args_&&... args_
) noexcept {

	if (manager_.hasImporter(fileTypeId_)) {
		return;
	}

	auto importer = ::hg::make_sptr<ImporterType_>(std::forward<Args_>(args_)...);
	manager_.registerImporter(fileTypeId_, importer);
}

/**/

void engine::graphics::loader::register_loader(
	ref<resource::LoaderManager> manager_,
	ref<Graphics> graphics_,
	ref<graphics::cache::GlobalCacheCtrl> cacheCtrl_,
	ref<graphics::pool::GlobalResourcePool> pool_
) {

	auto event = GraphicRegisterLoaderEvent { manager_, graphics_, cacheCtrl_, pool_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	registerIfNotPresent<TextureLoader, assets::TextureAsset, TextureResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<StaticGeometryLoader, assets::StaticGeometry, StaticGeometryResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<MaterialPrototypeLoader, assets::GfxMaterialPrototype, MaterialPrototypeResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<MaterialLoader, assets::GfxMaterial, MaterialResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<FontLoader, assets::FontAsset, FontResource>(
		manager_,
		cacheCtrl_,
		pool_,
		manager_.sharedSourceLoader()
	);
}

void engine::graphics::loader::unregister_loader(
	ref<resource::LoaderManager> manager_,
	ref<Graphics> graphics_,
	ref<graphics::cache::GlobalCacheCtrl> cacheCtrl_,
	ref<graphics::pool::GlobalResourcePool> pool_
) {

	auto event = GraphicUnregisterLoaderEvent { manager_, graphics_, cacheCtrl_, pool_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	manager_.unregisterLoader<assets::TextureAsset>();
	manager_.unregisterLoader<assets::StaticGeometry>();
	manager_.unregisterLoader<assets::GfxMaterial>();
	manager_.unregisterLoader<assets::GfxMaterialPrototype>();
	manager_.unregisterLoader<assets::FontAsset>();
}

void engine::graphics::loader::register_importer(ref<res::ImporterManager> manager_, ref<Graphics> graphics_) {

	auto event = GraphicRegisterImporterEvent { manager_, graphics_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	registerIfNotPresent<KtxImporter>(manager_, ImageFileType::Ktx);
	registerIfNotPresent<KtxImporter>(manager_, ImageFileType::Ktx2);
	registerIfNotPresent<FbxImporter>(manager_, ModelFileType::Fbx);
	registerIfNotPresent<FbxImporter>(manager_, ModelFileType::Obj);
	registerIfNotPresent<FontImporter>(manager_, FontFileType::Otf);
	registerIfNotPresent<FontImporter>(manager_, FontFileType::Ttf);
}

void engine::graphics::loader::unregister_importer(ref<res::ImporterManager> manager_, ref<Graphics> graphics_) {

	auto event = GraphicUnregisterImporterEvent { manager_, graphics_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	manager_.unregisterImporter(ImageFileType::Ktx);
	manager_.unregisterImporter(ImageFileType::Ktx2);
	manager_.unregisterImporter(ModelFileType::Fbx);
	manager_.unregisterImporter(ModelFileType::Obj);
	manager_.unregisterImporter(FontFileType::Otf);
	manager_.unregisterImporter(FontFileType::Ttf);
}
