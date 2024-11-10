#pragma once

#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "Package.hpp"
#include "Footer/PackageFooter.hpp"
#include "Header/PackageHeader.hpp"
#include "Linker/ReadonlyPackageLinker.hpp"

namespace hg::engine::resource {
	class ReadonlyPackage :
		public InheritMeta<ReadonlyPackage, Package> {
	public:
		using this_type = ReadonlyPackage;

	public:
		ReadonlyPackage() = delete;

		ReadonlyPackage(
			_In_ mref<PackageHeader> header_,
			_In_ mref<ReadonlyPackageLinker> linker_,
			_In_ mref<PackageFooter> footer_
		) noexcept :
			InheritMeta(),
			_header(::hg::move(header_)),
			_linker(::hg::move(linker_)),
			_footer(::hg::move(footer_)) {}

		ReadonlyPackage(cref<this_type>) = delete;

		ReadonlyPackage(mref<this_type>) noexcept = default;

		~ReadonlyPackage() noexcept override = default;

	private:
		PackageHeader _header;
		ReadonlyPackageLinker _linker;
		PackageFooter _footer;

	public:
		[[nodiscard]] constexpr cref<PackageHeader> getHeader() const noexcept {
			return _header;
		}

		[[nodiscard]] constexpr cref<ReadonlyPackageLinker> getLinker() const noexcept {
			return _linker;
		}

		[[nodiscard]] constexpr cref<PackageFooter> getFooter() const noexcept {
			return _footer;
		}
	};
}
