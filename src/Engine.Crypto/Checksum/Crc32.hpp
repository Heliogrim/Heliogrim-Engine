#pragma once

#include <span>

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::crypto {
	class Crc32 {
	public:
		using this_type = Crc32;
		using result_type = u32;

	private:
		result_type _intermediate {};
		bool _started = false;

	public:
		ref<Crc32> update(_In_ std::span<const _::byte> data_);

		[[nodiscard]] result_type finalize() &&;
	};
}
