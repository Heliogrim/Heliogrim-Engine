#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg::engine::res {
	class FileTypeRegister;

	/**
	 * TODO: replace magic number with identifier callback, cause can be splitted or multiple can occure for special types (e.g. mp3, wav/riff)
	 */
	struct FileTypeId {
		FileTypeId() noexcept {}

		explicit FileTypeId(cref<type_id> typeId_, cref<string> ext_, const bool register_ = true);

		[[nodiscard]] bool valid() const noexcept;

		[[nodiscard]] bool operator>(const FileTypeId& other_) const noexcept {
			return typeId > other_.typeId;
		}

		[[nodiscard]] bool operator<(const FileTypeId& other_) const noexcept {
			return typeId < other_.typeId;
		}

		[[nodiscard]] bool operator>=(const FileTypeId& other_) const noexcept {
			return typeId >= other_.typeId;
		}

		[[nodiscard]] bool operator<=(const FileTypeId& other_) const noexcept {
			return typeId <= other_.typeId;
		}

		[[nodiscard]] bool operator==(cref<FileTypeId> other_) const noexcept {
			return typeId == other_.typeId;
		}

		[[nodiscard]] bool operator!=(cref<FileTypeId> other_) const noexcept {
			return typeId != other_.typeId;
		}

		type_id typeId;
		string ext;
	};

	class FileTypeRegister {
	private:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 23.09.2021
		 */
		FileTypeRegister() noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 23.09.2021
		 */
		~FileTypeRegister() noexcept;

	private:
		static ptr<FileTypeRegister> _instance;

	public:
		/**
		 * Gets a reference to the static internal stored instance
		 *
		 * @author Julius
		 * @date 23.09.2021
		 *
		 * @returns A cref&lt;FileTypeRegister&gt;
		 */
		[[nodiscard]] static ref<FileTypeRegister> make();

		[[nodiscard]] static ref<FileTypeRegister> get();

		[[nodiscard]] static const ptr<FileTypeRegister> get(std::nothrow_t) noexcept;

		static void destroy();

	private:
		Vector<FileTypeId> _list;

	public:
		[[nodiscard]] FileTypeId getByTypeId(cref<type_id> id_) const noexcept;

		[[nodiscard]] FileTypeId getByExt(string ext_) const noexcept;

		[[nodiscard]] FileTypeId getByExt(string_view ext_) const noexcept;

	public:
		void registerType(cref<FileTypeId> type_);
	};
}

namespace std {
	template <>
	struct hash<hg::engine::res::FileTypeId> :
		public std::hash<hg::type_id> {
		[[nodiscard]] size_t operator()(const hg::engine::res::FileTypeId& value_) const noexcept {
			return static_cast<const std::hash<hg::type_id>*>(this)->operator()(value_.typeId);
		}
	};
}
