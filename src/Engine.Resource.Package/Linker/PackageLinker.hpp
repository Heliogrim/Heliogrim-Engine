#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "LinkedArchive.hpp"
#include "__fwd.hpp"
#include "../Package/Package.hpp"

namespace hg::engine::resource {
    class PackageLinker {
    public:
        friend class Package;

        template <bool Const_>
        friend class LinkedArchiveIterator;

    public:
        using this_type = PackageLinker;

        using iterator_type = LinkedArchiveIterator<false>;
        using const_iterator_type = LinkedArchiveIterator<true>;

        using iter_archive = serialization::SourceBaseArchive;
        using readonly_iter_archive = serialization::SourceReadonlyArchive;

    protected:
        PackageLinker(const non_owning_rptr<Package> package_);

    public:
        ~PackageLinker();

    private:
        const non_owning_rptr<Package> _package;

    public:
        [[nodiscard]] const non_owning_rptr<const Package> getPackage() const noexcept;

        [[nodiscard]] bool canLoad() const noexcept;

        [[nodiscard]] bool canStore() const noexcept;

    private:
        Vector<LinkedArchive> _links;

    protected:
        void restoreLinks();

    public:
        bool store(mref<ArchiveHeader> header_, mref<uptr<serialization::Archive>> archive_);

        bool store(_STD initializer_list<_STD pair<ArchiveHeader, uptr<serialization::Archive>>> archives_);

        [[nodiscard]] uptr<serialization::SourceReadonlyArchive> load(
            const Guid archiveGuid_
        ) const noexcept;

    public:
        [[nodiscard]] bool containsArchive(const Guid archiveGuid_) const noexcept;

        [[nodiscard]] u64 getArchiveCount() const noexcept;

        [[nodiscard]] s64 getArchiveSize() const noexcept;

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
        [[nodiscard]] uptr<readonly_iter_archive> operator[](const_iterator_type where_) const noexcept;

        [[nodiscard]] uptr<iter_archive> operator[](iterator_type where_) const noexcept;
    };
}
