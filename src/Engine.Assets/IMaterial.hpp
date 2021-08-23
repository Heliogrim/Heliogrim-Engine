#pragma once
#include <Engine.Common/stdafx.h>

#ifdef DEFAULT_CONSTRUCTOR
#undef DEFAULT_CONSTRUCTOR
#endif
#define DEFAULT_CONSTRUCTOR(class_,name_,texture_,ac_,dc_,sc_,shine_) class_() \
{ \
	setId(#class_); \
	setName(name_); \
	setTexture((texture_.empty() ? #class_ : texture_)); \
	setAmbient(ac_); \
	setDiffuse(dc_); \
	setSpecular(sc_); \
	setShininess(shine_); \
}

namespace asset::material {
	/** A material. */
	class IMaterial {
	public:

		/**
		* General Data
		*
		* @return The identifier.
		*/
		const std::string getId() const {
			return _id;
		};

		/**
		* Gets the name
		*
		* @return The name.
		*/
		const std::string getName() const {
			return _name;
		};

		/**
		* Gets the texture
		*
		* @return The texture.
		*/
		const std::string getTexture() const {
			return _texture;
		};

		/**
		* Gets ambient color
		*
		* @return The ambient color.
		*/
		const clockwork::__internal::types::vec3 getAmbientColor() const {
			return _ambient;
		};

		/**
		* Gets diffuse color
		*
		* @return The diffuse color.
		*/
		const clockwork::__internal::types::vec3 getDiffuseColor() const {
			return _diffuse;
		};

		/**
		* Gets specular color
		*
		* @return The specular color.
		*/
		const clockwork::__internal::types::vec3 getSpecularColor() const {
			return _specular;
		};

		/**
		* Gets the shininess
		*
		* @return The shininess.
		*/
		const float getShininess() const {
			return _shininess;
		};

		//protected:

		/**
		* Sets an identifier
		*
		* @param id_ The identifier.
		*/
		void setId(const std::string& id_) {
			_id = id_;
		};

		/**
		* Sets a name
		*
		* @param name_ The name.
		*/
		void setName(const std::string& name_) {
			_name = name_;
		};

		/**
		* Sets a texture
		*
		* @param texture_ The texture.
		*/
		void setTexture(const std::string& texture_) {
			_texture = texture_;
		};

		/**
		* Sets an ambient
		*
		* @param color_ The color.
		*/
		void setAmbient(const clockwork::__internal::types::vec3& color_) {
			_ambient = color_;
		};

		/**
		* Sets a diffuse
		*
		* @param color_ The color.
		*/
		void setDiffuse(const clockwork::__internal::types::vec3& color_) {
			_diffuse = color_;
		};

		/**
		* Sets a specular
		*
		* @param color_ The color.
		*/
		void setSpecular(const clockwork::__internal::types::vec3& color_) {
			_specular = color_;
		};

		/**
		* Sets the shininess
		*
		* @param shininess_ The shininess.
		*/
		void setShininess(const float shininess_) {
			_shininess = shininess_;
		};

	private:
		std::string _id;
		std::string _name;
		std::string _texture;

		clockwork::__internal::types::vec3 _ambient;
		clockwork::__internal::types::vec3 _diffuse;
		clockwork::__internal::types::vec3 _specular;
		float _shininess = .0F;
	};

	TYPE_SHARED(asset::material::IMaterial, IMaterial);
};

namespace std {
	template <>
	struct hash<asset::material::IMaterial> {
		size_t operator()(const asset::material::IMaterial& material_) const noexcept {
			// TODO: new hash for material - take all attribute into account
			material_.getAmbientColor();
			material_.getDiffuseColor();
			material_.getSpecularColor();
			material_.getShininess();
			material_.getId();
			material_.getName();
			material_.getTexture();

			return ((hash<clockwork::__internal::types::vec3>()(material_.getAmbientColor()) ^ (hash<
				clockwork::__internal::types::vec3>()(material_.getDiffuseColor()) << 1)) >> 1) ^ (hash<
					clockwork::__internal::types::vec2>()(clockwork::__internal::types::vec2(material_.getShininess()))
				<< 1
			);
		}
	};
};
