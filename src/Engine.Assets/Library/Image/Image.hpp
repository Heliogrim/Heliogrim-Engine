#pragma once
#include "./Inc.hpp"

namespace clockwork::__internal::asset::types {
	class Image {
	public:

		/**
		 * Make Serializable
		 */
		inline static TypeId __typeId = TypeId::eImage;

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 19.06.2020
		 */
		Image() = default;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 19.06.2020
		 */
		~Image() = default;

	protected:
		/**
		 * Identifier
		 */
		Url _id;

	public:
		const Url id() const {
			return _id;
		}
	};

	class ImageSerializer : public Serializer<Image> {
	public:

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 19.06.2020
		 */
		ImageSerializer() :
			Serializer<Image>(ImageSerializer::serialize, ImageSerializer::deserialize) { }

		/**
		 * Serialize this  to the given stream
		 *
		 * @author Julius
		 * @date 19.06.2020
		 *
		 * @param 		   obj_ The object.
		 * @param [in,out] writer_ The writer.
		 */
		static void serialize(const Image& obj_, SerializationWriter& writer_) { }

		/**
		 * Deserialize this  to the given stream
		 *
		 * @author Julius
		 * @date 19.06.2020
		 *
		 * @param [in,out] dst_ [in,out] If non-null, destination for the.
		 * @param [in,out] reader_ The reader.
		 */
		static void deserialize(Image*& dst_, SerializationReader& reader_) { }
	};
}
