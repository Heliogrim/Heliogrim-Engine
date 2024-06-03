#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::storage {
	class __declspec(novtable) StorageInputStream {
	public:
		using this_type = StorageInputStream;

	protected:
		constexpr StorageInputStream() = default;

	public:
		StorageInputStream(cref<this_type>) = delete;

		StorageInputStream(mref<this_type>) = delete;

		constexpr virtual ~StorageInputStream() noexcept = default;
	};
}
