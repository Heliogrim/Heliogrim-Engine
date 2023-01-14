#pragma once

#include <Engine.Common/Collection/AutoArray.hpp>

#include "MemoryArchive.hpp"

namespace ember::engine::serialization {
    class MemoryReadWriteArchive :
        public MemoryBaseArchive {
    public:
        using this_type = MemoryReadWriteArchive;

    public:
        MemoryReadWriteArchive(_Inout_ ref<AutoArray<u8>> bytes_);

        MemoryReadWriteArchive(_Inout_ ref<AutoArray<u8>> bytes_, const s64 limit_);

    public:
        [[nodiscard]] Url getArchiveUrl() const noexcept override;

    protected:
        ref<AutoArray<u8>> _bytes;
        s64 _limit;

    public:
        [[nodiscard]] s64 totalSize() const noexcept override;

        [[nodiscard]] bool hasSizeLimit() const noexcept;

        [[nodiscard]] s64 sizeLimit() const noexcept;

        void setSizeLimit(const s64 limit_);

        void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode) override;
    };
}
