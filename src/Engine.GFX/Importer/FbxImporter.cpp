#include "ModelImporter.hpp"

#include <filesystem>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Resource.Package/Linker/PackageLinker.hpp>
#include <Engine.Storage.Registry/IStorageRegistry.hpp>
#include <Engine.Storage.Registry/Storage/PackageStorage.hpp>

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
		[value = std::move(value_)]() {
			return value;
		}
	};

	auto result { prom.get() };
	prom();

	return result;
}

FbxImporter::FbxImporter() {}

FbxImporter::~FbxImporter() {}

bool FbxImporter::canImport(cref<res::FileTypeId> typeId_, cref<hg::fs::File> file_) const noexcept {

	if (typeId_.ext != ".fbx" && typeId_.ext != ".obj") {
		return false;
	}

	if (not
		file_.exists()
	) {
		return false;
	}

	return true;
}

FbxImporter::descriptor_type FbxImporter::descriptor() const noexcept {
	return nullptr;
}

FbxImporter::import_result_type FbxImporter::import(
	ref<const res::FileTypeId> typeId_,
	ref<const hg::fs::File> file_,
	mref<res::ImportDestination> destination_
) const {

	/**/

	const auto sourceFileName { static_cast<cref<std::filesystem::path>>(file_.path()).filename().string() };
	const auto sourceExt { static_cast<cref<std::filesystem::path>>(file_.path()).extension().string() };
	const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

	/**/

	const auto data = assimpGetImportData(file_);
	if (data.indexCount == 0 && data.vertexCount == 0) {
		return makeImportResult({ nullptr });
	}

	/**/

	using namespace ::hg::engine::serialization;
	using namespace ::hg::engine::assets;

	/**/

	auto materials = AutoArray<AssetGuid> {};
	materials.resize(data.materialCount, AssetGuid {});

	auto asset = Arci<StaticGeometry>::create(
		generate_asset_guid(),
		StringView { sourceName },
		/* TODO: Check whether the importer should already locate and seal the storage used for the asset */
		AssetReferenceUrl {},
		AssetUrl { AssetPath { ::hg::move(destination_).virtualBasePath }, AssetName { sourceName } },
		Vector<fs::Url> { { storage::FileScheme, clone(file_.path()) } },
		::hg::move(materials)
	);

	IM_CORE_LOGF(
		"Created new static geometry asset `{}` -> `{}`",
		asset->getAssetName(),
		encodeGuid4228(asset->getAssetGuid())
	);

	/**/

	return makeImportResult(::hg::move(asset));
}

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

FbxAssimpImportData assimpGetImportData(cref<fs::File> file_) {

	Assimp::Importer importer {};
	constexpr u32 ppFlags = { aiProcess_PreTransformVertices | aiProcess_Triangulate };

	/**/

	const streamsize size = file_.size();
	if (size < 0) {
		::hg::breakpoint();
		return {};
	}

	auto* const scene = importer.ReadFile(static_cast<std::string>(file_.path()), ppFlags);

	if (scene == nullptr) {
		::hg::breakpoint();
		return {};
	}

	/**/

	FbxAssimpImportData data { 0uLL, 0uLL, 0uLL };

	auto** meshes = scene->mMeshes;
	const auto meshCount = scene->mNumMeshes;

	for (auto meshIdx = 0; meshIdx < meshCount; ++meshIdx) {

		const auto* const mesh = meshes[meshIdx];
		const auto fc = mesh->mNumFaces;
		const auto vc = mesh->mNumVertices;

		data.indexCount += fc * 3uLL;
		data.vertexCount += vc;
	}

	data.materialCount = scene->mNumMaterials;

	/**/

	return data;
}