#pragma once

#include "RenderableAsset.hpp"
#include "StreamableAsset.hpp"

namespace hg {
	class StreamableRenderableAsset :
		public RenderableAsset {
	public:
		using base_type = RenderableAsset;

		using value_type = StreamableRenderableAsset;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 * @param  typeId_ Identifier for the type.
		 * @param  internal_ The internal.
		 */
		StreamableRenderableAsset(
			mref<asset_guid> guid_,
			cref<asset_type_id> typeId_,
			_In_ ref<::hg::engine::assets::Asset> internal_
		) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~StreamableRenderableAsset() noexcept;
	};
}
