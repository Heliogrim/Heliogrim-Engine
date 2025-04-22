#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

namespace hg::engine::assets {
	class Image final :
		public InheritMeta<Image, Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "Image"_typeId };

	private:
	public:
		Image();

	protected:
		Image(mref<AssetGuid> guid_);

	public:
		Image(mref<AssetGuid> guid_, mref<Vector<fs::Url>> sources_);

	private:
		Vector<fs::Url> _sources;

	public:
		[[nodiscard]] cref<Vector<fs::Url>> sources() const noexcept;

		void addSource(cref<fs::Url> sourceUrl_);

		void removeSource(cref<fs::Url> sourceUrl_);
	};
}
