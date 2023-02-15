#include <filesystem>
#include <regex>
#include <sstream>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Core/Engine.hpp>

#include "ImageImporter.hpp"
#include "../API/VkTranslate.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Serialization/Access/Structure.hpp"
#include "Engine.Serialization/Archive/StructuredArchive.hpp"

using namespace ember::engine::gfx;
using namespace ember;

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

bool KtxImporter::canImport(cref<res::FileTypeId> typeId_, cref<fs::File> file_) const noexcept {
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

    ::ember::concurrent::promise<KtxImporter::import_type> prom {
        [value = _STD move(value_)]() {
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

    const _STD regex formatReg { R"(^(.*)(_(png|jpg|exr|tif|ktx|ktx2|hdr)+)+)" };
    // :: cobblestone_large_01_nor_8k_png -> cobblestone_large_01_nor_8k
    const _STD regex sizeReg { R"(^(.*)(_[1|2|4|8|16|24|32]+k)+)" };
    // :: cobblestone_large_01_nor_8k -> cobblestone_large_01_nor
    const _STD regex typeReg { R"(^(.*)(_[nor|rough|dif|diff|arm|spec|metal]+)+)" };
    // :: cobblestone_large_01_nor -> cobblestone_large_01

    string tmp0 = _STD regex_replace(value_, formatReg, "$1");
    string tmp1 = _STD regex_replace(tmp0, sizeReg, "$1");
    tmp0 = _STD regex_replace(tmp1, typeReg, "$1");

    return tmp0;
}

KtxImporter::import_result_type KtxImporter::import(cref<res::FileTypeId> typeId_, cref<fs::File> file_) const {

    const auto rootCwd { _STD filesystem::current_path().append(R"(..\..)") };
    const auto rootAssetPath { _STD filesystem::path(R"(resources\assets\texture)") };
    const auto rootImportPath { _STD filesystem::path(R"(resources\imports)") };

    /**/

    Url targetUrl { "", rootAssetPath.string() };

    // TODO:
    const auto sourcePath { file_.path() };
    const auto sourceFileName { sourcePath.filename().string() };
    const auto sourceExt { sourcePath.extension().string() };
    const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

    const bool isKtx2 { sourceExt.contains("ktx2") };

    /**/
    const auto targetSubDir { normalizeDirName(sourceName) };
    const auto targetDirPath { _STD filesystem::path(targetUrl.path()).append(targetSubDir) };

    const auto subRelativePath { _STD filesystem::relative(targetDirPath, rootAssetPath) };

    const auto storePath {
        _STD filesystem::path { rootCwd }
        .append(rootImportPath.string())
        .append(isKtx2 ? R"(ktx2)" : R"(ktx)")
        .append(subRelativePath.string())
        .append(sourceFileName)
    };

    /**/

    if (/* _STD filesystem::exists(targetDirPath) || */_STD filesystem::exists(storePath)) {
        return makeImportResult({ nullptr, nullptr });
    }

    /**/

    _STD filesystem::create_directories(storePath.parent_path());
    _STD filesystem::copy(sourcePath, storePath);

    /**/

    using namespace ::ember::engine::serialization;
    using namespace ::ember::engine::assets;

    auto& factory { *Engine::getEngine()->getAssets()->getFactory() };

    auto* imgAsset { factory.createImageAsset() };
    auto* img { static_cast<ptr<Image>>(imgAsset) };

    auto texAsset { factory.createTextureAsset() };
    auto* tex { static_cast<ptr<Texture>>(texAsset) };

    /**/

    const auto imgSrcPath { _STD filesystem::relative(storePath, rootCwd) };
    img->setAssetName(sourceName);
    img->addSource(Url { "file"sv, imgSrcPath.string() });

    /**/

    _STD ifstream ifs { sourcePath, _STD ios::in | _STD ios::binary };
    assert(ifs);

    ifs.seekg(0, _STD ios::beg);
    const auto beg { ifs.tellg() };

    constexpr auto mem_block_size { 4096ui64 };
    _STD vector<char> raw(mem_block_size);

    ifs.read(raw.data(), mem_block_size);
    const auto end { ifs.tellg() };

    ifs.close();
    const auto readSize { end - beg };

    constexpr auto headerSize { sizeof(InternalKtxHeader) };
    assert(readSize >= headerSize + /* Magic Offset */12);

    auto* cursor { raw.data() + 12ui64 };
    cref<InternalKtxHeader> header { *reinterpret_cast<ptr<const InternalKtxHeader>>(cursor) };

    /**/

    tex->setAssetName(sourceName);
    tex->setBaseImage(img->get_guid());

    tex->setExtent(
        {
            static_cast<u32>(header.pixelWidth),
            _STD max(static_cast<u32>(header.pixelHeight), 1ui32),
            _STD max(static_cast<u32>(header.pixelDepth), 1ui32)
        }
    );

    tex->setTextureFormat(api::vkTranslateFormat(*reinterpret_cast<const vk::Format*>(&header.vkFormat)));
    tex->setMipLevelCount(_STD max(header.levelCount, 1ui32));

    // TODO: Replace with actual/correct type resolving for textures
    if (header.faceCount <= 1ui32) {
        tex->setTextureType(TextureType::e2d);
    } else if (header.faceCount == 6ui32) {
        tex->setTextureType(TextureType::eCube);
    } else if (header.faceCount > 1ui32) {
        tex->setTextureType(TextureType::e2dArray);
    }

    /**/

    #define EDITOR TRUE
    #ifdef EDITOR

    BufferArchive imgBuffer {};
    StructuredArchive imgArch { &imgBuffer };

    {
        auto root = imgArch.insertRootSlot();
        access::Structure<Image>::serialize(img, _STD move(root));
    }

    #else

    auto imgLayout { make_sptr<DataLayout<Image>>() };

    imgLayout->reflect().storeType<Image>();
    imgLayout->describe();

    BufferArchive imgBuffer {};
    LayoutArchive<DataLayout<Image>> imgArch { &imgBuffer, imgLayout.get() };

    imgArch << img;

    #endif

    const auto imgPath {
        _STD filesystem::path { rootCwd }.append(targetDirPath.string()).append(sourceName).concat(R"(.img.imasset)")
    };

    _STD filesystem::create_directories(imgPath.parent_path());
    _STD ofstream ofs {
        imgPath,
        _STD ios::out | _STD ios::binary
    };
    ofs.seekp(0, _STD ios::beg);

    ofs.write(reinterpret_cast<char*>(imgBuffer.data()), imgBuffer.totalSize());

    ofs.flush();
    ofs.close();

    /**/

    #ifdef EDITOR

    BufferArchive texBuffer {};
    StructuredArchive texArch { &texBuffer };

    {
        auto root = texArch.insertRootSlot();
        access::Structure<Texture>::serialize(tex, _STD move(root));
    }

    #else

    auto texLayout { make_sptr<DataLayout<Texture>>() };

    texLayout->reflect().storeType<Texture>();
    texLayout->describe();

    BufferArchive texBuffer {};
    LayoutArchive<DataLayout<Texture>> texArch { &texBuffer, texLayout.get() };

    texArch << tex;

    #endif

    const auto texPath {
        _STD filesystem::path { rootCwd }.append(targetDirPath.string()).append(sourceName).concat(R"(.imasset)")
    };

    _STD filesystem::create_directories(texPath.parent_path());
    ofs = _STD ofstream {
        texPath,
        _STD ios::out | _STD ios::binary
    };
    ofs.seekp(0, _STD ios::beg);

    ofs.write(reinterpret_cast<char*>(texBuffer.data()), texBuffer.totalSize());

    ofs.flush();
    ofs.close();

    /**/
    return makeImportResult({ tex, img });
}
