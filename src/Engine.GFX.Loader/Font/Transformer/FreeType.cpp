#include "FreeType.hpp"

#include <filesystem>
#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Math/Compat.inl>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Command/CommandPool.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.Logging/Logger.hpp>

/**/
#include <ft2build.h>

#include FT_FREETYPE_H
/**/

using namespace hg::engine::gfx::loader::transformer;
using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

static std::atomic_uint_fast8_t ftRefCount {};
static FT_Library freeTypeLibrary {};
static DenseMap<asset_guid, FT_Face> freeTypeFaces {};

constexpr static math::uivec2 font_texture_padding { 1uL };

/**/

static math::uivec2 getFontExtent(cref<engine::assets::Font> font_, cref<FontLoadOptions> options_);

static void writeToMemory(
	cref<FT_GlyphSlot> slot_,
	cref<math::uivec2> ext_,
	cref<math::uivec2> off_,
	_Inout_ ptr<u8> dst_
);

static void storeFontToTexture(
	nmpt<FontResource::value_type> font_,
	cref<FontLoadOptions> options_,
	cref<math::uivec2> report_,
	_Inout_ ptr<u8> dst_
);

/**/

void transformer::convertFreeType(
	const non_owning_rptr<const assets::Font> assets_,
	mref<storage::AccessBlobReadonly> src_,
	nmpt<reflow::Font> dst_,
	cref<sptr<Device>> device_,
	const FontLoadOptions options_
) {

	// Problem: We may have to release the process lock of the file to enable freetype to get access...
	discard(std::move(src_));

	// Problem: We have to dynamically manage fonts within the FreeType setup
	if (not freeTypeFaces.contains(assets_->get_guid())) {
		initFaceFromAsset(*assets_);
	}

	/**/

	const math::uivec2 repExt = getFontExtent(*assets_, options_);
	math::uivec2 reqExt = repExt;

	/**/

	if (repExt.x > 512) {

		const auto coverSize = repExt.x * repExt.y;
		const auto expCoverSize = static_cast<float>(coverSize) * 1.2F;
		const auto expCoverSqrf = math::sqrtf(expCoverSize);

		u32 nextDim { 0x1 };
		while (nextDim < static_cast<u32>(expCoverSqrf)) {
			nextDim <<= 1;
		}

		reqExt.x = nextDim;
		reqExt.y = nextDim;
	}

	/**/

	const auto* const factory = engine::gfx::TextureFactory::get();
	auto atlas = factory->build(
		TextureBuildPayload {
			math::uivec3 { reqExt, 1uL },
			TextureFormat::eR8G8B8A8Unorm,
			1uL,
			TextureType::e2d,
			vk::ImageAspectFlagBits::eColor,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::SharingMode::eExclusive
		}
	);
	factory->buildView(atlas);

	/**/

	Buffer buffer {};
	buffer.memory = nullptr;
	buffer.buffer = nullptr;
	buffer.device = device_->vkDevice();
	buffer.size = sizeof(u8) * reqExt.x * reqExt.y * 4uL;
	buffer.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

	buffer.buffer = device_->vkDevice().createBuffer(
		{
			vk::BufferCreateFlags(), buffer.size, buffer.usageFlags, vk::SharingMode::eExclusive, 0, nullptr
		}
	);
	assert(buffer.buffer);

	[[maybe_unused]] auto allocRes = memory::allocate(
		*device_->allocator(),
		device_,
		buffer.buffer,
		MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent,
		buffer.memory
	);
	buffer.bind();

	/**/

	const auto font = dst_;
	font->_name = assets_->getAssetName();
	font->_atlas = make_sptr<Texture>(std::move(atlas));
	font->_extent = reqExt;
	font->_fontSize = 16uL;
	font->_glyphCount = 0;

	// Warning: !!Important!!
	font->_ftFace = freeTypeFaces.at(assets_->get_guid());

	/**/

	buffer.map();
	storeFontToTexture(font, options_, repExt, static_cast<ptr<u8>>(buffer.memory->mapping));
	buffer.unmap();

	/**/

	auto* const pool = device_->graphicsQueue()->pool();
	pool->lck().acquire();
	auto cmd { pool->make() };

	cmd.begin();

	/* Pre-transform texture layout */

	Vector<vk::ImageMemoryBarrier> preTrans {};
	preTrans.push_back(
		{
			vk::AccessFlags(),
			vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			font->atlas()->buffer().image(),
			vk::ImageSubresourceRange {
				vk::ImageAspectFlagBits::eColor,
				0, std::max(font->atlas()->mipLevels(), 1u), 0, font->atlas()->layer()
			}
		}
	);

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags {},
		0,
		nullptr,
		0,
		nullptr,
		preTrans.size(),
		preTrans.data()
	);

	/* Copy buffer to texture */

	font->_atlas->buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;
	cmd.copyBufferToImage(
		buffer,
		font->atlas()->buffer(),
		vk::BufferImageCopy {
			0uLL, 0, 0, vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0uL, 0uL, 1uL },
			vk::Offset3D {}, vk::Extent3D { font->_atlas->width(), font->_atlas->height(), 1uL }
		}
	);

	/* Post-transform texture layout */

	Vector<vk::ImageMemoryBarrier> postTrans {};
	postTrans.push_back(
		{
			vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite,
			vk::AccessFlags(),
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			font->atlas()->buffer().image(),
			vk::ImageSubresourceRange {
				vk::ImageAspectFlagBits::eColor,
				0, std::max(atlas.mipLevels(), 1u), 0, atlas.layer()
			}
		}
	);

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eAllCommands,
		vk::DependencyFlags {},
		0,
		nullptr,
		0,
		nullptr,
		static_cast<u32>(postTrans.size()),
		postTrans.data()
	);

	/**/

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();
	font->_atlas->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	buffer.destroy();
}

void transformer::prepareFreeType() {

	if (ftRefCount.fetch_add(1) != 0) {
		return;
	}

	auto error = FT_Init_FreeType(&freeTypeLibrary);
	if (error) {
		IM_CORE_ERROR("Failed to initialize FreeType library.");
		__debugbreak();
		return;
	}

	// Warning: Temporary
	return;
}

void transformer::initFaceFromAsset(cref<assets::Font> asset_) {

	::hg::assertrt(not asset_.sources().empty());
	const auto srcPath = asset_.sources().front();

	const auto [faceIt, inserted] = freeTypeFaces.emplace(asset_.get_guid(), FT_Face {});
	::hg::assertd(inserted);

	/**/

	// error = FT_New_Face(freeTypeLibrary, srcPath.string().c_str(), -1, &cascadiaMonoFace);
	auto error = FT_New_Face(freeTypeLibrary, srcPath.path().string().c_str(), 0, &faceIt->second);

	if (error == FT_Err_Unknown_File_Format) {
		IM_CORE_ERRORF("Font face file `{}` has an unknown file format.", srcPath.path().string());
		__debugbreak();
		return;
	}

	if (error) {
		IM_CORE_ERROR("Failed to load font face.");
		__debugbreak();
		return;
	}

	FT_Set_Pixel_Sizes(faceIt->second, 0, 16);
}

void transformer::cleanupFreeType() {

	if (ftRefCount.fetch_sub(1) != 1) {
		return;
	}

	// Warning: Temporary
	//FT_Done_Face(cascadiaMonoFace);
	for (const auto& [guid, face] : freeTypeFaces.values()) {
		FT_Done_Face(face);
	}
	freeTypeFaces.clear();
	FT_Done_FreeType(freeTypeLibrary);
}

math::uivec2 getFontExtent(cref<engine::assets::Font> font_, cref<FontLoadOptions> options_) {

	const auto& face = freeTypeFaces.at(font_.get_guid());

	math::uivec2 extent {};
	for (const auto& fontSize : options_.fontSizes) {

		/**/
		FT_Set_Pixel_Sizes(face, 0, fontSize);
		/**/

		for (const auto& single : options_.glyphs) {
			/**/
			auto glyphIndex { FT_Get_Char_Index(face, single) };

			if (glyphIndex == 0) {
				continue;
			}

			/**/
			auto error { FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) };

			if (error) {
				continue;
			}

			/**/
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

			if (error) {
				continue;
			}

			/**/
			//extent.x += face->glyph->advance.x >> 6;
			//extent.x += face->glyph->advance.x;
			extent.x += face->glyph->bitmap.width + font_texture_padding.x;
			//extent.y += face->glyph->advance.y >> 6;
			extent.y = MAX(extent.y, face->glyph->bitmap.rows + font_texture_padding.y);
		}

		for (const auto& range : options_.ranges) {
			for (engine::reflow::GlyphCode c { range.begin }; c < range.end; ++c) {

				/**/
				auto glyphIndex { FT_Get_Char_Index(face, c) };

				if (glyphIndex == 0) {
					continue;
				}

				/**/
				auto error { FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) };

				if (error) {
					continue;
				}

				/**/
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

				if (error) {
					continue;
				}

				/**/
				//extent.x += face->glyph->advance.x >> 6;
				//extent.x += face->glyph->advance.x;
				extent.x += face->glyph->bitmap.width + font_texture_padding.x;
				//extent.y += face->glyph->advance.y >> 6;
				extent.y = MAX(extent.y, face->glyph->bitmap.rows + font_texture_padding.y);
			}
		}
	}

	return extent;
}

void writeToMemory(cref<FT_GlyphSlot> slot_, cref<math::uivec2> ext_, cref<math::uivec2> off_, ptr<u8> dst_) {

	auto* cur { dst_ };

	cur += off_.y * ext_.x * 4uL;
	cur += off_.x * 4uL;

	const auto wrapDiff { ext_.x - slot_->bitmap.width };

	const auto gw { slot_->bitmap.width };
	const auto gh { slot_->bitmap.rows };

	auto* src { slot_->bitmap.buffer };
	auto* line { src };

	for (u32 y { 0 }; y < gh; ++y) {

		src = line;
		for (u32 x { 0 }; x < gw; ++x) {
			const auto val { *src };
			++src;

			*cur = 0xFFu;
			++cur;
			*cur = 0xFFu;
			++cur;
			*cur = 0xFFu;
			++cur;
			*cur = val;
			++cur;
		}

		line += slot_->bitmap.pitch;
		cur += wrapDiff * 4;
	}
}

void storeFontToTexture(
	nmpt<FontResource::value_type> font_,
	cref<FontLoadOptions> options_,
	cref<math::uivec2> report_,
	ptr<u8> dst_
) {
	const auto& face = static_cast<FT_Face>(font_->_ftFace);
	::hg::assertrt(font_->_ftFace != nullptr);

	const auto& atlas { font_->_atlas };
	const math::uivec2 atlasExt { atlas->width(), atlas->height() };

	math::uivec2 step { 0, report_.y };
	math::uivec2 fwd { 0 };

	u32 curSize { std::numeric_limits<u32>::max() };
	for (u32 i { 0 }; i < options_.fontSizes.size(); ++i) {

		u32 nextSize { 0uL };
		for (const auto& entry : options_.fontSizes) {
			if (entry < curSize && entry > nextSize) {
				nextSize = entry;
			}
		}
		curSize = nextSize;

		/**/
		FT_Set_Pixel_Sizes(face, 0, nextSize);

		/**/
		font_->_sizes.push_back(nextSize);

		for (const auto& range : options_.ranges) {
			for (engine::reflow::GlyphCode c { range.begin }; c < range.end; ++c) {

				/**/
				auto glyphIndex { FT_Get_Char_Index(face, c) };

				if (glyphIndex == 0) {
					continue;
				}

				/**/
				auto error { FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) };

				if (error) {
					continue;
				}

				/**/
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

				if (error) {
					continue;
				}

				/**/
				const auto nfwd { fwd.x + /*face->glyph->advance.x*/face->glyph->bitmap.width };
				if (nfwd > atlas->width()) {
					fwd.x = 0uL;
					fwd.y += step.y + font_texture_padding.y;
				}

				/**/
				writeToMemory(face->glyph, atlasExt, fwd, dst_);

				/**/
				const math::vec2 viewMinSt {
					static_cast<float>(fwd.x) / static_cast<float>(atlasExt.x),
					static_cast<float>(fwd.y) / static_cast<float>(atlasExt.y)
				};
				const math::vec2 viewMaxSt {
					static_cast<float>(fwd.x + face->glyph->bitmap.width) / static_cast<float>(atlasExt.x),
					static_cast<float>(fwd.y + face->glyph->bitmap.rows) / static_cast<float>(atlasExt.y)
				};

				/**/
				font_->_glyphs.insert_or_assign(
					font_->encodeKey(c, curSize),
					new engine::reflow::FontGlyph {
						math::uivec2 { face->glyph->bitmap.width, face->glyph->bitmap.rows },
						math::ivec2 { face->glyph->bitmap_left, face->glyph->bitmap_top },
						static_cast<float>(face->glyph->advance.x >> 6),
						viewMinSt,
						viewMaxSt
					}
				);

				/**/
				//fwd.x += face->glyph->advance.x >> 6;
				fwd.x += face->glyph->bitmap.width + font_texture_padding.x;
			}
		}
	}

}
