#pragma once
#include "Archive.hpp"

namespace ember::engine::serialization {

    class StructuredArchive : public Archive {
    public:
        using this_type = StructuredArchive;

    public:
        StructuredArchive() = default;

        ~StructuredArchive() override = default;
    };

}
