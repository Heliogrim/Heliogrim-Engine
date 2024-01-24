#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.Filesystem/Url.hpp>

#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>
#include <Engine.Reflect/Inherit/Concept.hpp>

#include "../Compression/__fwd.hpp"
#include "../Filter/__fwd.hpp"
#include "ArchiveStreamMode.hpp"
#include "ArchiveVersion.hpp"

namespace hg::engine::serialization {
    class __declspec(novtable) Archive {
    public:
        using this_type = Archive;

    protected:
        Archive() noexcept;

    public:
        virtual ~Archive() = default;

        #pragma region Meta Archive Attributes

    public:
        [[nodiscard]] FORCE_INLINE bool shouldSwapBytes() const noexcept {
            #if ENV_MSVC
            return false;
            #else
            return true;
            #endif
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
        #pragma region Naming / Locating

    public:
        [[nodiscard]] virtual fs::Url getArchiveUrl() const noexcept = 0;

        [[nodiscard]] virtual string_view getArchiveName() const noexcept = 0;

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
        ptr<ArchiveFilter> _filter;

    public:
        [[nodiscard]] bool hasFilter() const noexcept;

        [[nodiscard]] const ptr<ArchiveFilter> getArchiveFilter() const noexcept;

        void setFilter(const ptr<ArchiveFilter> filter_);

    protected:
        ptr<ArchiveCompression> _compression;

    public:
        [[nodiscard]] bool hasCompression() const noexcept;

        [[nodiscard]] const ptr<ArchiveCompression> getArchiveCompression() const noexcept;

        void setCompression(const ptr<ArchiveCompression> compression_);

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

    public:
        void serializeBytesSwapped(ref<u16> value_, const ArchiveStreamMode mode_);

        void serializeBytesSwapped(ref<u32> value_, const ArchiveStreamMode mode_);

        void serializeBytesSwapped(ref<u64> value_, const ArchiveStreamMode mode_);

        void serializeBytesSwapped(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_);

    public:
        template <typename Type_>
        ref<this_type> serializeByteOrdered(Type_&& value_, const ArchiveStreamMode mode_) {

            static_assert(not std::is_signed_v<Type_>, "");

            if (not shouldSwapBytes()) {
                serializeBytes(&value_, sizeof(value_), mode_);
                return *this;
            }

            if constexpr (std::is_integral_v<Type_>) {
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
        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, const ptr<const Type_> object_) {
            return self_ << object_;
        }

        #pragma endregion
        #pragma region Input Serialization

    public:
        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<bool> value_) {
            u8 rep {};
            self_.serializeBytes(&rep, sizeof(rep), ArchiveStreamMode::eOut);
            value_ = rep != 0ui8;
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<u8> value_) {
            self_.serializeBytes(&value_, sizeof(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<u16> value_) {
            self_.serializeByteOrdered(value_, ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<u32> value_) {
            self_.serializeByteOrdered(value_, ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<u64> value_) {
            self_.serializeByteOrdered(value_, ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<s8> value_) {
            self_.serializeBytes(&value_, sizeof(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<s16> value_) {
            self_.serializeByteOrdered(reinterpret_cast<ref<u16>>(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<s32> value_) {
            self_.serializeByteOrdered(reinterpret_cast<ref<u32>>(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<s64> value_) {
            self_.serializeByteOrdered(reinterpret_cast<ref<u64>>(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<float> value_) {
            self_.serializeByteOrdered(reinterpret_cast<ref<u32>>(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<double> value_) {
            self_.serializeByteOrdered(reinterpret_cast<ref<u64>>(value_), ArchiveStreamMode::eOut);
            return self_;
        }

        template <typename EnumType_> requires std::is_enum_v<EnumType_>
        FORCE_INLINE friend ref<this_type> operator>>(ref<this_type> self_, ref<EnumType_> value_) {

            using enum_raw_type = std::underlying_type_t<EnumType_>;
            static_assert(
                std::is_integral_v<enum_raw_type>,
                "Enum is required to be based on an integral underlying type while serialization."
            );

            if constexpr (sizeof(enum_raw_type) == sizeof(u8)) {
                self_.serializeBytes(&value_, sizeof(u8), ArchiveStreamMode::eOut);
            } else if constexpr (sizeof(enum_raw_type) == sizeof(u16)) {
                self_.serializeByteOrdered(reinterpret_cast<ref<u16>>(value_), ArchiveStreamMode::eOut);
            } else if constexpr (sizeof(enum_raw_type) == sizeof(u32)) {
                self_.serializeByteOrdered(reinterpret_cast<ref<u32>>(value_), ArchiveStreamMode::eOut);
            } else if constexpr (sizeof(enum_raw_type) == sizeof(u64)) {
                self_.serializeByteOrdered(reinterpret_cast<ref<u64>>(value_), ArchiveStreamMode::eOut);
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
        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<bool> value_) {
            u8 rep { value_ ? 0x1ui8 : 0x0ui8 };
            self_.serializeBytes(&rep, sizeof(rep), ArchiveStreamMode::eIn);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<u8> value_) {
            self_.serializeBytes(&const_cast<ref<u8>>(value_), sizeof(value_), ArchiveStreamMode::eIn);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<u16> value_) {
            self_.serializeByteOrdered(const_cast<ref<u16>>(value_), ArchiveStreamMode::eIn);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<u32> value_) {
            self_.serializeByteOrdered(const_cast<ref<u32>>(value_), ArchiveStreamMode::eIn);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<u64> value_) {
            self_.serializeByteOrdered(const_cast<ref<u64>>(value_), ArchiveStreamMode::eIn);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<s8> value_) {
            self_.serializeBytes(&const_cast<ref<s8>>(value_), sizeof(value_), ArchiveStreamMode::eIn);
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<s16> value_) {
            self_.serializeByteOrdered(
                reinterpret_cast<ref<u16>>(const_cast<ref<s16>>(value_)),
                ArchiveStreamMode::eIn
            );
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<s32> value_) {
            self_.serializeByteOrdered(
                reinterpret_cast<ref<u32>>(const_cast<ref<s32>>(value_)),
                ArchiveStreamMode::eIn
            );
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<s64> value_) {
            self_.serializeByteOrdered(
                reinterpret_cast<ref<u64>>(const_cast<ref<s64>>(value_)),
                ArchiveStreamMode::eIn
            );
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<float> value_) {
            self_.serializeByteOrdered(
                reinterpret_cast<ref<u32>>(const_cast<ref<float>>(value_)),
                ArchiveStreamMode::eIn
            );
            return self_;
        }

        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<double> value_) {
            self_.serializeByteOrdered(
                reinterpret_cast<ref<u64>>(const_cast<ref<double>>(value_)),
                ArchiveStreamMode::eIn
            );
            return self_;
        }

        template <typename EnumType_> requires std::is_enum_v<EnumType_>
        FORCE_INLINE friend ref<this_type> operator<<(ref<this_type> self_, cref<EnumType_> value_) {

            using enum_raw_type = std::underlying_type_t<EnumType_>;
            static_assert(
                std::is_integral_v<enum_raw_type>,
                "Enum is required to be based on an integral underlying type while serialization."
            );

            if constexpr (sizeof(enum_raw_type) == sizeof(u8)) {
                self_.serializeBytes(
                    &const_cast<ref<EnumType_>>(value_),
                    sizeof(u8),
                    ArchiveStreamMode::eIn
                );

            } else if constexpr (sizeof(enum_raw_type) == sizeof(u16)) {
                self_.serializeByteOrdered(
                    reinterpret_cast<ref<u16>>(const_cast<ref<EnumType_>>(value_)),
                    ArchiveStreamMode::eIn
                );

            } else if constexpr (sizeof(enum_raw_type) == sizeof(u32)) {
                self_.serializeByteOrdered(
                    reinterpret_cast<ref<u32>>(const_cast<ref<EnumType_>>(value_)),
                    ArchiveStreamMode::eIn
                );

            } else if constexpr (sizeof(enum_raw_type) == sizeof(u64)) {
                self_.serializeByteOrdered(
                    reinterpret_cast<ref<u64>>(const_cast<ref<EnumType_>>(value_)),
                    ArchiveStreamMode::eIn
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
