#pragma once

#include <span>

#include "MemoryArchive.hpp"

namespace hg::engine::serialization {
    class MemoryReadonlyArchive :
        public MemoryBaseArchive {
    public:
        using this_type = MemoryReadonlyArchive;
        using underlying_type = MemoryBaseArchive;

    public:
        MemoryReadonlyArchive(_In_ ref<std::span<u8, std::dynamic_extent>> bytes_);

        MemoryReadonlyArchive(_In_ ref<std::span<u8, std::dynamic_extent>> bytes_, const s64 limit_);

    public:
        [[nodiscard]] fs::Url getArchiveUrl() const noexcept override;

    protected:
        ref<std::span<u8, std::dynamic_extent>> _bytes;
        s64 _limit;

    public:
        [[nodiscard]] s64 totalSize() const noexcept override;

        [[nodiscard]] bool hasSizeLimit() const noexcept;

        [[nodiscard]] s64 sizeLimit() const noexcept;

        void setSizeLimit(const s64 limit_);

        void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
    };
}
