#pragma once

#include <Engine.Common/__macro.hpp>

#include "Engine.Graphics/vkinc.hpp"
#include "Engine.Graphics/Device/Device.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
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

	class macro_novtable BufferFactory {
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
