#pragma once
#include "Archive.hpp"

namespace ember::engine::serialization {
    class __declspec(novtable) MemoryBaseArchive :
        public Archive {
    public:
        using this_type = MemoryBaseArchive;

    protected:
        MemoryBaseArchive();

    public:
        [[nodiscard]] string_view getArchiveName() const noexcept override;

    protected:
        s64 _pos;

    public:
        void seek(const s64 pos_) final;

        [[nodiscard]] s64 tell() const noexcept final;
    };
}
