#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "LinkedArchive.hpp"
#include "__fwd.hpp"
#include "../Package.hpp"

namespace ember::engine::resource {
    class PackageLinker {
    public:
        friend class Package;

        template <bool Const_>
        friend class LinkedArchiveIterator;

    public:
        using this_type = PackageLinker;

        using iterator_type = LinkedArchiveIterator<false>;
        using const_iterator_type = LinkedArchiveIterator<true>;

    protected:
        PackageLinker(const non_owning_rptr<Package> package_);

    public:
        ~PackageLinker();

    private:
        const non_owning_rptr<Package> _package;

    public:
        [[nodiscard]] const non_owning_rptr<const Package> getPackage() const noexcept;

    private:
        Vector<LinkedArchive> _links;

    public:
        bool store(mref<ArchiveHeader> header_, mref<uptr<serialization::Archive>> archive_);

        [[nodiscard]] uptr<serialization::SourceBaseArchive> load(
            const Guid archiveGuid_
        ) const noexcept;

    public:
        [[nodiscard]] bool containsArchive(const Guid archiveGuid_) const noexcept;

        [[nodiscard]] u64 getArchiveCount() const noexcept;

    public:
        [[deprecated]] void remove(const Guid archiveGuid_);

    public:
        [[nodiscard]] const_iterator_type begin() const noexcept;

        [[nodiscard]] iterator_type begin() noexcept;

        [[nodiscard]] const_iterator_type cbegin() const noexcept;

        [[nodiscard]] const_iterator_type end() const noexcept;

        [[nodiscard]] iterator_type end() noexcept;

        [[nodiscard]] const_iterator_type cend() const noexcept;

    protected:
        [[nodiscard]] uptr<serialization::SourceBaseArchive> operator[](const_iterator_type where_) const noexcept;

    public:
        [[deprecated]] bool read();

        [[deprecated]] bool write();

    private:
        [[deprecated]] void readPackageIndex();

        [[deprecated]] void writePackageIndex();
    };
}
