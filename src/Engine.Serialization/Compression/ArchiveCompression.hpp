#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::serialization {
    class __declspec(novtable) ArchiveCompression {
    public:
        using this_type = ArchiveCompression;

    protected:
        ArchiveCompression() = default;

    public:
        virtual ~ArchiveCompression() = default;

    private:
        ptr<ArchiveCompression> _next;

    public:
        [[nodiscard]] const ptr<ArchiveCompression> next() const noexcept;

        void setNext(const ptr<ArchiveCompression> next_) noexcept;
    };
}
