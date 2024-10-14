#pragma once

#include <type_traits>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>
#include <Engine.Reflect/Inherit/Concept.hpp>

#include "ArchiveStreamMode.hpp"
#include "ArchiveVersion.hpp"
#include "Compression/__fwd.hpp"
#include "Filter/__fwd.hpp"

namespace hg::engine::resource {
	class macro_novtable Archive {
	public:
		using this_type = Archive;

	protected:
		Archive() noexcept;

	public:
		virtual ~Archive() = default;

		#pragma region Meta Archive Attributes

	public:
		[[nodiscard]] inline bool shouldSwapBytes() const noexcept {
			return std::endian::native != std::endian::little;
		}
		#pragma endregion

		#pragma region Archive State

	protected:
		bool _error;

	public:
		[[nodiscard]] bool hasError() const noexcept;

		void setError() noexcept;

		void clearError() noexcept;

		#pragma endregion
		#pragma region Input / Output

	public:
		virtual void seek(const s64 pos_);

		[[nodiscard]] virtual s64 tell() const noexcept;

		[[nodiscard]] virtual s64 totalSize() const noexcept;

		[[nodiscard]] virtual bool isEOA() const noexcept;

	public:
		virtual void flush();

		[[nodiscard]] virtual bool close();

	protected:
		nmpt<ArchiveFilter> _filter;

	public:
		[[nodiscard]] bool hasFilter() const noexcept;

		[[nodiscard]] nmpt<ArchiveFilter> getArchiveFilter() const noexcept;

		void setFilter(nmpt<ArchiveFilter> filter_);

	protected:
		nmpt<ArchiveCompression> _compression;

	public:
		[[nodiscard]] bool hasCompression() const noexcept;

		[[nodiscard]] nmpt<ArchiveCompression> getArchiveCompression() const noexcept;

		void setCompression(nmpt<ArchiveCompression> compression_);

		#pragma endregion
		#pragma region Integrity

	protected:
		ArchiveVersion _version;

	public:
		[[nodiscard]] ArchiveVersion getArchiveVersion() const noexcept;

		void setArchiveVersion(cref<ArchiveVersion> version_) noexcept;

		#pragma endregion
		#pragma region Stream Serializer

	public:
		virtual void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) = 0;

		void serializeBytes(_Inout_ std::span<_::byte> data_, ArchiveStreamMode mode_) {
			this->serializeBytes(data_.data(), data_.size_bytes(), mode_);
		}

	public:
		void serializeBytesSwapped(ref<u16> value_, const ArchiveStreamMode mode_);

		void serializeBytesSwapped(ref<u32> value_, const ArchiveStreamMode mode_);

		void serializeBytesSwapped(ref<u64> value_, const ArchiveStreamMode mode_);

		void serializeBytesSwapped(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_);

	public:
		template <typename Type_>
		ref<this_type> serializeByteOrdered(Type_&& value_, const ArchiveStreamMode mode_) {

			static_assert(not std::is_signed_v<Type_>);

			if (not shouldSwapBytes()) {
				serializeBytes(&value_, sizeof(value_), mode_);
				return *this;
			}

			if constexpr (std::is_integral_v<std::remove_cvref_t<Type_>>) {
				serializeBytesSwapped(std::forward<Type_>(value_), mode_);
			} else {
				serializeBytesSwapped(&value_, sizeof(value_), mode_);
			}
			return *this;
		}

	public:
		virtual ref<this_type> operator<<(const ptr<ClassMetaBase> object_);

	public:
		template <ClassHasMeta Type_>
		inline friend ref<this_type> operator<<(ref<this_type> self_, const ptr<const Type_> object_) {
			return self_ << object_;
		}

		#pragma endregion
		#pragma region Input Serialization

	public:
		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<bool> value_) {
			u8 rep {};
			self_.serializeBytes(&rep, sizeof(rep), ArchiveStreamMode::eLoad);
			value_ = rep != 0u;
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<u8> value_) {
			self_.serializeBytes(&value_, sizeof(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<u16> value_) {
			self_.serializeByteOrdered(value_, ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<u32> value_) {
			self_.serializeByteOrdered(value_, ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<u64> value_) {
			self_.serializeByteOrdered(value_, ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<s8> value_) {
			self_.serializeBytes(&value_, sizeof(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<s16> value_) {
			self_.serializeByteOrdered(reinterpret_cast<ref<u16>>(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<s32> value_) {
			self_.serializeByteOrdered(reinterpret_cast<ref<u32>>(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<s64> value_) {
			self_.serializeByteOrdered(reinterpret_cast<ref<u64>>(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		START_SUPPRESS_WARNINGS

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<float> value_) {
			self_.serializeByteOrdered(reinterpret_cast<ref<u32>>(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<double> value_) {
			self_.serializeByteOrdered(reinterpret_cast<ref<u64>>(value_), ArchiveStreamMode::eLoad);
			return self_;
		}

		STOP_SUPPRESS_WARNINGS

		template <typename EnumType_> requires std::is_enum_v<EnumType_>
		inline friend ref<this_type> operator>>(ref<this_type> self_, ref<EnumType_> value_) {

			using enum_raw_type = std::underlying_type_t<EnumType_>;
			static_assert(
				std::is_integral_v<enum_raw_type>,
				"Enum is required to be based on an integral underlying type while serialization."
			);

			if constexpr (sizeof(enum_raw_type) == sizeof(u8)) {
				self_.serializeBytes(&value_, sizeof(u8), ArchiveStreamMode::eLoad);
			} else if constexpr (sizeof(enum_raw_type) == sizeof(u16)) {
				self_.serializeByteOrdered(reinterpret_cast<ref<u16>>(value_), ArchiveStreamMode::eLoad);
			} else if constexpr (sizeof(enum_raw_type) == sizeof(u32)) {
				self_.serializeByteOrdered(reinterpret_cast<ref<u32>>(value_), ArchiveStreamMode::eLoad);
			} else if constexpr (sizeof(enum_raw_type) == sizeof(u64)) {
				self_.serializeByteOrdered(reinterpret_cast<ref<u64>>(value_), ArchiveStreamMode::eLoad);
			} else {
				static_assert(
					sizeof(enum_raw_type) > sizeof(u64) || sizeof(enum_raw_type) == 0,
					"Failed to determine usable integral type for enum while serializing."
				);
			}

			return self_;
		}

		#pragma endregion
		#pragma region Output Serialization

	public:
		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<bool> value_) {
			u8 rep { value_ ? (u8 { 0x1u }) : (u8 { 0x0u }) };
			self_.serializeBytes(&rep, sizeof(rep), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<char> value_) {
			self_.serializeBytes(&const_cast<ref<char>>(value_), sizeof(value_), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<u8> value_) {
			self_.serializeBytes(&const_cast<ref<u8>>(value_), sizeof(value_), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<u16> value_) {
			self_.serializeByteOrdered(const_cast<ref<u16>>(value_), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<u32> value_) {
			self_.serializeByteOrdered(const_cast<ref<u32>>(value_), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<u64> value_) {
			self_.serializeByteOrdered(const_cast<ref<u64>>(value_), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<s8> value_) {
			self_.serializeBytes(&const_cast<ref<s8>>(value_), sizeof(value_), ArchiveStreamMode::eStore);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<s16> value_) {
			self_.serializeByteOrdered(
				reinterpret_cast<ref<u16>>(const_cast<ref<s16>>(value_)),
				ArchiveStreamMode::eStore
			);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<s32> value_) {
			self_.serializeByteOrdered(
				reinterpret_cast<ref<u32>>(const_cast<ref<s32>>(value_)),
				ArchiveStreamMode::eStore
			);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<s64> value_) {
			self_.serializeByteOrdered(
				reinterpret_cast<ref<u64>>(const_cast<ref<s64>>(value_)),
				ArchiveStreamMode::eStore
			);
			return self_;
		}

		START_SUPPRESS_WARNINGS

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<float> value_) {
			self_.serializeByteOrdered(
				// Note: This should be well defined as float* (-> u32*) -> char*
				reinterpret_cast<ref<u32>>(const_cast<ref<float>>(value_)),
				ArchiveStreamMode::eStore
			);
			return self_;
		}

		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<double> value_) {
			self_.serializeByteOrdered(
				reinterpret_cast<ref<u64>>(const_cast<ref<double>>(value_)),
				ArchiveStreamMode::eStore
			);
			return self_;
		}

		STOP_SUPPRESS_WARNINGS

		template <typename EnumType_> requires std::is_enum_v<EnumType_>
		inline friend ref<this_type> operator<<(ref<this_type> self_, cref<EnumType_> value_) {

			using enum_raw_type = std::underlying_type_t<EnumType_>;
			static_assert(
				std::is_integral_v<enum_raw_type>,
				"Enum is required to be based on an integral underlying type while serialization."
			);

			if constexpr (sizeof(enum_raw_type) == sizeof(u8)) {
				self_.serializeBytes(
					&const_cast<ref<EnumType_>>(value_),
					sizeof(u8),
					ArchiveStreamMode::eStore
				);

			} else if constexpr (sizeof(enum_raw_type) == sizeof(u16)) {
				self_.serializeByteOrdered(
					reinterpret_cast<ref<u16>>(const_cast<ref<EnumType_>>(value_)),
					ArchiveStreamMode::eStore
				);

			} else if constexpr (sizeof(enum_raw_type) == sizeof(u32)) {
				self_.serializeByteOrdered(
					reinterpret_cast<ref<u32>>(const_cast<ref<EnumType_>>(value_)),
					ArchiveStreamMode::eStore
				);

			} else if constexpr (sizeof(enum_raw_type) == sizeof(u64)) {
				self_.serializeByteOrdered(
					reinterpret_cast<ref<u64>>(const_cast<ref<EnumType_>>(value_)),
					ArchiveStreamMode::eStore
				);

			} else {
				static_assert(
					sizeof(enum_raw_type) > sizeof(u64) || sizeof(enum_raw_type) == 0,
					"Failed to determine usable integral type for enum while serializing."
				);
			}

			return self_;
		}
		#pragma endregion
	};
}
