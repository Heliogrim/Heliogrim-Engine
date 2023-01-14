#pragma once

#include "SourceBaseArchive.hpp"

namespace ember::engine::serialization {
    class SourceReadonlyArchive final :
        public SourceBaseArchive {
    public:
        using this_type = SourceReadonlyArchive;

    public:
        SourceReadonlyArchive(mref<smr<res::Source>> source_);

        ~SourceReadonlyArchive() override;

    public:
        void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
    };
}
