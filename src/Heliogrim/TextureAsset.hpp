#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "StreamableRenderableAsset.hpp"
#include "TextureFormat.hpp"
#include "TextureType.hpp"

namespace hg {
	class ImageAsset;

	class TextureAsset :
		public StreamableRenderableAsset {
	public:
		using base_type = StreamableRenderableAsset;

		using value_type = TextureAsset;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 */
		TextureAsset(mref<asset_guid> guid_) noexcept;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 * @param  baseImage_ The base image.
		 * @param  images_ The images.
		 * @param  extent_ The extent.
		 * @param  format_ The texture format
		 * @param  mipLevel_ The mip level.
		 * @param  type_ The texture type.
		 */
		TextureAsset(
			mref<asset_guid> guid_,
			cref<asset_guid> baseImage_,
			mref<Vector<asset_guid>> images_,
			cref<math::uivec3> extent_,
			cref<TextureFormat> format_,
			cref<u32> mipLevel_,
			cref<TextureType> type_
		) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~TextureAsset() noexcept;

	public:
		/**
		 * Query if this is valid type
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns True if valid type, false if not.
		 */
		[[nodiscard]] bool isValidType() const noexcept;

	public:
		[[nodiscard]] nmpt<const ImageAsset> baseImage() const noexcept;

		[[nodiscard]] cref<asset_guid> baseImageGuid() const noexcept;

		[[nodiscard]] Vector<const ptr<const ImageAsset>> images() const noexcept;

		[[nodiscard]] Vector<asset_guid> imagesGuids() const noexcept;

		[[nodiscard]] cref<math::uivec3> extent() const noexcept;

		[[nodiscard]] cref<TextureFormat> format() const noexcept;

		[[nodiscard]] cref<u32> mipLevel() const noexcept;

		[[nodiscard]] cref<TextureType> type() const noexcept;
	};
}
