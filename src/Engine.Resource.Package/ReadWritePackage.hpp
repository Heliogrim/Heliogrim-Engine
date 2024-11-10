#pragma once

#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "Package.hpp"
#include "Footer/PackageFooter.hpp"
#include "Header/PackageHeader.hpp"
#include "Linker/ReadWritePackageLinker.hpp"

namespace hg::engine::resource {
	class ReadWritePackage :
		public InheritMeta<ReadWritePackage, Package> {
	public:
		using this_type = ReadWritePackage;

	public:
		ReadWritePackage() = delete;

		ReadWritePackage(
			_Inout_ mref<PackageHeader> header_,
			_Inout_ mref<ReadWritePackageLinker> linker_,
			_Inout_ mref<PackageFooter> footer_
		) noexcept :
			InheritMeta(),
			_header(::hg::move(header_)),
			_linker(::hg::move(linker_)),
			_footer(::hg::move(footer_)) {}

		ReadWritePackage(cref<this_type>) = delete;

		ReadWritePackage(mref<this_type>) noexcept = default;

		~ReadWritePackage() noexcept override = default;

	private:
		PackageHeader _header;
		ReadWritePackageLinker _linker;
		PackageFooter _footer;

	public:
		[[nodiscard]] constexpr cref<PackageHeader> getHeader() const noexcept {
			return _header;
		}

		[[nodiscard]] constexpr ref<PackageHeader> getHeader() noexcept {
			return _header;
		}

		[[nodiscard]] constexpr cref<ReadWritePackageLinker> getLinker() const noexcept {
			return _linker;
		}

		[[nodiscard]] constexpr ref<ReadWritePackageLinker> getLinker() noexcept {
			return _linker;
		}

		[[nodiscard]] constexpr cref<PackageFooter> getFooter() const noexcept {
			return _footer;
		}

		[[nodiscard]] constexpr ref<PackageFooter> getFooter() noexcept {
			return _footer;
		}
	};
}
