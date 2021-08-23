#pragma once
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.GFX/Color/Color.hpp>
#include "../ResourceReference.hpp"

using color = clockwork::__internal::gfx::color;

namespace clockwork::__internal::asset::types {
	class Material {
	public:

		/**
		 * Make Serializable
		 */
		static core::type_id __typeId;

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 09.06.2020
		 */
		Material() = default;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 18.06.2020
		 */
		~Material() = default;

	protected:
		/**
		 * Identifier
		 */
		Url _id;

	public:
		const Url& id() const {
			return _id;
		}

	private:
		/**
		 * Albedo
		 */
		color _albedoBase;
		__ptr__ResourceReference _albedoResource;

	public:
		const color& albedoBase() const {
			return _albedoBase;
		}

		const __ptr__ResourceReference& albedoResource() const {
			return _albedoResource;
		}

	private:
		/**
		 * Alpha
		 */
		__ptr__ResourceReference _alphaResource;

	public:
		const __ptr__ResourceReference& alphaResource() const {
			return _alphaResource;
		}

	private:
		/**
		 * AO
		 */
		__ptr__ResourceReference _aoResource;

	public:
		const __ptr__ResourceReference& aoResource() const {
			return _aoResource;
		}

	private:
		/**
		 * Diffuse
		 */
		color _diffuseBase;
		__ptr__ResourceReference _diffuseResource;

	public:
		const color& diffuseBase() const {
			return _diffuseBase;
		}

		const __ptr__ResourceReference& diffuseResource() const {
			return _diffuseResource;
		}

	private:
		/**
		 * Displacement
		 */
		__ptr__ResourceReference _displaceResource;

	public:
		const __ptr__ResourceReference& displacementResource() const {
			return _displaceResource;
		}

	private:
		/**
		 * Metalness
		 */
		__ptr__ResourceReference _metalnessResource;

	public:
		const __ptr__ResourceReference& metalnessResource() const {
			return _metalnessResource;
		}

	private:
		/**
		 * Normal
		 */
		__ptr__ResourceReference _normalResource;

	public:
		const __ptr__ResourceReference& normalResource() const {
			return _normalResource;
		}

	private:
		/**
		 * Roughness
		 */
		__ptr__ResourceReference _roughnessResource;

	public:
		const __ptr__ResourceReference& roughnessResource() const {
			return _roughnessResource;
		}

	private:
		/**
		 * Specular
		 */
		color _specularBase;
		__ptr__ResourceReference _specularResource;

	public:
		const color& specularBase() const {
			return _specularBase;
		}

		const __ptr__ResourceReference& specularResource() const {
			return _specularResource;
		}
	};

	class MaterialSerializer : public Serializer<Material> {
	public:

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 11.06.2020
		 */
		MaterialSerializer() :
			Serializer<Material>(MaterialSerializer::serialize, MaterialSerializer::deserialize) { }

		/**
		 * Serialize this  to the given stream
		 *
		 * @author Julius
		 * @date 11.06.2020
		 *
		 * @param 		   obj_ The object.
		 * @param [in,out] writer_ The writer.
		 */
		static void serialize(const Material& obj_, SerializationWriter& writer_) { }

		/**
		 * Deserialize this  to the given stream
		 *
		 * @author Julius
		 * @date 11.06.2020
		 *
		 * @param [in,out] dst_ [in,out] If non-null, destination for the.
		 * @param [in,out] reader_ The reader.
		 */
		static void deserialize(Material*& dst_, SerializationReader& reader_) { }
	};

	TYPE_SHARED(clockwork::__internal::asset::types::Material, Material);
}
