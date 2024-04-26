#include <filesystem>
#include <regex>
#include <sstream>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Serialization/Archive/Archive.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>

#include "ImageImporter.hpp"
#include "../API/VkTranslate.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Resource/Source/FileSource.hpp"
#include "Engine.Resource.Package/PackageFactory.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Access/Structure.hpp"
#include "Engine.Serialization/Archive/StructuredArchive.hpp"

using namespace hg::engine::gfx;
using namespace hg;

/**/
#pragma region KTX Format Specification
struct InternalKtxHeader {
	// Meta
	u32 vkFormat;
	u32 typeSize;
	u32 pixelWidth;
	u32 pixelHeight;
	u32 pixelDepth;
	u32 layerCount;
	u32 faceCount;
	u32 levelCount;
	u32 ss;

	// Indices
	u32 dfdOff;
	u32 dfdSize;
	u32 kvdOff;
	u32 kvdSize;
	u32 sgdOff;
	u32 sgdSize;
};

struct InternalKtxLevel {
	u64 byteOff;
	u64 byteSize;
	u64 ucByteSize;
};

static constexpr unsigned char ktx20Identifier[] = {
	0xAB,
	0x4B,
	0x54,
	0x58,
	0x20,
	0x32,
	0x30,
	0xBB,
	0x0D,
	0x0A,
	0x1A,
	0x0A
};
#pragma endregion
/**/

KtxImporter::KtxImporter() {}

KtxImporter::~KtxImporter() {}

bool KtxImporter::canImport(cref<res::FileTypeId> typeId_, cref<hg::fs::File> file_) const noexcept {
	if (typeId_.ext != ".ktx" && typeId_.ext != ".ktx2") {
		return false;
	}

	if (not file_.exists()) {
		return false;
	}

	constexpr auto headerSize { sizeof(ktx20Identifier) + sizeof(InternalKtxHeader) };
	if (file_.size() < headerSize) {
		return false;
	}

	return true;
}

KtxImporter::descriptor_type KtxImporter::descriptor() const noexcept {
	return nullptr;
}

static KtxImporter::import_result_type makeImportResult(mref<KtxImporter::import_type> value_) {

	::hg::concurrent::promise<KtxImporter::import_type> prom {
		[value = std::move(value_)]() {
			return value;
		}
	};

	auto result { prom.get() };
	prom();

	return result;
}

static string normalizeDirName(cref<string> value_) {

	// sandstone_blocks_05_8k_png
	// cobblestone_large_01_8k_png
	// cobblestone_large_01_nor_8k
	// cobblestone_large_01_rough_8k

	const std::regex formatReg { R"(^(.*)(_(png|jpg|jpeg|exr|tif|tiff|ktx|ktx2|hdr)+)+)" };
	// :: cobblestone_large_01_nor_8k_png -> cobblestone_large_01_nor_8k
	const std::regex sizeReg { R"(^(.*)(_[1|2|4|8|16|24|32]+k)+)" };
	// :: cobblestone_large_01_nor_8k -> cobblestone_large_01_nor
	const std::regex typeReg { R"(^(.*)(_[nor|normal|rough|dif|diff|arm|spec|metal]+)+)" };
	// :: cobblestone_large_01_nor -> cobblestone_large_01

	string tmp0 = std::regex_replace(value_, formatReg, "$1");
	string tmp1 = std::regex_replace(tmp0, sizeReg, "$1");
	tmp0 = std::regex_replace(tmp1, typeReg, "$1");

	return tmp0;
}

KtxImporter::import_result_type KtxImporter::import(cref<res::FileTypeId> typeId_, cref<hg::fs::File> file_) const {

	const auto rootCwd { std::filesystem::current_path().append(R"(..\..)") };
	const auto rootAssetPath { std::filesystem::path(R"(resources\assets\texture)") };
	const auto rootImportPath { std::filesystem::path(R"(resources\imports)") };

	/**/

	fs::Url targetUrl { fs::Path { rootAssetPath } };

	// TODO:
	const auto sourcePath { file_.path() };
	const auto sourceFileName { sourcePath.filename().string() };
	const auto sourceExt { sourcePath.extension().string() };
	const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

	const bool isKtx2 { sourceExt.contains("ktx2") };

	/**/
	const auto targetSubDir { normalizeDirName(sourceName) };
	const auto targetDirPath { std::filesystem::path(targetUrl.path()).append(targetSubDir) };

	const auto subRelativePath { std::filesystem::relative(targetDirPath, rootAssetPath) };

	const auto storePath {
		std::filesystem::path { rootCwd }
		.append(rootImportPath.string())
		.append(isKtx2 ? R"(ktx2)" : R"(ktx)")
		.append(subRelativePath.string())
		.append(sourceFileName)
	};

	/**/

	if (/* std::filesystem::exists(targetDirPath) || */std::filesystem::exists(storePath)) {
		return makeImportResult({ nullptr, nullptr });
	}

	/**/

	IM_CORE_LOGF("Copying file to {:}", storePath.string());
	std::filesystem::create_directories(storePath.parent_path());
	std::filesystem::copy(sourcePath, storePath);

	/**/

	using namespace ::hg::engine::serialization;
	using namespace ::hg::engine::assets;

	auto& factory { *Engine::getEngine()->getAssets()->getFactory() };

	auto imgAsset { factory.createImageAsset() };
	auto* img { static_cast<ptr<Image>>(imgAsset.get()) };

	auto texAsset { factory.createTextureAsset() };
	auto* tex { static_cast<ptr<TextureAsset>>(texAsset.get()) };

	/**/

	const auto imgSrcPath { std::filesystem::relative(storePath, rootCwd) };
	img->setAssetName(sourceName);
	img->addSource(fs::Url { "file"sv, imgSrcPath.string() });

	IM_CORE_LOGF(
		"Created new image asset `{}` -> `{}`",
		imgAsset->getAssetName(),
		encodeGuid4228(imgAsset->get_guid())
	);

	/**/

	std::ifstream ifs { sourcePath, std::ios::in | std::ios::binary };
	assert(ifs);

	ifs.seekg(0, std::ios::beg);
	const auto beg { ifs.tellg() };

	constexpr auto mem_block_size { 4096uLL };
	std::vector<char> raw(mem_block_size);

	ifs.read(raw.data(), mem_block_size);
	const auto end { ifs.tellg() };

	ifs.close();
	const auto readSize { end - beg };

	constexpr auto headerSize { sizeof(InternalKtxHeader) };
	assert(readSize >= headerSize + /* Magic Offset */12);

	auto* cursor { raw.data() + 12uLL };
	cref<InternalKtxHeader> header { *reinterpret_cast<ptr<const InternalKtxHeader>>(cursor) };

	/**/

	tex->setAssetName(sourceName);
	tex->setBaseImage(img->get_guid());

	tex->setExtent(
		{
			static_cast<u32>(header.pixelWidth),
			std::max(static_cast<u32>(header.pixelHeight), 1u),
			std::max(static_cast<u32>(header.pixelDepth), 1u)
		}
	);

	tex->setTextureFormat(api::vkTranslateFormat(*reinterpret_cast<const vk::Format*>(&header.vkFormat)));
	tex->setMipLevelCount(std::max(header.levelCount, 1u));

	// TODO: Replace with actual/correct type resolving for textures
	if (header.faceCount <= 1uL) {
		tex->setTextureType(TextureType::e2d);
	} else if (header.faceCount == 6uL) {
		tex->setTextureType(TextureType::eCube);
	} else if (header.faceCount > 1uL) {
		tex->setTextureType(TextureType::e2dArray);
	}

	IM_CORE_LOGF(
		"Created new texture asset `{}` -> `{}`",
		texAsset->getAssetName(),
		encodeGuid4228(texAsset->get_guid())
	);

	/**/

	#define EDITOR TRUE

	#ifdef EDITOR

	/* Serialize Image */

	auto imgBuffer = make_uptr<BufferArchive>();
	StructuredArchive imgArch { imgBuffer.get() }; {
		auto root = imgArch.insertRootSlot();
		access::Structure<Image>::serialize(img, std::move(root));
	}

	Guid imgArchGuid {};
	GuidGenerate(imgArchGuid);

	/* Serialize Texture */

	auto texBuffer = make_uptr<BufferArchive>();
	StructuredArchive texArch { texBuffer.get() }; {
		auto root = texArch.insertRootSlot();
		access::Structure<TextureAsset>::serialize(tex, std::move(root));
	}

	Guid texArchGuid {};
	GuidGenerate(texArchGuid);

	/* Generate Target Path */

	const auto packagePath {
		std::filesystem::path { rootCwd }.append(targetDirPath.string()).append(sourceName).concat(R"(.impackage)")
	};

	if (not std::filesystem::exists(packagePath.parent_path())) {
		std::filesystem::create_directories(packagePath.parent_path());
	}

	/* Generate Package */

	hg::fs::File packageFile { packagePath };
	auto source = make_uptr<resource::FileSource>(std::move(packageFile));

	auto package = resource::PackageFactory::createEmptyPackage(std::move(source));
	auto* const linker = package->getLinker();

	/* Store Data to Package */

	linker->store(
		resource::ArchiveHeader { resource::ArchiveHeaderType::eSerializedStructure, std::move(imgArchGuid) },
		std::move(imgBuffer)
	);

	linker->store(
		resource::ArchiveHeader { resource::ArchiveHeaderType::eSerializedStructure, std::move(texArchGuid) },
		std::move(texBuffer)
	);

	/* Flush/Write the package */

	package->unsafeWrite();

	#endif

	/**/
	return makeImportResult({ tex, img });
}
