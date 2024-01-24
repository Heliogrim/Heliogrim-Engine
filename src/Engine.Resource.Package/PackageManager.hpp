#pragma once
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include <ranges>

#include "PackageResource.hpp"
#include "Package/PackageGuid.hpp"

namespace hg::fs {
    class File;
}

namespace hg::engine::resource {
    class PackageManager {
    public:
        using this_type = PackageManager;

    public:
        PackageManager();

        ~PackageManager() noexcept;

    public:
        void tidy();

    public:
        [[nodiscard]] bool isPackageFile(cref<hg::fs::File> file_) const noexcept;

        [[nodiscard]] bool isPackageSource(const non_owning_rptr<Source> source_) const noexcept;

    private:
        RobinMap<PackageGuid, smr<PackageResource>> _packages;

        /* Temporary Solution */
    public:
        [[nodiscard]] inline auto packages() const noexcept {
            return std::ranges::views::values(_packages);
        }

        /* Temporary Solution */

    public:
        /**
         * Will insert a new package to the package mapping
         *
         * @param package_ The package to insert.
         */
        void addPackage(mref<smr<PackageResource>> package_);

        /**
         * Query the package related to the provided guid
         *
         * @param guid_ The guid to search for.
         *
         * @returns A smr<Ty_> of the package if any is present, otherwise and empty smr<Ty_>.
         */
        [[nodiscard]] _Success_(!return.empty()) smr<PackageResource> getPackage(
            cref<PackageGuid> guid_
        ) const noexcept;
    };
}
