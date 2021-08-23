#pragma once
#include "Inc.hpp"
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Resource/Library/Serializer.hpp>
#include <Engine.Resource/Library/Url.hpp>

using SerializationReader = clockwork::__internal::resource::types::SerializationReader;
using SerializationWriter = clockwork::__internal::resource::types::SerializationWriter;
using Url = clockwork::__internal::resource::types::Url;

template <typename Ty>
using Serializer = clockwork::__internal::resource::types::Serializer<Ty>;

namespace clockwork::__internal::asset::types {
	class Geometry {
	public:

		/**
		 * Make Serializable
		 */
		static core::type_id __typeId;

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 17.06.2020
		 */
		Geometry() = default;

	private:
		/**
		 * Identifier
		 */
		Url _id;

	public:
		const Url& id() const {
			return _id;
		}

	private:
	};

	class GeometrySerializer : public Serializer<Geometry> {
	public:

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 17.06.2020
		 */
		GeometrySerializer() :
			Serializer<Geometry>(GeometrySerializer::serialize, GeometrySerializer::deserialize) { }

		/**
		 * Serialize this  to the given stream
		 *
		 * @author Julius
		 * @date 17.06.2020
		 *
		 * @param 		   obj_ The object.
		 * @param [in,out] writer_ The writer.
		 */
		static void serialize(const Geometry& obj_, SerializationWriter& writer_) { }

		/**
		 * Deserialize this  to the given stream
		 *
		 * @author Julius
		 * @date 17.06.2020
		 *
		 * @param [in,out] dst_ [in,out] If non-null, destination for the.
		 * @param [in,out] reader_ The reader.
		 */
		static void deserialize(Geometry*& dst_, SerializationReader& reader_) { }
	};

	TYPE_SHARED(clockwork::__internal::asset::types::Geometry, Geometry);
}
