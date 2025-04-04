#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/__default.inl>

#include "SparseTexturePageFlag.hpp"
#include "__fwd.hpp"
#include "../Device/Device.hpp"
#include "../Memory/VirtualMemoryPage.hpp"

namespace hg::engine::gfx {
	class SparseTexturePage final {
	public:
		friend class ::hg::engine::gfx::SparseTexture;

	public:
		using this_type = SparseTexturePage;

	protected:
		SparseTexturePage(
			nmpt<VirtualMemoryPage> memory_,
			u32 layer_,
			math::uivec3 offset_,
			math::uivec3 extent_,
			u32 mipLevel_,
			const SparseTexturePageFlags flags_ = {}
		);

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		~SparseTexturePage() noexcept;

	private:
		SparseTexturePageFlags _flags;

	public:
		[[nodiscard]] const SparseTexturePageFlags flags() const noexcept;

	private:
		nmpt<VirtualMemoryPage> _memory;

	private:
		[[nodiscard]] nmpt<VirtualMemoryPage> release() noexcept;

	public:
		/**
		 * Get the underlying paged virtual memory
		 *
		 * @author Julius
		 * @date 19.06.2022
		 *
		 * @returns A non owning pointer to the const VirtualMemoryPage.
		 */
		[[nodiscard]] nmpt<VirtualMemoryPage> memory() const noexcept;

		/**
		 * Check whether this is currently residential
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool residential() const noexcept;

		bool load();

		bool unload();

	private:
		u32 _layer;
		math::uivec3 _offset;
		math::uivec3 _extent;
		u32 _mipLevel;

	public:
		[[nodiscard]] u32 layer() const noexcept;

		[[nodiscard]] cref<math::uivec3> offset() const noexcept;

		[[nodiscard]] cref<math::uivec3> extent() const noexcept;

		[[nodiscard]] u32 mipLevel() const noexcept;

	protected:
		[[nodiscard]] vk::SparseImageMemoryBind vkSparseImageMemoryBind() const noexcept;

		[[nodiscard]] vk::SparseMemoryBind vkSparseMemoryBind() const noexcept;
	};
}
