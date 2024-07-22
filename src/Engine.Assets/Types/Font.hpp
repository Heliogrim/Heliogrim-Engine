#pragma once

#include <Engine.Filesystem/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflow/Font/GlyphRanges.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include "Asset.hpp"

namespace hg::engine::assets {
	class Font :
		public InheritMeta<Font, Asset> {
	public:
		template <typename>
		friend class serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		inline static constexpr asset_type_id typeId { "Font"_typeId };

	public:
		Font();

	protected:
		Font(mref<asset_guid> guid_);

	public:
		Font(mref<asset_guid> guid_, mref<Vector<fs::Url>> sources_);

	private:
		Vector<fs::Url> _sources;

	public:
		[[nodiscard]] cref<Vector<fs::Url>> sources() const noexcept;
	};
}
