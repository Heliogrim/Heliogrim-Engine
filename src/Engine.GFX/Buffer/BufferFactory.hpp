#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Engine.GFX/Device/Device.hpp"

namespace hg::engine::gfx {
	struct BufferBuildPayload {
		size_t byteSize = 0uLL;
		align_t byteAlign = 0uLL;

		/**/

		MemoryProperties memoryProperties;

		/**/

		vk::BufferCreateFlags vkFlags;
		vk::BufferUsageFlags vkUsage;
		vk::SharingMode vkSharing = vk::SharingMode::eExclusive;

		[[nodiscard]] explicit operator bool() const noexcept;
	};

	/**/

	class __declspec(novtable) BufferFactory {
	public:
		static nmpt<BufferFactory> get();

		static void destroy();

	protected:
		static uptr<BufferFactory> _instance;

	protected:
		BufferFactory(cref<sptr<Device>> device_);

	public:
		virtual ~BufferFactory() noexcept;

	protected:
		sptr<Device> _device;

	public:
		[[nodiscard]] virtual Buffer build(
			mref<BufferBuildPayload> payload_
		) const = 0;
	};
}
