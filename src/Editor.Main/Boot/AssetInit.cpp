#include "AssetInit.hpp"

#include <Heliogrim/Heliogrim.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Common/GuidFormat.hpp>

#include <filesystem>
#include <fstream>

#include "Heliogrim.Default/Assets/Fonts/CascadiaCode.hpp"
#include "Heliogrim.Default/Assets/GfxMaterials/DefaultMaterial.hpp"
#include "Heliogrim.Default/Assets/GfxMaterials/DefaultMaterial2.hpp"
#include "Heliogrim.Default/Assets/GfxMaterials/DefaultSkybox.hpp"
#include "Heliogrim.Default/Assets/Images/Brand.hpp"
#include "Heliogrim.Default/Assets/Images/DefaultMetalness.hpp"
#include "Heliogrim.Default/Assets/Images/DefaultRoughness.hpp"
#include "Heliogrim.Default/Assets/Images/Directory.hpp"
#include "Heliogrim.Default/Assets/Images/FolderAnimation.hpp"
#include "Heliogrim.Default/Assets/Images/FolderAudio.hpp"
#include "Heliogrim.Default/Assets/Images/FolderConfig.hpp"
#include "Heliogrim.Default/Assets/Images/FolderContent.hpp"
#include "Heliogrim.Default/Assets/Images/FolderCore.hpp"
#include "Heliogrim.Default/Assets/Images/FolderDatabase.hpp"
#include "Heliogrim.Default/Assets/Images/FolderDebug.hpp"
#include "Heliogrim.Default/Assets/Images/FolderEnvironment.hpp"
#include "Heliogrim.Default/Assets/Images/FolderError.hpp"
#include "Heliogrim.Default/Assets/Images/FolderFont.hpp"
#include "Heliogrim.Default/Assets/Images/FolderGenerator.hpp"
#include "Heliogrim.Default/Assets/Images/FolderImages.hpp"
#include "Heliogrim.Default/Assets/Images/FolderImport.hpp"
#include "Heliogrim.Default/Assets/Images/FolderLib.hpp"
#include "Heliogrim.Default/Assets/Images/FolderLog.hpp"
#include "Heliogrim.Default/Assets/Images/FolderMeta.hpp"
#include "Heliogrim.Default/Assets/Images/FolderPackages.hpp"
#include "Heliogrim.Default/Assets/Images/FolderResource.hpp"
#include "Heliogrim.Default/Assets/Images/FolderScripts.hpp"
#include "Heliogrim.Default/Assets/Images/FolderShader.hpp"
#include "Heliogrim.Default/Assets/Images/FolderShared.hpp"
#include "Heliogrim.Default/Assets/Images/FolderTemplate.hpp"
#include "Heliogrim.Default/Assets/Images/FolderVideo.hpp"
#include "Heliogrim.Default/Assets/Images/Icon3d.hpp"
#include "Heliogrim.Default/Assets/Images/IconArchitecture.hpp"
#include "Heliogrim.Default/Assets/Images/IconAudio.hpp"
#include "Heliogrim.Default/Assets/Images/IconC.hpp"
#include "Heliogrim.Default/Assets/Images/IconCMake.hpp"
#include "Heliogrim.Default/Assets/Images/IconCommand.hpp"
#include "Heliogrim.Default/Assets/Images/IconConsole.hpp"
#include "Heliogrim.Default/Assets/Images/IconContributing.hpp"
#include "Heliogrim.Default/Assets/Images/IconCpp.hpp"
#include "Heliogrim.Default/Assets/Images/IconCSharp.hpp"
#include "Heliogrim.Default/Assets/Images/IconDatabase.hpp"
#include "Heliogrim.Default/Assets/Images/IconDiff.hpp"
#include "Heliogrim.Default/Assets/Images/IconDocument.hpp"
#include "Heliogrim.Default/Assets/Images/IconFavicon.hpp"
#include "Heliogrim.Default/Assets/Images/IconFont.hpp"
#include "Heliogrim.Default/Assets/Images/IconGit.hpp"
#include "Heliogrim.Default/Assets/Images/IconH.hpp"
#include "Heliogrim.Default/Assets/Images/IconHpp.hpp"
#include "Heliogrim.Default/Assets/Images/IconI18n.hpp"
#include "Heliogrim.Default/Assets/Images/IconImage.hpp"
#include "Heliogrim.Default/Assets/Images/IconJson.hpp"
#include "Heliogrim.Default/Assets/Images/IconLib.hpp"
#include "Heliogrim.Default/Assets/Images/IconLog.hpp"
#include "Heliogrim.Default/Assets/Images/IconLua.hpp"
#include "Heliogrim.Default/Assets/Images/IconPdf.hpp"
#include "Heliogrim.Default/Assets/Images/IconPipeline.hpp"
#include "Heliogrim.Default/Assets/Images/IconReadme.hpp"
#include "Heliogrim.Default/Assets/Images/IconSearch.hpp"
#include "Heliogrim.Default/Assets/Images/IconSettings.hpp"
#include "Heliogrim.Default/Assets/Images/IconShader.hpp"
#include "Heliogrim.Default/Assets/Images/IconTemplate.hpp"
#include "Heliogrim.Default/Assets/Images/IconTodo.hpp"
#include "Heliogrim.Default/Assets/Images/IconTree.hpp"
#include "Heliogrim.Default/Assets/Images/IconUrl.hpp"
#include "Heliogrim.Default/Assets/Images/IconVideo.hpp"
#include "Heliogrim.Default/Assets/Images/IconZip.hpp"
#include "Heliogrim.Default/Assets/Textures/DefaultDiffuse.hpp"
#include "Heliogrim.Default/Assets/Textures/DefaultNormal.hpp"
#include "Heliogrim.Default/Assets/Textures/DefaultRoughness.hpp"
#include "Heliogrim.Default/Assets/Textures/Directory.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderAnimation.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderAudio.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderConfig.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderContent.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderCore.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderDatabase.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderDebug.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderEnvironment.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderError.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderFont.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderGenerator.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderImages.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderImport.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderLib.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderLog.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderMeta.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderPackages.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderResource.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderScripts.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderShader.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderShared.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderTemplate.hpp"
#include "Heliogrim.Default/Assets/Textures/FolderVideo.hpp"
#include "Heliogrim.Default/Assets/Textures/Icon3d.hpp"
#include "Heliogrim.Default/Assets/Textures/IconArchitecture.hpp"
#include "Heliogrim.Default/Assets/Textures/IconAudio.hpp"
#include "Heliogrim.Default/Assets/Textures/IconC.hpp"
#include "Heliogrim.Default/Assets/Textures/IconCMake.hpp"
#include "Heliogrim.Default/Assets/Textures/IconCommand.hpp"
#include "Heliogrim.Default/Assets/Textures/IconConsole.hpp"
#include "Heliogrim.Default/Assets/Textures/IconContributing.hpp"
#include "Heliogrim.Default/Assets/Textures/IconCpp.hpp"
#include "Heliogrim.Default/Assets/Textures/IconCSharp.hpp"
#include "Heliogrim.Default/Assets/Textures/IconDatabase.hpp"
#include "Heliogrim.Default/Assets/Textures/IconDiff.hpp"
#include "Heliogrim.Default/Assets/Textures/IconDocument.hpp"
#include "Heliogrim.Default/Assets/Textures/IconFavicon.hpp"
#include "Heliogrim.Default/Assets/Textures/IconFont.hpp"
#include "Heliogrim.Default/Assets/Textures/IconGit.hpp"
#include "Heliogrim.Default/Assets/Textures/IconH.hpp"
#include "Heliogrim.Default/Assets/Textures/IconHpp.hpp"
#include "Heliogrim.Default/Assets/Textures/IconI18n.hpp"
#include "Heliogrim.Default/Assets/Textures/IconImage.hpp"
#include "Heliogrim.Default/Assets/Textures/IconJson.hpp"
#include "Heliogrim.Default/Assets/Textures/IconLib.hpp"
#include "Heliogrim.Default/Assets/Textures/IconLog.hpp"
#include "Heliogrim.Default/Assets/Textures/IconLua.hpp"
#include "Heliogrim.Default/Assets/Textures/IconPdf.hpp"
#include "Heliogrim.Default/Assets/Textures/IconPipeline.hpp"
#include "Heliogrim.Default/Assets/Textures/IconReadme.hpp"
#include "Heliogrim.Default/Assets/Textures/IconSearch.hpp"
#include "Heliogrim.Default/Assets/Textures/IconSettings.hpp"
#include "Heliogrim.Default/Assets/Textures/IconShader.hpp"
#include "Heliogrim.Default/Assets/Textures/IconTemplate.hpp"
#include "Heliogrim.Default/Assets/Textures/IconTodo.hpp"
#include "Heliogrim.Default/Assets/Textures/IconTree.hpp"
#include "Heliogrim.Default/Assets/Textures/IconUrl.hpp"
#include "Heliogrim.Default/Assets/Textures/IconVideo.hpp"
#include "Heliogrim.Default/Assets/Textures/IconZip.hpp"
#include "Engine.Assets/AssetFactory.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Assets/Types/Image.hpp"
#include "Engine.Assets/Types/Font.hpp"
#include "Engine.Assets/Types/Geometry/StaticGeometry.hpp"
#include "Engine.Assets/Types/Texture/TextureAsset.hpp"
#include "Engine.Resource.Package/PackageManager.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Resource/Source/FileSource.hpp"
#include "Engine.Resource.Package/PackageFactory.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"
#include "Engine.Serialization.Layouts/LayoutManager.hpp"
#include "Game.Main/Assets/Images/DefaultAlpha.hpp"
#include "Game.Main/Assets/Images/DefaultAO.hpp"
#include "Game.Main/Assets/Images/DefaultDiffuse.hpp"
#include "Game.Main/Assets/Images/DefaultNormal.hpp"
#include "Game.Main/Assets/Images/DefaultSkybox.hpp"
#include "Game.Main/Assets/Meshes/Sphere.hpp"
#include "Game.Main/Assets/Meshes/PlaneD128.hpp"
#include "Game.Main/Assets/Textures/DefaultAlpha.hpp"
#include "Game.Main/Assets/Textures/DefaultAO.hpp"
#include "Game.Main/Assets/Textures/DefaultMetalness.hpp"
#include "Game.Main/Assets/Textures/DefaultSkybox.hpp"

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

using path = _STD filesystem::path;

namespace hg::engine::serialization {
    class RecordScopedSlot;
}

/**/
void packageDiscoverAssets();

bool isArchivedAsset(mref<serialization::RecordScopedSlot> record_);

bool tryLoadArchivedAsset(mref<serialization::RecordScopedSlot> record_);

/**/

#pragma region File System Loading

void dispatchLoad(cref<path> path_) {
    scheduler::exec(
        [file = path_]() {
            // TODO:

            constexpr auto minLength = sizeof(asset_guid) + sizeof(asset_type_id);

            _STD ifstream ifs { file, _STD ios::in | _STD ios::binary };
            ifs.seekg(0, _STD ios::beg);

            serialization::BufferArchive archive {};
            archive.resize(minLength, 0ui8);

            auto start { ifs.tellg() };
            ifs.read(reinterpret_cast<char*>(archive.data()), archive.size());
            auto end { ifs.tellg() };

            if (ifs.bad()) {
                // Can't even read asset guid or type id, so no viable data
                return;
            }

            auto length { end - start };
            if (length < minLength) {
                // Can't even read asset guid or type id, so no viable data
                return;
            }

            /**/

            asset_guid possibleGuid { invalid_asset_guid };
            asset_type_id possibleTypeId {};

            // TODO: Use predefined data layouts to deserialize guid and type id, cause they are not representable as u64 at every platform
            archive >> reinterpret_cast<ref<u64>>(possibleGuid);
            archive >> reinterpret_cast<ref<u64>>(possibleTypeId);

            if (possibleGuid == invalid_asset_guid) {
                return;
            }

            if (possibleTypeId.data == 0) {
                return;
            }

            /**/

            const auto& layouts = serialization::LayoutManager::get();
            const auto layout = layouts.getLayout(possibleTypeId);

            if (not layout || not layout->hasClass()) {
                // Seams not to be a valid data package, asset type was not bootstrapped correctly or asset is not generically reconstructable
                return;
            }

            /**/

            ifs.seekg(0, _STD ios::end);
            end = ifs.tellg();

            ifs.seekg(0, _STD ios::beg);
            start = ifs.tellg();

            archive.seek(0);
            archive.resize(end - start, 0ui8);

            ifs.read(reinterpret_cast<ptr<char>>(archive.data()), archive.size());

            if (ifs.bad()) {
                // Something went wrong while reading the actual data
                IM_CORE_ERRORF("Received an error while reading possible asset file `{}`", file.string());
                return;
            }

            /**/

            // Warning: Unsafe operation, should be capsulated within io system
            auto* obj { layout->reflect().instantiate() };
            const _STD span<u8, _STD dynamic_extent> view {
                reinterpret_cast<ptr<u8>>(obj),
                static_cast<u64>(layout->size())
            };

            layout->dispatch().load(archive, view);

            /**/

            auto* asset { static_cast<ptr<engine::assets::Asset>>(obj) };
            Engine::getEngine()->getAssets()->getRegistry()->insert({ asset });
        }
    );
}

void tryDispatchLoad(cref<path> path_) {

    const auto extension = path_.extension().string();

    if (extension.empty()) {
        return;
    }

    auto* const pm = Engine::getEngine()->getResources()->packages(traits::nothrow);
    hg::fs::File file { path_ };

    if (pm->isPackageFile(file)) {

        // TODO: Load Package from file
        auto source = make_uptr<resource::FileSource>(_STD move(file));
        auto package = resource::PackageFactory::createFromSource(_STD move(source));

        /**/

        auto managed = make_smr<resource::UniqueResource<resource::PackageResource::value_type>>(package.release());

        /**/

        pm->addPackage(_STD move(managed));

    } else if (extension.ends_with(".ima") || extension.ends_with(".imasset")) {

        dispatchLoad(path_);
    }
}

void indexLoadable(cref<path> path_, ref<Vector<path>> backlog_) {

    const auto directory {
        _STD filesystem::directory_iterator { path_, _STD filesystem::directory_options::follow_directory_symlink }
    };

    for (const auto& entry : directory) {

        if (entry.is_directory()) {
            backlog_.push_back(entry.path());
            continue;
        }

        if (entry.is_regular_file()) {
            tryDispatchLoad(entry.path());
            //continue;
        }
    }
}

#pragma endregion

#pragma region Predefined Loading

static void initMaterialDefaults() {

    using namespace ::hg::game::assets;
    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();

    factory->createImageAsset(
        image::DefaultAlpha::unstable_auto_guid(),
        R"(resources\imports\ktx\default_alpha.ktx)"
    );

    factory->createImageAsset(
        image::DefaultAO::unstable_auto_guid(),
        R"(resources\imports\ktx\default_ao.ktx)"
    );

    factory->createImageAsset(
        image::DefaultDiffuse::auto_guid(),
        R"(resources\imports\ktx\default_diffuse.ktx)"
    );

    factory->createImageAsset(
        image::DefaultMetalness::unstable_auto_guid(),
        R"(resources\imports\ktx\default_metalness.ktx)"
    );

    factory->createImageAsset(
        image::DefaultNormal::unstable_auto_guid(),
        R"(resources\imports\ktx\default_normal.ktx)"
    );

    factory->createImageAsset(
        image::DefaultRoughness::unstable_auto_guid(),
        R"(resources\imports\ktx\default_roughness.ktx)"
    );

    /**/

    delete(new(texture::DefaultAlpha));
    delete(new(texture::DefaultAO));
    delete(new(texture::DefaultDiffuse));
    delete(new(texture::DefaultMetalness));
    delete(new(texture::DefaultNormal));
    delete(new(texture::DefaultRoughness));

    /**/

    {
        auto* tmp = new(material::DefaultMaterial);
        tmp->setAssetName("Default Material"sv);
        delete tmp;
    }

    {
        auto* tmp = new(material::DefaultMaterial2);
        tmp->setAssetName("Default Material 2"sv);
        delete tmp;
    }
}

static void initSkyboxDefaults() {

    using namespace ::hg::game::assets;
    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();

    factory->createImageAsset(
        image::DefaultSkybox::unstable_auto_guid(),
        R"(resources\imports\ktx\default_skybox.ktx)"
    );

    /**/

    delete(new(texture::DefaultSkybox));

    /**/

    delete(new(material::DefaultSkybox));
}

static void initDirectoryIcons() {

    using namespace ::hg::game::assets;
    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();

    factory->createImageAsset(
        image::Directory::unstable_auto_guid(),
        R"(resources\imports\ktx\icons8-folder-144.ktx)"
    );

    factory->createImageAsset(
        image::FolderAnimation::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-animation.ktx)"
    );

    factory->createImageAsset(
        image::FolderAudio::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-audio.ktx)"
    );

    factory->createImageAsset(
        image::FolderConfig::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-config.ktx)"
    );

    factory->createImageAsset(
        image::FolderContent::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-content.ktx)"
    );

    factory->createImageAsset(
        image::FolderCore::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-core.ktx)"
    );

    factory->createImageAsset(
        image::FolderDatabase::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-database.ktx)"
    );

    factory->createImageAsset(
        image::FolderDebug::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-debug.ktx)"
    );

    factory->createImageAsset(
        image::FolderEnvironment::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-environment.ktx)"
    );

    factory->createImageAsset(
        image::FolderError::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-error.ktx)"
    );

    factory->createImageAsset(
        image::FolderFont::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-font.ktx)"
    );

    factory->createImageAsset(
        image::FolderGenerator::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-generator.ktx)"
    );

    factory->createImageAsset(
        image::FolderImages::unstable_auto_guid(),
        R"(resources\imports\ktx2\folder-images.ktx2)"
    );

    factory->createImageAsset(
        image::FolderImport::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-import.ktx)"
    );

    factory->createImageAsset(
        image::FolderLib::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-lib.ktx)"
    );

    factory->createImageAsset(
        image::FolderLog::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-log.ktx)"
    );

    factory->createImageAsset(
        image::FolderMeta::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-meta.ktx)"
    );

    factory->createImageAsset(
        image::FolderPackages::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-packages.ktx)"
    );

    factory->createImageAsset(
        image::FolderResource::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-resource.ktx)"
    );

    factory->createImageAsset(
        image::FolderScripts::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-scripts.ktx)"
    );

    factory->createImageAsset(
        image::FolderShader::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-shader.ktx)"
    );

    factory->createImageAsset(
        image::FolderShared::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-shared.ktx)"
    );

    factory->createImageAsset(
        image::FolderTemplate::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-template.ktx)"
    );

    factory->createImageAsset(
        image::FolderVideo::unstable_auto_guid(),
        R"(resources\imports\ktx\folder-video.ktx)"
    );

    /**/

    delete(new(texture::Directory));
    delete(new(texture::FolderAnimation));
    delete(new(texture::FolderAudio));
    delete(new(texture::FolderConfig));
    delete(new(texture::FolderContent));
    delete(new(texture::FolderCore));
    delete(new(texture::FolderDatabase));
    delete(new(texture::FolderDebug));
    delete(new(texture::FolderEnvironment));
    delete(new(texture::FolderError));
    delete(new(texture::FolderFont));
    delete(new(texture::FolderGenerator));
    delete(new(texture::FolderImages));
    delete(new(texture::FolderImport));
    delete(new(texture::FolderLib));
    delete(new(texture::FolderLog));
    delete(new(texture::FolderMeta));
    delete(new(texture::FolderPackages));
    delete(new(texture::FolderResource));
    delete(new(texture::FolderScripts));
    delete(new(texture::FolderShader));
    delete(new(texture::FolderShared));
    delete(new(texture::FolderTemplate));
    delete(new(texture::FolderVideo));
}

static void initFileIcons() {

    using namespace ::hg::game::assets;
    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();

    factory->createImageAsset(
        image::Brand::unstable_auto_guid(),
        R"(resources\imports\ktx\brand.ktx)"
    );

    factory->createImageAsset(
        image::Icon3d::unstable_auto_guid(),
        R"(resources\imports\ktx\3d.ktx)"
    );

    factory->createImageAsset(
        image::IconArchitecture::unstable_auto_guid(),
        R"(resources\imports\ktx\architecture.ktx)"
    );

    factory->createImageAsset(
        image::IconAudio::unstable_auto_guid(),
        R"(resources\imports\ktx\audio.ktx)"
    );

    factory->createImageAsset(
        image::IconC::unstable_auto_guid(),
        R"(resources\imports\ktx\c.ktx)"
    );

    factory->createImageAsset(
        image::IconCMake::unstable_auto_guid(),
        R"(resources\imports\ktx\cmake.ktx)"
    );

    factory->createImageAsset(
        image::IconCommand::unstable_auto_guid(),
        R"(resources\imports\ktx\command.ktx)"
    );

    factory->createImageAsset(
        image::IconConsole::unstable_auto_guid(),
        R"(resources\imports\ktx\console.ktx)"
    );

    factory->createImageAsset(
        image::IconContributing::unstable_auto_guid(),
        R"(resources\imports\ktx\contributing.ktx)"
    );

    factory->createImageAsset(
        image::IconCpp::unstable_auto_guid(),
        R"(resources\imports\ktx\cpp.ktx)"
    );

    factory->createImageAsset(
        image::IconCSharp::unstable_auto_guid(),
        R"(resources\imports\ktx\csharp.ktx)"
    );

    factory->createImageAsset(
        image::IconDatabase::unstable_auto_guid(),
        R"(resources\imports\ktx\database.ktx)"
    );

    factory->createImageAsset(
        image::IconDiff::unstable_auto_guid(),
        R"(resources\imports\ktx\diff.ktx)"
    );

    factory->createImageAsset(
        image::IconDocument::unstable_auto_guid(),
        R"(resources\imports\ktx\document.ktx)"
    );

    factory->createImageAsset(
        image::IconFavicon::unstable_auto_guid(),
        R"(resources\imports\ktx\favicon.ktx)"
    );

    factory->createImageAsset(
        image::IconFont::unstable_auto_guid(),
        R"(resources\imports\ktx\font.ktx)"
    );

    factory->createImageAsset(
        image::IconGit::unstable_auto_guid(),
        R"(resources\imports\ktx\git.ktx)"
    );

    factory->createImageAsset(
        image::IconH::unstable_auto_guid(),
        R"(resources\imports\ktx\h.ktx)"
    );

    factory->createImageAsset(
        image::IconHpp::unstable_auto_guid(),
        R"(resources\imports\ktx\hpp.ktx)"
    );

    factory->createImageAsset(
        image::IconI18n::unstable_auto_guid(),
        R"(resources\imports\ktx\i18n.ktx)"

    );

    factory->createImageAsset(
        image::IconImage::unstable_auto_guid(),
        R"(resources\imports\ktx\image.ktx)"

    );

    factory->createImageAsset(
        image::IconJson::unstable_auto_guid(),
        R"(resources\imports\ktx\json.ktx)"

    );

    factory->createImageAsset(
        image::IconLib::unstable_auto_guid(),
        R"(resources\imports\ktx\lib.ktx)"

    );

    factory->createImageAsset(
        image::IconLog::unstable_auto_guid(),
        R"(resources\imports\ktx\log.ktx)"

    );

    factory->createImageAsset(
        image::IconLua::unstable_auto_guid(),
        R"(resources\imports\ktx\lua.ktx)"

    );

    factory->createImageAsset(
        image::IconPdf::unstable_auto_guid(),
        R"(resources\imports\ktx\pdf.ktx)"

    );

    factory->createImageAsset(
        image::IconPipeline::unstable_auto_guid(),
        R"(resources\imports\ktx\pipeline.ktx)"

    );

    factory->createImageAsset(
        image::IconReadme::unstable_auto_guid(),
        R"(resources\imports\ktx\readme.ktx)"

    );

    factory->createImageAsset(
        image::IconSearch::unstable_auto_guid(),
        R"(resources\imports\ktx\search.ktx)"

    );

    factory->createImageAsset(
        image::IconSettings::unstable_auto_guid(),
        R"(resources\imports\ktx\settings.ktx)"

    );

    factory->createImageAsset(
        image::IconShader::unstable_auto_guid(),
        R"(resources\imports\ktx\shader.ktx)"

    );

    factory->createImageAsset(
        image::IconTemplate::unstable_auto_guid(),
        R"(resources\imports\ktx\template.ktx)"

    );

    factory->createImageAsset(
        image::IconTodo::unstable_auto_guid(),
        R"(resources\imports\ktx\todo.ktx)"

    );

    factory->createImageAsset(
        image::IconTree::unstable_auto_guid(),
        R"(resources\imports\ktx\tree.ktx)"

    );

    factory->createImageAsset(
        image::IconUrl::unstable_auto_guid(),
        R"(resources\imports\ktx\url.ktx)"

    );

    factory->createImageAsset(
        image::IconVideo::unstable_auto_guid(),
        R"(resources\imports\ktx\video.ktx)"

    );

    factory->createImageAsset(
        image::IconZip::unstable_auto_guid(),
        R"(resources\imports\ktx\zip.ktx)"

    );

    /**/

    delete(new(texture::Icon3d));
    delete(new(texture::IconArchitecture));
    delete(new(texture::IconAudio));
    delete(new(texture::IconC));
    delete(new(texture::IconCMake));
    delete(new(texture::IconCommand));
    delete(new(texture::IconConsole));
    delete(new(texture::IconContributing));
    delete(new(texture::IconCpp));
    delete(new(texture::IconCSharp));
    delete(new(texture::IconDatabase));
    delete(new(texture::IconDiff));
    delete(new(texture::IconDocument));
    delete(new(texture::IconFavicon));
    delete(new(texture::IconFont));
    delete(new(texture::IconGit));
    delete(new(texture::IconH));
    delete(new(texture::IconHpp));
    delete(new(texture::IconI18n));
    delete(new(texture::IconImage));
    delete(new(texture::IconJson));
    delete(new(texture::IconLib));
    delete(new(texture::IconLog));
    delete(new(texture::IconLua));
    delete(new(texture::IconPdf));
    delete(new(texture::IconPipeline));
    delete(new(texture::IconReadme));
    delete(new(texture::IconSearch));
    delete(new(texture::IconSettings));
    delete(new(texture::IconShader));
    delete(new(texture::IconTemplate));
    delete(new(texture::IconTodo));
    delete(new(texture::IconTree));
    delete(new(texture::IconUrl));
    delete(new(texture::IconVideo));
    delete(new(texture::IconZip));
}

static void initFontDefaults() {

    using namespace ::hg::game::assets;
    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();

    factory->createFontAsset(
        font::CascadiaCode::unstable_auto_guid(),
        R"(resources\imports\ttf\CascadiaCode.ttf)"

    );
}

static void initStaticGeometryDefaults() {

    using namespace ::hg::game::assets;
    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();

    factory->createStaticGeometryAsset(
        meshes::Sphere::unstable_auto_guid(),
        R"(resources\imports\obj\sphere.obj)",
        11520ui64,
        11520ui64

    );

    factory->createStaticGeometryAsset(
        meshes::PlaneD128::unstable_auto_guid(),
        R"(resources\imports\obj\plane_d128.obj)",
        98304ui64,
        98304ui64

    );
}

#pragma endregion

void editor::boot::initAssets() {

    initMaterialDefaults();
    initSkyboxDefaults();
    initDirectoryIcons();
    initFileIcons();
    initFontDefaults();
    initStaticGeometryDefaults();

    /**/

    const auto root { _STD filesystem::current_path().append(R"(..\..\resources\assets)") };
    Vector<path> backlog {};

    backlog.push_back(root);
    while (not backlog.empty()) {

        const auto cur { backlog.back() };
        backlog.pop_back();

        indexLoadable(cur, backlog);
    }

    /**/

    packageDiscoverAssets();

}

/**/

#include <Engine.Resource.Package/Linker/LinkedArchive.hpp>
#include <Engine.Resource.Package/Linker/PackageLinker.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchiveIterator.hpp>
#include <Engine.Serialization/Archive/SourceReadonlyArchive.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Reflect/Reflect.hpp>

void packageDiscoverAssets() {

    const auto* const pm = Engine::getEngine()->getResources()->packages(traits::nothrow);
    const auto view = pm->packages();

    for (const auto& resource : view) {

        const auto guard = resource->acquire(resource::ResourceUsageFlag::eRead);
        const auto* const package = guard.imm();
        const auto* const linker = package->getLinker();

        /* Check for stored archives */

        if (linker->getArchiveCount() <= 0) {
            continue;
        }

        /* Index / Load data from archives */

        const auto end = linker->cend();
        for (auto iter = linker->cbegin(); iter != end; ++iter) {

            if (iter.header().type != resource::ArchiveHeaderType::eSerializedStructure) {
                continue;
            }

            const auto linkedArchive = iter.archive();
            const serialization::StructuredArchive archive { linkedArchive.get() };

            /* Check for root stored assets */

            if (isArchivedAsset(archive.getRootSlot())) {

                linkedArchive->seek(0);
                tryLoadArchivedAsset(archive.getRootSlot());

                continue;
            }

            /* Check for a sequence of stored assets */

            {
                linkedArchive->seek(0);
                auto root = archive.getRootSlot();

                root.slot()->readHeader();
                if (root.slot()->getSlotHeader().type != serialization::StructureSlotType::eSeq) {
                    continue;
                }

                /* Check whether first element is asset, otherwise treat as unknown sequence */

                const auto seq = root.intoSeq();
                const auto count = seq.getRecordCount();

                if (count <= 0) {
                    continue;
                }

                if (not isArchivedAsset(seq.getRecordSlot(0))) {
                    continue;
                }

                /* Iterate over archives and try to load */

                for (s64 idx = 0; idx < count; ++idx) {
                    tryLoadArchivedAsset(seq.getRecordSlot(idx));
                }
            }
        }
    }
}

bool isArchivedAsset(mref<serialization::RecordScopedSlot> record_) {

    const auto record = record_.intoStruct();
    if (not record.slot()->validateType()) {
        return false;
    }

    /**/

    if (not record.hasRecordSlot("__type__")) {
        return false;
    }

    if (not record.hasRecordSlot("__guid__")) {
        return false;
    }

    /**/

    asset_type_id typeId {};
    asset_guid guid = invalid_asset_guid;

    serialization::access::Structure<Guid>::deserialize(&guid, record.getRecordSlot("__guid__"));
    record.getSlot<u64>("__type__") >> typeId.data;

    /**/

    if (guid == invalid_asset_guid || typeId.data == 0) {
        return false;
    }

    // TODO: Lookup type id

    return true;
}

bool tryLoadArchivedAsset(mref<serialization::RecordScopedSlot> record_) {

    const auto record = record_.asStruct();

    /**/

    asset_type_id typeId {};
    asset_guid guid = invalid_asset_guid;

    serialization::access::Structure<Guid>::deserialize(&guid, record.getRecordSlot("__guid__"));
    record.getSlot<u64>("__type__") >> typeId.data;

    /**/

    switch (typeId.data) {
        case assets::Image::typeId.data: {

            auto* const registry = Engine::getEngine()->getAssets()->getRegistry();
            const auto* const asset = registry->findAssetByGuid(guid);

            if (asset != nullptr) {
                return false;
            }

            /**/

            auto* image = new assets::Image();
            serialization::access::Structure<assets::Image>::deserialize(image, _STD move(record_));

            engine::assets::storeDefaultNameAndUrl(image, {});
            const auto succeeded = registry->insert({ image });

            if (not succeeded) {

                IM_CORE_WARNF(
                    "Tried to load Image (`{}` -> `{}`) from package, but it is already present.",
                    image->getAssetName(),
                    encodeGuid4228(guid)
                );

                __debugbreak();
                delete image;

            } else {
                IM_CORE_LOGF(
                    "Successfully loaded Image (`{}` -> `{}`) from package.",
                    image->getAssetName(),
                    encodeGuid4228(guid)
                );
            }

            /**/

            break;
        }
        case assets::TextureAsset::typeId.data: {

            auto* const registry = Engine::getEngine()->getAssets()->getRegistry();
            const auto* const asset = registry->findAssetByGuid(guid);

            if (asset != nullptr) {
                return false;
            }

            /**/

            auto* texture = new assets::TextureAsset();
            serialization::access::Structure<assets::TextureAsset>::deserialize(texture, _STD move(record_));

            engine::assets::storeDefaultNameAndUrl(texture, {});
            const auto succeeded = registry->insert({ texture });

            if (not succeeded) {

                IM_CORE_WARNF(
                    "Tried to load Texture (`{}` -> `{}`) from package, but it is already present.",
                    texture->getAssetName(),
                    encodeGuid4228(guid)
                );

                __debugbreak();
                delete texture;

            } else {
                IM_CORE_LOGF(
                    "Successfully loaded Texture (`{}` -> `{}`) from package.",
                    texture->getAssetName(),
                    encodeGuid4228(guid)
                );
            }

            /**/

            break;
        }
        case assets::StaticGeometry::typeId.data: {

            auto* const registry = Engine::getEngine()->getAssets()->getRegistry();
            const auto* const asset = registry->findAssetByGuid(guid);

            if (asset != nullptr) {
                return false;
            }

            /**/

            auto* geom = new assets::StaticGeometry();
            serialization::access::Structure<assets::StaticGeometry>::deserialize(geom, _STD move(record_));

            engine::assets::storeDefaultNameAndUrl(geom, {});
            const auto succeeded = registry->insert({ geom });

            if (not succeeded) {

                IM_CORE_WARNF(
                    "Tried to load Static Geometry (`{}` -> `{}`) from package, but it is already present.",
                    geom->getAssetName(),
                    encodeGuid4228(guid)
                );

                __debugbreak();
                delete geom;

            } else {
                IM_CORE_LOGF(
                    "Successfully loaded Static Geometry (`{}` -> `{}`) from package.",
                    geom->getAssetName(),
                    encodeGuid4228(guid)
                );
            }

            /**/

            break;
        }
    }
}
