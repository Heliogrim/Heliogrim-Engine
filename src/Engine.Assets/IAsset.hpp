#pragma once

#include <Engine.Common/stdafx.h>
#include <Engine.Resource/FileResource.hpp>

#ifdef DEFAULT_CONSTRUCTOR
#undef DEFAULT_CONSTRUCTOR
#endif
#define DEFAULT_CONSTRUCTOR(class_,id_) class_() \
{ \
	setId(id_); \
	setName(#class_); \
	setMaterials(std::vector<std::string>({ QUOTE(DOUBLE(class_,DOUBLE(DOUBLE(__,id_),__))) })); \
}

#ifdef DEFAULT_CONSTRUCTOR_5
#undef DEFAULT_CONSTRUCTOR_5
#endif
#define DEFAULT_CONSTRUCTOR_5(class_,id_,model_,materials_,modelData_) class_() \
{ \
	setId(id_); \
	setName(#class_); \
	setModelType(model_); \
	setMaterials(materials_); \
	setModelData(modelData_); \
}

namespace asset::subject {
	class ModelData {
	public:
		ModelData() :
			_lodCount(1),
			_partCount(1) { }

		ModelData(const uint8_t partCount_, const uint8_t lodCount_ = 1) :
			_lodCount(lodCount_),
			_partCount(partCount_) { }

		uint8_t getLODCount() const {
			return _lodCount;
		}

		uint8_t getPartCount() const {
			return _partCount;
		}

	private:
		uint8_t _lodCount;
		uint8_t _partCount;
	};

	class IAsset {
	public:
		using identifier = uint32_t;

		IAsset() :
			_id(static_cast<identifier>(0)) { }

		/**
		* Is building
		*
		* @return A const bool.
		*/
		virtual bool isBuilding() const {
			return false;
		}

		/**
		* Is living
		*
		* @return A const bool.
		*/
		virtual bool isLiving() const {
			return false;
		}

		/**
		* Is plant
		*
		* @return A const bool.
		*/
		virtual bool isPlant() const {
			return false;
		}

		/**
		* Is static
		*
		* @return A const bool.
		*/
		virtual bool isStatic() const {
			return false;
		}

		/**
		* Gets asset identifier
		*
		* @return The asset identifier.
		*/
		identifier getAssetId() const;

		/**
		 * Gets the name
		 *
		 * @return The name.
		 */
		std::string getName() const;

		/**
		* Gets model file
		*
		* @return The model file.
		*/
		std::string getModelFile() const;

		/* Temporary */
		[[nodiscard]] FileResource::DataType getModelFileType() const {
			return _modelType;
		}

		/**
		 * Gets model data
		 *
		 * @return The model data.
		 */
		ModelData getModelData() const;

		/**
		* Gets the materials
		*
		* @return The materials.
		*/
		std::vector<std::string> getMaterials() const;

		//protected:

		/**
		* Sets an identifier
		*
		* @param id_ The identifier.
		*/
		void setId(const uint32_t id_) {
			_id = id_;
		}

		/**
		* Sets a name
		*
		* @param name_ The name.
		*/
		void setName(const std::string& name_) {
			_classname = name_;
		}

		/**
		* Sets a material
		*
		* @param index_    Zero-based index of the.
		* @param material_ The material.
		*/
		void setMaterial(const uint8_t index_, const std::string& material_) {
			if (index_ < _materials.size()) {
				_materials[index_] = material_;
			}
		}

		/**
		* Sets the materials
		*
		* @param materials_ The materials.
		*/
		void setMaterials(const std::vector<std::string>& materials_) {
			_materials.clear();
			_materials.insert(_materials.end(), materials_.begin(), materials_.end());
		}

		/**
		 * Sets model data
		 *
		 * @param data_ The data.
		 */
		void setModelData(const ModelData& data_) {
			_modelData = data_;
		}

		/**
		* Sets model type
		*
		* @param type_ The type.
		*/
		void setModelType(const FileResource::DataType& type_) {
			_modelType = type_;
		}

	private:
		std::string _classname;
		identifier _id;

		std::vector<std::string> _materials;

		ModelData _modelData;
		FileResource::DataType _modelType = FileResource::DataType::eObj;

		/**
		* Gets asset path
		*
		* @return The asset path.
		*/
		std::string getAssetPath() const;
	};

	TYPE_SHARED(asset::subject::IAsset, IAsset);
};
