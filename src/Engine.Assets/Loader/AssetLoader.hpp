#pragma once

#include "Inc.hpp"
#include "../Asset.hpp"
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Resource/Library/Url.hpp>

namespace clockwork::__internal::assets::loader {
	class AssetLoader {
	public:
		using this_type = _STD unique_ptr<AssetLoader>;
		using reference_type = AssetLoader&;
		using const_reference_type = const AssetLoader&;

		using future_type = concurrent::future<Asset::value_type>;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 24.08.2020
		 */
		~AssetLoader() = default;

		/**
		 * Makes a instance of the AssetLoader
		 *
		 * @author Julius
		 * @date 24.08.2020
		 *
		 * @returns A const_reference_type to the unique instance.
		 */
		static const_reference_type make();

		/**
		 * Gets the unique instance of AssetLoader
		 *
		 * @author Julius
		 * @date 24.08.2020
		 *
		 * @returns A const_reference_type to the unique instance.
		 */
		static const_reference_type get();

		/**
		 * Loads the data structure of the given Url
		 *
		 * @author Julius
		 * @date 24.08.2020
		 *
		 * @param  url_ The Url mapping the data to load.
		 *
		 * @returns A future_type.
		 */
		future_type load(const resource::types::Url& url_) const;

	private:
		static this_type _this;

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 24.08.2020
		 */
		AssetLoader();
	};
}
