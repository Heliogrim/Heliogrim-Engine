#pragma once

#include <direct.h>
#include <filesystem>
#include <fstream>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::fs {
	/** Values that represent file types */
	enum FileType {
		eNone,
		eAsset,
		eMaterial,
		eTexture
	};

	class File {
	public:
		using path_type = std::filesystem::path;

	public:
		/**
		 * Default Constructor
		 */
		constexpr File();

		File(mref<path_type> path_) noexcept;

		File(cref<path_type> path_) noexcept;

		/**
		 * Constructor
		 *
		 * @param url_ The file.
		 */
		File(cref<string> url_);

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 16.09.2021
		 *
		 * @param  url_ URL_ of the resource.
		 */
		File(cref<string_view> url_);

		File(mref<File> other_) noexcept;

		File(cref<File> other_) noexcept;

		~File() noexcept;

	public:
		ref<File> operator=(mref<File> other_) noexcept;

		ref<File> operator=(cref<File> other_) noexcept;

	public:
		/**
		 * Gets the file
		 *
		 * @return The file.
		 */
		[[nodiscard]] cref<path_type> path() const;

		/**
		 * Check whether file or directory exists
		 *
		 * @return A const bool.
		 */
		[[nodiscard]] bool exists() const;

		/**
		 * Check whether URL is a directory or a file
		 *
		 * @return A const bool.
		 */
		[[nodiscard]] bool isDirectory() const;

		/**
		 * Gets the size
		 *
		 * @return A const size_t.
		 */
		[[nodiscard]] streamsize size() const;

		/**
		 * Gets the parent
		 *
		 * @return A const File.
		 */
		[[nodiscard]] File parent() const;

		/**
		 * Gets all subfiles from this directory
		 *
		 * @exception std::exception Thrown when file is not a directory or can not fetch the subfiles.
		 *
		 * @return A const std::vector&lt;File&gt;
		 */
		[[nodiscard]] Vector<File> files() const;

		/**
		 * Sets a file
		 *
		 * @param path_ The file.
		 */
		void setPath(cref<path_type> path_);

		/**
		 * Make directory
		 */
		void mkdir() const;

		/**
		 * Make directories
		 */
		void mkdirs() const;

		/**
		 * Creates the file
		 */
		void createFile() const;

		/**
		 * Moves the given destination
		 *
		 * @exception std::exception Thrown when an exception error condition occurs.
		 *
		 * @param dst_ Destination for the.
		 */
		void move(const File& dst_);

		/**
		 * Cast that converts the given  to a string
		 *
		 * @return The result of the operation.
		 */
		[[nodiscard]] operator string() const noexcept;

		const static char seperator = '\\';

	protected:
		std::filesystem::path _path;

	public:
		[[nodiscard]] bool operator==(cref<File> other_) const noexcept;

		[[nodiscard]] bool operator!=(cref<File> other_) const noexcept;
	};
}

namespace std {
	template <>
	struct hash<hg::fs::File> {
		[[nodiscard]] size_t operator()(const hg::fs::File& value_) const noexcept {
			return std::hash<string> {}(value_.path().string());
		}
	};

	template <>
	struct less<hg::fs::File> {
		[[nodiscard]] bool operator()(const hg::fs::File& left_, const hg::fs::File& right_) const noexcept {
			return std::less<std::filesystem::path> {}(left_.path(), right_.path());
		}
	};
}
