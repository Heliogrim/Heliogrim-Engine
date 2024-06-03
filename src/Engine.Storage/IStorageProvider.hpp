#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::storage::system {
	class __declspec(novtable) IStorageProvider {
	public:
		using this_type = IStorageProvider;

	public:
		constexpr IStorageProvider() noexcept = default;

		IStorageProvider(cref<this_type>) = delete;

		IStorageProvider(mref<this_type>) = delete;

		constexpr virtual ~IStorageProvider() noexcept = default;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) = delete;
	};
}
