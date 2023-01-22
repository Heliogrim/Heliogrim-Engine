#pragma once

#include "SourceBaseArchive.hpp"

namespace ember::engine::serialization {
    class SourceWriteonlyArchive final :
        public SourceBaseArchive {
    public:
        using this_type = SourceWriteonlyArchive;

    public:
        SourceWriteonlyArchive(
            mref<smr<res::Source>> source_,
            mref<streamoff> srcOff_,
            mref<streamsize> srcSize_
        );

        ~SourceWriteonlyArchive() override;

    public:
        void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
    };
}
