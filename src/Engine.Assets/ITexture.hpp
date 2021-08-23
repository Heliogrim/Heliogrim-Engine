#pragma once

#include <Engine.Common/stdafx.h>
#include <Engine.Resource/FileResource.hpp>
#include <Engine.Utils/StringUtils.hpp>

#ifdef DEFAULT_CONSTRUCTOR
#undef DEFAULT_CONSTRUCTOR
#endif
#define DEFAULT_CONSTRUCTOR(class_,name_,file_) class_() \
{ \
	setId(#class_); \
	setName(name_); \
	setFile((file_.empty() ? #class_ : file_)); \
}

namespace asset::texture {
	/** A texture. */
	class ITexture {
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
		* Gets the file
		*
		* @return The file.
		*/
		const std::string getFile() const;

		/**
		* Gets file name
		*
		* @return The file name.
		*/
		const std::string getFileName() const {
			return _file;
		};

		/**
		 * Gets file type
		 *
		 * @return The file type.
		 */
		const FileResource::DataType getFileType() const {
			return _type;
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
		* Sets a file
		*
		* @param file_ The file.
		* @param type_ (Optional) The type.
		*/
		void setFile(const std::string& file_, const FileResource::DataType& type_ = FileResource::DataType::eKtx) {
			_file = file_;
			_type = type_;
		};

	private:
		std::string _id;

		std::string _name;
		std::string _file;
		FileResource::DataType _type = FileResource::DataType::eKtx;
	};
}

TYPE_SHARED(asset::texture::ITexture, ITexture);
