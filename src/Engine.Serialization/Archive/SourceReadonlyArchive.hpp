#pragma once

#include "SourceBaseArchive.hpp"

namespace hg::engine::serialization {
    class SourceReadonlyArchive final :
        public SourceBaseArchive {
    public:
        using this_type = SourceReadonlyArchive;

    public:
        SourceReadonlyArchive(
            mref<smr<resource::Source>> source_,
            mref<streamoff> srcOff_,
            mref<streamsize> srcSize_
        );

        ~SourceReadonlyArchive() override;

    public:
        void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
    };
}
