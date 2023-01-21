#pragma once

#include <Engine.Resource/Source/Source.hpp>

#include "__fwd.hpp"
#include "../Linker/__fwd.hpp"

#include "PackageHeader.hpp"
#include "PackageMeta.hpp"
#include "PackageFooter.hpp"

namespace ember::engine::resource {
    class Package {
    public:
        friend class PackageLinker;

    public:
        using this_type = Package;

    public:
        Package(
            mref<uptr<res::Source>> source_,
            mref<PackageHeader> header_,
            mref<PackageFooter> footer_
        );

        ~Package();

    private:
        PackageHeader _header;
        PackageMeta _meta;
        PackageFooter _footer;

    public:
        [[nodiscard]] ref<PackageHeader> header() noexcept;

        [[nodiscard]] ref<PackageFooter> footer() noexcept;

    private:
        uptr<res::Source> _source;
        uptr<PackageLinker> _linker;

    public:
        [[nodiscard]] const non_owning_rptr<PackageLinker> getLinker() const noexcept;

    public:
        void unsafeReleaseSource(_Out_ ref<uptr<res::Source>> dst_);

    public:
        void unsafeWrite();

        void unsafeWriteHeader();

        void unsafeWriteIndex(streamoff pos_);

        void unsafeWriteMeta(streamoff pos_);

        void unsafeWriteFooter(streamoff pos_);
    };
}
