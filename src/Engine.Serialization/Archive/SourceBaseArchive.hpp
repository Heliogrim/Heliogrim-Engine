#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Source/Source.hpp>

#include "__fwd.hpp"
#include "Archive.hpp"

namespace ember::engine::serialization {
    class __declspec(novtable) SourceBaseArchive :
        public Archive {
    public:
        using this_type = SourceBaseArchive;

    protected:
        SourceBaseArchive(
            mref<smr<res::Source>> source_,
            mref<streamoff> srcOff_,
            mref<streamsize> srcSize_
        );

    public:
        ~SourceBaseArchive() noexcept override;

    public:
        [[nodiscard]] Url getArchiveUrl() const noexcept override;

        [[nodiscard]] string_view getArchiveName() const noexcept override;

    protected:
        smr<res::Source> _source;
        streamoff _srcOff;
        streamsize _srcSize;

    public:
        [[nodiscard]] smr<res::Source> getSource() const noexcept;

    protected:
        s64 _pos;

    public:
        void seek(const s64 pos_) override;

        [[nodiscard]] s64 tell() const noexcept override;
    };
}
