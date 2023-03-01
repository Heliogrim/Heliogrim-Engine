#pragma once
#include "MemoryReadWriteArchive.hpp"

namespace hg::engine::serialization {
    class BufferArchive :
        public MemoryReadWriteArchive,
        public AutoArray<u8> {
    public:
        using this_type = BufferArchive;

    public:
        BufferArchive();

    public:
        [[nodiscard]] string_view getArchiveName() const noexcept override;
    };
}
