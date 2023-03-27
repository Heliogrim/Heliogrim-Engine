#include <filesystem>
#include <regex>
#include <sstream>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Serialization/Archive/Archive.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Core/Engine.hpp>

#include "ModelImporter.hpp"
#include "../API/VkTranslate.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Resource/Source/FileSource.hpp"
#include "Engine.Resource.Package/PackageFactory.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Access/Structure.hpp"
#include "Engine.Serialization/Archive/StructuredArchive.hpp"

using namespace hg::engine::gfx;
using namespace hg;

struct FbxAssimpImportData {
    u64 indexCount;
    u64 vertexCount;
    u32 materialCount;
};

static FbxAssimpImportData assimpGetImportData(cref<fs::File> file_);

/**/

static FbxImporter::import_result_type makeImportResult(mref<FbxImporter::import_type> value_) {

    ::hg::concurrent::promise<FbxImporter::import_type> prom {
        [value = _STD move(value_)]() {
            return value;
        }
    };

    auto result { prom.get() };
    prom();

    return result;
}

FbxImporter::FbxImporter() { }

FbxImporter::~FbxImporter() { }

bool FbxImporter::canImport(cref<res::FileTypeId> typeId_, cref<hg::fs::File> file_) const noexcept {

    if (typeId_.ext != ".fbx") {
        return false;
    }

    if (not file_.exists()) {
        return false;
    }

    return true;
}

FbxImporter::descriptor_type FbxImporter::descriptor() const noexcept {
    return nullptr;
}

FbxImporter::import_result_type FbxImporter::import(cref<res::FileTypeId> typeId_, cref<hg::fs::File> file_) const {

    const auto rootCwd { _STD filesystem::current_path().append(R"(..\..)") };
    const auto rootAssetPath { _STD filesystem::path(R"(resources\assets\geometry)") };
    const auto rootImportPath { _STD filesystem::path(R"(resources\imports)") };

    /**/

    fs::Url targetUrl { fs::Path { rootAssetPath } };

    /**/

    const auto sourcePath { file_.path() };
    const auto sourceFileName { sourcePath.filename().string() };
    const auto sourceExt { sourcePath.extension().string() };
    const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

    /**/
    const auto targetSubDir { sourceName };
    const auto targetDirPath { _STD filesystem::path(targetUrl.path()).append(targetSubDir) };

    const auto subRelativePath { _STD filesystem::relative(targetDirPath, rootAssetPath) };

    const auto storePath {
        _STD filesystem::path { rootCwd }
        .append(rootImportPath.string())
        .append(R"(fbx)")
        .append(subRelativePath.string())
        .append(sourceFileName)
    };

    /**/

    if (/* _STD filesystem::exists(targetDirPath) || */_STD filesystem::exists(storePath)) {
        return makeImportResult({ nullptr });
    }

    /**/

    _STD filesystem::create_directories(storePath.parent_path());
    _STD filesystem::copy(sourcePath, storePath);

    /**/

    const auto data = assimpGetImportData(file_);
    if (data.indexCount == 0 && data.vertexCount == 0) {
        return makeImportResult({ nullptr });
    }

    const auto srcPath = _STD filesystem::relative(storePath, rootCwd);

    /**/

    using namespace ::hg::engine::serialization;
    using namespace ::hg::engine::assets;

    auto& factory { *Engine::getEngine()->getAssets()->getFactory() };

    auto guid = generate_asset_guid();
    auto* asset = factory.createStaticGeometryAsset(
        guid,
        srcPath.string(),
        data.vertexCount,
        data.indexCount
    );

    /**/

    auto* geom = static_cast<ptr<StaticGeometry>>(asset);
    geom->setAssetName(sourceName);

    /**/

    auto memBuffer = make_uptr<BufferArchive>();
    auto arch = StructuredArchive(memBuffer.get());

    {
        auto root = arch.insertRootSlot();
        access::Structure<StaticGeometry>::serialize(geom, _STD move(root));
    }

    /**/

    Guid archGuid {};
    GuidGenerate(archGuid);

    /**/

    const auto packagePath = _STD filesystem::path(rootCwd).append(targetDirPath.string()).append(sourceName).concat(
        R"(.impackage)"
    );

    if (not _STD filesystem::exists(packagePath.parent_path())) {
        _STD filesystem::create_directories(packagePath.parent_path());
    }

    /**/

    hg::fs::File packageFile { packagePath };
    auto source = make_uptr<resource::FileSource>(_STD move(packageFile));

    auto package = resource::PackageFactory::createEmptyPackage(_STD move(source));
    auto* const linker = package->getLinker();

    /**/

    linker->store(
        resource::ArchiveHeader { resource::ArchiveHeaderType::eSerializedStructure, _STD move(archGuid) },
        _STD move(memBuffer)
    );

    /**/

    package->unsafeWrite();

    /**/

    return makeImportResult(_STD move(geom));
}

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

FbxAssimpImportData assimpGetImportData(cref<fs::File> file_) {

    Assimp::Importer importer {};
    const u32 ppFlags = { aiProcess_PreTransformVertices };

    /**/

    const streamsize size = file_.size();
    if (size < 0) {
        __debugbreak();
        return {};
    }

    auto* const scene = importer.ReadFile(file_.path().string(), ppFlags);

    if (scene == nullptr) {
        __debugbreak();
        return {};
    }

    /**/

    FbxAssimpImportData data { 0ui64, 0ui64, 0ui64 };

    auto** meshes = scene->mMeshes;
    const auto meshCount = scene->mNumMeshes;

    for (auto meshIdx = 0; meshIdx < meshCount; ++meshIdx) {

        const auto* const mesh = meshes[meshIdx];
        const auto fc = mesh->mNumFaces;
        const auto vc = mesh->mNumVertices;

        data.indexCount += fc * 3ui64;
        data.vertexCount += vc;
    }

    data.materialCount = scene->mNumMaterials;

    /**/

    return data;
}
