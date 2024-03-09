#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "../Device/Device.hpp"
#include "../vkinc.hpp"

namespace hg::engine::gfx {
	class TextureSampler :
		public InheritBase<TextureSampler> {
	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		TextureSampler() noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		~TextureSampler() noexcept;

		/**
		 * Setups with given device
		 *
		 * @author Julius
		 * @date 14.12.2020
		 *
		 * @param  device_ The device.
		 */
		void setup(cref<sptr<Device>> device_);

		/**
		 * Destroys this 
		 *
		 * @author Julius
		 * @date 14.12.2020
		 */
		void destroy();

	private:
		/**
		 * Device
		 */
		sptr<Device> _device;

	private:
		/**
		 * Anisotropy
		 */
		float _anisotropy;

	public:
		/**
		 * Gets the anisotropy
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A float.
		 */
		[[nodiscard]] float anisotropy() const noexcept;

		/**
		 * Gets the anisotropy
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A reference to the float.
		 */
		[[nodiscard]] float& anisotropy() noexcept;

	private:
		/**
		 * Lod's
		 */
		u8 _lods;

	public:
		/**
		 * Gets the lod count
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns An u8.
		 */
		[[nodiscard]] u8 lods() const noexcept;

		/**
		 * Gets the lod count
		 *
		 * @author Julius
		 * @date 14.12.2020
		 *
		 * @returns A reference to an u8.
		 */
		[[nodiscard]] u8& lods() noexcept;

	private:
		/**
		 * Magnification
		 */
		vk::Filter _magnification;

	public:
		/**
		 * Gets the magnification
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A vk::Filter.
		 */
		[[nodiscard]] vk::Filter magnification() const noexcept;

		[[nodiscard]] ref<vk::Filter> magnification() noexcept;

	private:
		/**
		 * Minification
		 */
		vk::Filter _minification;

	public:
		/**
		 * Gets the minification
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A vk::Filter.
		 */
		[[nodiscard]] vk::Filter minification() const noexcept;

		[[nodiscard]] ref<vk::Filter> minification() noexcept;

	private:
		/**
		 * Mip Mode
		 */
		vk::SamplerMipmapMode _mipMode;

	public:
		/**
		 * Sampler Mipmap Mode
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A reference to the vk::SamplerMipmapMode.
		 */
		[[nodiscard]] ref<vk::SamplerMipmapMode> mipMode() noexcept;

	private:
		/**
		 * Addressing
		 */
		vk::SamplerAddressMode _addressModeU;
		vk::SamplerAddressMode _addressModeV;
		vk::SamplerAddressMode _addressModeW;

	public:
		/**
		 * Address mode u
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A vk::SamplerAddressMode.
		 */
		[[nodiscard]] vk::SamplerAddressMode addressModeU() const noexcept;

		/**
		 * Address mode u
		 *
		 * @author Julius
		 * @date 22.12.2020
		 *
		 * @returns A reference to a vk::SamplerAddressMode.
		 */
		[[nodiscard]] vk::SamplerAddressMode& addressModeU() noexcept;

		/**
		 * Address mode v
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A vk::SamplerAddressMode.
		 */
		[[nodiscard]] vk::SamplerAddressMode addressModeV() const noexcept;

		/**
		 * Address mode v
		 *
		 * @author Julius
		 * @date 22.12.2020
		 *
		 * @returns A reference to a vk::SamplerAddressMode.
		 */
		[[nodiscard]] vk::SamplerAddressMode& addressModeV() noexcept;

		/**
		 * Address mode w
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns A vk::SamplerAddressMode.
		 */
		[[nodiscard]] vk::SamplerAddressMode addressModeW() const noexcept;

		/**
		 * Address mode w
		 *
		 * @author Julius
		 * @date 22.12.2020
		 *
		 * @returns A reference to a vk::SamplerAddressMode.
		 */
		[[nodiscard]] vk::SamplerAddressMode& addressModeW() noexcept;

	private:
		/**
		 * Border
		 */
		vk::BorderColor _borderColor;

	public:
		[[nodiscard]] vk::BorderColor& borderColor() noexcept;

	private:
		/**
		 * Vulkan API
		 */
		vk::Sampler _vkSampler;

	public:
		[[nodiscard]] const vk::Sampler& vkSampler() const noexcept;
	};
}
