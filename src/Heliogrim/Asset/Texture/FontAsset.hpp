#pragma once

#include <Engine.Common/String.hpp>

#include "../Asset.hpp"

namespace hg {
	class FontAsset :
		public Asset {
	public:
		using this_type = FontAsset;
		using base_type = Asset;

	protected:
		FontAsset(mref<asset_guid> guid_) noexcept;

		FontAsset(mref<asset_guid> guid_, cref<string> url_) noexcept;

	public:
		~FontAsset() noexcept;

	public:
		[[nodiscard]] bool isValidType() const noexcept;

	public:
		[[nodiscard]] cref<string> url() const noexcept;
	};
}
