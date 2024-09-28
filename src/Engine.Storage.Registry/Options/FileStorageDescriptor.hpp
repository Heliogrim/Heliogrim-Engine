#pragma once

#include "IStorageDescriptor.hpp"
#include "../Url/FileUrl.hpp"

namespace hg::engine::storage {
	class FileStorageDescriptor final :
		public IStorageDescriptor<FileStorageDescriptor> {
	public:
		using this_type = FileStorageDescriptor;

	public:
		struct Flags {
			u8 createIfNotExist : 1;
		};

	public:
		FileStorageDescriptor() = delete;

		explicit constexpr FileStorageDescriptor(mref<FileUrl> fileUrl_) noexcept :
			IStorageDescriptor(),
			url(std::move(fileUrl_)),
			flags() {}

		constexpr FileStorageDescriptor(mref<FileUrl> fileUrl_, mref<Flags> flags_) noexcept :
			IStorageDescriptor(),
			url(std::move(fileUrl_)),
			flags(std::move(flags_)) {}

		FileStorageDescriptor(cref<this_type>) = delete;

		constexpr FileStorageDescriptor(mref<this_type> other_) noexcept = default;

		constexpr ~FileStorageDescriptor() noexcept = default;

	public:
		FileUrl url;
		Flags flags;

	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const noexcept {
			return url.scheme();
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return not url.scheme().empty();
		}
	};
}
