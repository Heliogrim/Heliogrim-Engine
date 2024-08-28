#include <cstring>
#include <filesystem>
#include <regex>
#include <sstream>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>

#include "ImageImporter.hpp"
#include "../API/VkTranslate.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Common/Optional.hpp"
#include "Engine.Storage/IStorageRegistry.hpp"
#include "Engine.Storage/Options/StorageDescriptor.hpp"
#include "Engine.Storage/Storage/PackageStorage.hpp"
#include "Engine.Storage/Url/FileUrl.hpp"

/**/
#include <Engine.Common/GLM.hpp>
#include <gli/load_ktx.hpp>
#include <gli/texture.hpp>

using namespace hg::engine::gfx;
using namespace hg;

/**/

static Opt<vk::Format> deduceFromFormat(cref<gli::format> format_);

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

struct InternalKtx11Header {
	u32 endianess;
	u32 glType;
	u32 glTypeSize;
	u32 glFormat;
	u32 glInternalFormat;
	u32 glBaseInternalFormat;
	u32 pixelWidth;
	u32 pixelHeight;
	u32 pixelDepth;
	u32 arrayCount;
	u32 faceCount;
	u32 mipLevelCount;
	u32 kvdSize;
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

	constexpr auto headerSize {
		MAX(sizeof(ktx20Identifier), sizeof(gli::detail::FOURCC_KTX10)) +
		MAX(sizeof(InternalKtxHeader), sizeof(InternalKtx11Header))
	};
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

[[maybe_unused]] static string normalizeDirName(cref<string> value_) {

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

	/**/

	const auto sourceFileName { static_cast<cref<std::filesystem::path>>(file_.path()).filename().string() };
	const auto sourceExt { static_cast<cref<std::filesystem::path>>(file_.path()).extension().string() };
	const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

	/**/

	using namespace ::hg::engine::serialization;
	using namespace ::hg::engine::assets;

	auto& factory { *Engine::getEngine()->getAssets()->getFactory() };

	auto imgAsset { factory.createImageAsset() };
	auto& image = *imgAsset;

	auto texAsset { factory.createTextureAsset() };
	auto& texture = *texAsset;

	/**/

	// TODO: Remap the source file url.
	image.setAssetName(sourceName);
	image.addSource(fs::Url { "file"sv, file_.path() });

	IM_CORE_LOGF(
		"Created new image asset `{}` -> `{}`",
		imgAsset->getAssetName(),
		encodeGuid4228(imgAsset->get_guid())
	);

	/**/

	std::ifstream ifs { static_cast<cref<std::filesystem::path>>(file_.path()), std::ios::in | std::ios::binary };
	assert(ifs);

	ifs.seekg(0, std::ios::beg);

	char internalIdentifier[sizeof(ktx20Identifier)] {};
	ifs.read(internalIdentifier, sizeof(internalIdentifier));

	if (std::memcmp(internalIdentifier, ktx20Identifier, sizeof(ktx20Identifier)) == 0) {

		InternalKtxHeader internalHeader {};
		ifs.read(std::bit_cast<char*>(&internalHeader), sizeof(internalHeader));
		::hg::assertrt(not ifs.fail() && not ifs.bad());

		/**/

		texture.setAssetName(sourceName);
		texture.setBaseImage(image.get_guid());

		texture.setExtent(
			{
				static_cast<u32>(internalHeader.pixelWidth),
				std::max(static_cast<u32>(internalHeader.pixelHeight), 1u),
				std::max(static_cast<u32>(internalHeader.pixelDepth), 1u)
			}
		);

		texture.setTextureFormat(
			api::vkTranslateFormat(*reinterpret_cast<const vk::Format*>(&internalHeader.vkFormat))
		);
		texture.setMipLevelCount(std::max(internalHeader.levelCount, 1u));

		// TODO: Replace with actual/correct type resolving for textures
		if (internalHeader.faceCount <= 1uL) {
			texture.setTextureType(TextureType::e2d);
		} else if (internalHeader.faceCount == 6uL) {
			texture.setTextureType(TextureType::eCube);
		} else if (internalHeader.faceCount > 1uL) {
			texture.setTextureType(TextureType::e2dArray);
		}

	} else if (std::memcmp(internalIdentifier, gli::detail::FOURCC_KTX10, sizeof(gli::detail::FOURCC_KTX10)) == 0) {

		InternalKtx11Header internalHeader {};
		ifs.read(std::bit_cast<char*>(&internalHeader), sizeof(internalHeader));
		::hg::assertrt(not ifs.fail() && not ifs.bad());

		/**/

		gli::texture glitex = gli::load_ktx(static_cast<std::string>(file_.path()));

		/**/

		texture.setAssetName(sourceName);
		texture.setBaseImage(image.get_guid());

		const auto lvlZeroExt = glitex.extent(0);
		texture.setExtent(
			{
				static_cast<u32>(lvlZeroExt.x),
				std::max(static_cast<u32>(lvlZeroExt.y), 1u),
				std::max(static_cast<u32>(lvlZeroExt.z), 1u)
			}
		);

		const auto lvlZeroForm = glitex.format();
		const auto format = deduceFromFormat(lvlZeroForm);
		::hg::assertrt(format.has_value());

		texture.setTextureFormat(api::vkTranslateFormat(format.value()));
		texture.setMipLevelCount(std::max(internalHeader.mipLevelCount, 1u));

		// TODO: Replace with actual/correct type resolving for textures
		if (glitex.faces() <= 1uL) {
			texture.setTextureType(TextureType::e2d);
		} else if (glitex.faces() == 6uL) {
			texture.setTextureType(TextureType::eCube);
		} else if (glitex.faces() > 1uL) {
			texture.setTextureType(TextureType::e2dArray);
		}

	} else {
		::hg::panic();
	}

	/**/

	ifs.close();
	IM_CORE_LOGF(
		"Created new texture asset `{}` -> `{}`",
		texAsset->getAssetName(),
		encodeGuid4228(texAsset->get_guid())
	);

	/**/

	return makeImportResult({ std::move(texAsset), std::move(imgAsset) });
}

/**/

Opt<vk::Format> deduceFromFormat(cref<gli::format> format_) {

	switch (format_) {
		/**
		 * R Formats (2D Images)
		 *
		 * Used for alpha, roughness, displacement
		 */
		case gli::FORMAT_R16_SFLOAT_PACK16: {
			return Some(vk::Format::eR16Sfloat);
		}
		case gli::FORMAT_R32_SFLOAT_PACK32: {
			return Some(vk::Format::eR32Sfloat);
		}

		/**
		 * D Formats (2D Images)
		 *
		 * Used for depth images or possible for alpha blending
		 */
		case gli::FORMAT_D16_UNORM_PACK16: {
			return Some(vk::Format::eD16Unorm);
		}
		case gli::FORMAT_D32_SFLOAT_PACK32: {
			return Some(vk::Format::eD32Sfloat);
		}

		/**
		 * DS Formats (2D Images)
		 *
		 * Used for depth stencil images
		 */
		case gli::FORMAT_D16_UNORM_S8_UINT_PACK32: {
			return Some(vk::Format::eD16UnormS8Uint);
		}
		case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64: {
			return Some(vk::Format::eD32SfloatS8Uint);
		}

		/**
		 * RGB Formats (2D Images)
		 *
		 * Used for color textures like albedo, sample maps or general image
		 */
		case gli::FORMAT_RGB8_UNORM_PACK8: {
			return Some(vk::Format::eR8G8B8Unorm);
		}
		case gli::FORMAT_RGB8_SNORM_PACK8: {
			return Some(vk::Format::eR8G8B8Snorm);
		}
		case gli::FORMAT_RGB16_UNORM_PACK16: {
			return Some(vk::Format::eR16G16B16A16Unorm);
		}
		case gli::FORMAT_RGB32_UINT_PACK32: {
			return Some(vk::Format::eR32G32B32Uint);
		}
		case gli::FORMAT_RGB32_SFLOAT_PACK32: {
			return Some(vk::Format::eR32G32B32Sfloat);
		}

		/**
		* RGBA Formats (2D Images)
		*
		* Used for color textures with alpha like albedo + blending or sample maps
		*/
		case gli::FORMAT_RGBA8_UNORM_PACK8: {
			return Some(vk::Format::eR8G8B8A8Unorm);
		}
		case gli::FORMAT_RGBA8_SRGB_PACK8: {
			return Some(vk::Format::eR8G8B8A8Srgb);
		}
		case gli::FORMAT_RGBA8_SNORM_PACK8: {
			return Some(vk::Format::eR8G8B8A8Snorm);
		}
		case gli::FORMAT_RGBA16_UNORM_PACK16: {
			return Some(vk::Format::eR16G16B16A16Unorm);
		}
		case gli::FORMAT_RGBA16_SFLOAT_PACK16: {
			return Some(vk::Format::eR16G16B16A16Sfloat);
		}
		case gli::FORMAT_RGBA32_UINT_PACK32: {
			return Some(vk::Format::eR32G32B32A32Uint);
		}
		case gli::FORMAT_RGBA32_SFLOAT_PACK32: {
			return Some(vk::Format::eR32G32B32A32Sfloat);
		}

		/**
		* RGB(A) Formats (Compressed Cube Images)
		*/
		case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16: {
			return Some(vk::Format::eAstc8x8UnormBlock);
		}
		case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8: {
			return Some(vk::Format::eEtc2R8G8B8UnormBlock);
		}
		case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16: {
			// TODO: Temporary
			return Some(vk::Format::eBc2UnormBlock);
		}
		default: {
			return None;
		}
	}
}
