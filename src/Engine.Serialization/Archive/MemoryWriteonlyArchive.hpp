#pragma once

#include <Engine.Common/Collection/AutoArray.hpp>

#include "MemoryArchive.hpp"

namespace ember::engine::serialization {

    class MemoryWriteonlyArchive :
        public MemoryArchive {
    public:
        using this_type = MemoryWriteonlyArchive;
        using underlying_type = MemoryArchive;

    public:
        MemoryWriteonlyArchive(_Inout_ ref<AutoArray<u8>> bytes_);

    public:
        [[nodiscard]] Url getArchiveUrl() const noexcept override;

    protected:
        ref<AutoArray<u8>> _bytes;

    public:
        [[nodiscard]] s64 totalSize() const noexcept override;

        void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
    };

}
