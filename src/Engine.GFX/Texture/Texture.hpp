#pragma once

#include <Engine.Common/Math/__default.inl>

#include "__fwd.hpp"
#include "../TextureFormat.hpp"
#include "TextureBuffer.hpp"
#include "TextureType.hpp"
#include "TextureObject.hpp"

namespace hg::engine::gfx {
	class Texture :
		public InheritMeta<Texture, TextureObject> {
	public:
		using value_type = Texture;
		using reference_type = Texture&;
		using const_reference_type = const Texture&;

	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		Texture() noexcept;

		/**
		 * Copy Constructor
		 *
		 * @author Julius
		 * @date 07.12.2020
		 */
		Texture(cref<value_type>) = delete;

		/**
		 * Move Constructor
		 *
		 * @author Julius
		 * @date 07.12.2020
		 *
		 * @param [in,out] other_ The other.
		 */
		Texture(value_type&& other_) noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		~Texture() override;

		/**
		 * Copy assignment operator
		 *
		 * @author Julius
		 * @date 07.12.2020
		 *
		 * @returns A shallow copy of this.
		 */
		Texture& operator=(cref<value_type>) = delete;

		/**
		 * Move assignment operator
		 *
		 * @author Julius
		 * @date 07.12.2020
		 *
		 * @param [in,out] other_ The other.
		 *
		 * @returns A shallow copy of this.
		 */
		Texture& operator=(value_type&& other_) noexcept;

		/**
		 * Cast that converts the given  to a bool
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @returns The result of the operation.
		 */
		explicit operator bool() const noexcept;

		/**
		 * Destroys this 
		 *
		 * @author Julius
		 * @date 20.12.2020
		 */
		void destroy();

	public:
		/**
		 * Create an sub-resource view of this texture
		 *
		 * @author Julius
		 * @date 23.10.2022
		 *
		 * @param layers_ The set of layers `[min, max]` to include
		 * @param extent_ The offset-ed sub-dimensions of the view
		 * @param mipLevels_ The set of mip levels `[min, max]` to include
		 */
		[[nodiscard]] uptr<TextureView> makeView(
			math::uivec2 layers_,
			math::uExtent3D extent_,
			math::uivec2 mipLevels_
		);

	private:
		/**
		 * Buffer
		 */
		TextureBuffer _buffer;

	public:
		/**
		 * Gets a const reference to the buffer
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A reference to a const texture::TextureBuffer.
		 */
		[[nodiscard]] const TextureBuffer& buffer() const noexcept;

		/**
		 * Gets a mutable reference to the buffer
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A reference to a texture::TextureBuffer.
		 */
		[[nodiscard]] TextureBuffer& buffer() noexcept;

	protected:
		/**
		 * Extent
		 */
		math::uivec3 _extent;

	public:
		/**
		 * Gets the extent
		 *
		 * @author Julius
		 * @date 31.08.2020
		 *
		 * @returns A types::uivec3.
		 */
		[[nodiscard]] math::uivec3 extent() const noexcept;

		/**
		 * Gets the extent
		 *
		 * @author Julius
		 * @date 04.12.2020
		 */
		[[nodiscard]] math::uivec3& extent() noexcept;

		/**
		 * Gets the depth
		 *
		 * @author Julius
		 * @date 31.08.2020
		 *
		 * @returns A types::uivec3::value_type.
		 */
		[[nodiscard]] math::uivec3::value_type depth() const noexcept;

		/**
		 * Gets the height
		 *
		 * @author Julius
		 * @date 31.08.2020
		 *
		 * @returns A types::uivec3::value_type.
		 */
		[[nodiscard]] math::uivec3::value_type height() const noexcept;

		/**
		 * Gets the width
		 *
		 * @author Julius
		 * @date 31.08.2020
		 *
		 * @returns A types::uivec3::value_type.
		 */
		[[nodiscard]] math::uivec3::value_type width() const noexcept;

		/**
		 * Gets the amount of layers
		 *
		 * @author Julius
		 * @date 20.12.2020
		 * 
		 * @return A types::uivec3::value_type.
		 */
		[[nodiscard]] math::uivec3::value_type layer() const noexcept;

	private:
		/**
		 * Format
		 */
		TextureFormat _format;

	public:
		/**
		 * Gets the format
		 *
		 * @author Julius
		 * @date 31.08.2020
		 *
		 * @returns The formatted value.
		 */
		[[nodiscard]] TextureFormat format() const noexcept;

		/**
		 * Gets the format
		 *
		 * @author Julius
		 * @date 04.12.2020
		 *
		 * @returns The formatted value.
		 */
		[[nodiscard]] TextureFormat& format() noexcept;

	private:
		/**
		 * Mip Levels
		 */
		u32 _mipLevels;

	public:
		/**
		 * Gets MIP-level
		 *
		 * @author Julius
		 * @date 31.08.2020
		 *
		 * @returns An u32.
		 */
		[[nodiscard]] u32 mipLevels() const noexcept;

		/**
		 * Gets mutable reference to Mip levels
		 *
		 * @author Julius
		 * @date 25.11.2020
		 *
		 * @returns A reference to an u32.
		 */
		[[nodiscard]] u32& mipLevels() noexcept;

	private:
		/**
		 * Texture Type
		 */
		TextureType _type;

	public:
		/**
		 * Gets the type
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A texture::TextureType.
		 */
		[[nodiscard]] TextureType type() const noexcept;

		/**
		 * Gets the type
		 *
		 * @author Julius
		 * @date 04.12.2020
		 *
		 * @returns A reference to a texture::TextureType.
		 */
		[[nodiscard]] TextureType& type() noexcept;

	private:
		/**
		 * View
		 */
		vk::ImageView _view;

	public:
		/**
		 * Vk view
		 *
		 * @author Julius
		 * @date 07.12.2020
		 *
		 * @returns A reference to a const vk::ImageView.
		 */
		[[nodiscard]] const vk::ImageView& vkView() const noexcept;

		/**
		 * Vk view
		 *
		 * @author Julius
		 * @date 07.12.2020
		 *
		 * @returns A reference to a vk::ImageView.
		 */
		[[nodiscard]] vk::ImageView& vkView() noexcept;
	};
}
