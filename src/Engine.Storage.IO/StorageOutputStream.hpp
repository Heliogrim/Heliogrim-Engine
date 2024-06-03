#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::storage {
	class __declspec(novtable) StorageOutputStream {
	public:
		using this_type = StorageOutputStream;

	protected:
		constexpr StorageOutputStream() = default;

	public:
		StorageOutputStream(cref<this_type>) = delete;

		StorageOutputStream(mref<this_type>) = delete;

		constexpr virtual ~StorageOutputStream() noexcept = default;
	};
}
