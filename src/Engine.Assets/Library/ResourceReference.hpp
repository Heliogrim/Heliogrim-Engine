#pragma once
#include "Inc.hpp"
#include <Engine.Resource/Library/Resource.hpp>
#include <Engine.Resource/Library/Url.hpp>

using __ptr__Resource = clockwork::__internal::resource::types::__ptr__Resource;
using Url = clockwork::__internal::resource::types::Url;

namespace clockwork::__internal::asset::types {

	/**
	 * A resource reference.
	 *	The internal url will identify the resource located in the data structure.
	 *	Cause of the url, the resource could be a remote link, local file or any other data location.
	 *	The provided resource pointer will be determined at runtime to reference the usable resource.
	 *
	 * @author Julius
	 * @date 09.06.2020
	 */
	class ResourceReference {
	public:

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 09.06.2020
		 */
		ResourceReference() = default;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 09.06.2020
		 *
		 * @param  url_ URL_ of the resource.
		 */
		ResourceReference(const Url& url_) :
			_url(url_)
		{ }
		
	private:
		/**
		 * Url
		 */
		Url _url;

	public:
		const Url& url() const {
			return _url;
		}

		Url& url() {
			return _url;
		}

	private:
		/**
		 * Resource
		 */
		__ptr__Resource _resource;

	public:
		const __ptr__Resource& resource() const {
			return _resource;
		}

		__ptr__Resource& resource() {
			return _resource;
		}
	};

	TYPE_RAW_PTR(clockwork::__internal::asset::types::ResourceReference, ResourceReference);
}
