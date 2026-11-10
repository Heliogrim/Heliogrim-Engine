#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Graphics/TextureFormat.hpp>
#include <Engine.Graphics/Texture/TextureType.hpp>
#include <Engine.Graphics/Texture/__fwd.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>

#include "Description.hpp"

namespace hg::engine::render::graph {
	struct ActiveMipBitMask {
		using mask_type = u16;
		mask_type mask;

		[[nodiscard]] bool active(u8 baseTwoLevel_) const {
			return mask & (0x1 << baseTwoLevel_);
		}

		[[nodiscard]] constexpr static u8 maxMipLevel() noexcept {
			return static_cast<u8>(sizeof(mask) * 8 - 1);
		}

		[[nodiscard]] constexpr bool operator==(const ActiveMipBitMask& other_) const noexcept {
			return mask == other_.mask;
		}

		[[nodiscard]] constexpr bool operator!=(const ActiveMipBitMask& other_) const noexcept {
			return mask != other_.mask;
		}

		[[nodiscard]] constexpr std::strong_ordering operator<=>(const ActiveMipBitMask& other_) const noexcept {
			return mask <=> other_.mask;
		}
	};

	class TextureDescription final :
		public InheritMeta<TextureDescription, Description> {
	public:
		using this_type = TextureDescription;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

	public:
		TextureDescription() noexcept;

		TextureDescription(
			mref<DescriptionValue<graphics::TextureType>> textureType_,
			mref<DescriptionValue<graphics::TextureFormat>> textureFormat_,
			mref<DescriptionValue<u32>> textureLayers_,
			//mref<DescriptionValue<math::uivec3>> textureExtent_,
			mref<DescriptionValue<ActiveMipBitMask>> textureMips_
		) noexcept;

		~TextureDescription() override;

	protected:
		[[nodiscard]] bool isValueCompatible(
			const non_owning_rptr<const Description> other_
		) const noexcept override;

	private:
	public:
		DescriptionValue<graphics::TextureType> _textureType;
		DescriptionValue<graphics::TextureFormat> _textureFormat;

		DescriptionValue<u32> _textureLayers;
		//DescriptionValue<math::uivec3> _textureExtent;

		DescriptionValue<ActiveMipBitMask> _textureMips;

	public:
		[[nodiscard]] bool isValidObject(const nmpt<graphics::Texture> texture_) const noexcept;

		[[nodiscard]] bool isValidObject(const nmpt<graphics::TextureView> textureView_) const noexcept;

		[[nodiscard]] bool isValidObject(const nmpt<graphics::SparseTexture> texture_) const noexcept;

		[[nodiscard]] bool isValidObject(const nmpt<graphics::SparseTextureView> textureView_) const noexcept;
	};
}
