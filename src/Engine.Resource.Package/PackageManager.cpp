#include "PackageManager.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.Logging/Logger.hpp>

#include <Engine.Resource/File.hpp>
#include <Engine.Resource/Source/Source.hpp>

#include <Engine.Resource/Source/FileSource.hpp>
#include "Package/MagicBytes.hpp"

using namespace hg::engine::resource;
using namespace hg;

[[nodiscard]] static string to_string(cref<PackageGuid> guid_) {
    return _STD format(R"({:08x}-{:04x}-{:04x}-{:016x})", guid_.pre, guid_.c0, guid_.c1, guid_.post);
}

PackageManager::PackageManager() = default;

PackageManager::~PackageManager() noexcept {
    tidy();
}

void PackageManager::tidy() {

    for (const auto& entry : _packages) {

        auto& package = entry.second;
        if (package.refs() > 1) {
            IM_CORE_ERRORF(
                "Failed to cleanup PackageManager with package `{}` cause it is still shared `{}` times.",
                to_string(entry.first),
                package.refs()
            );
            continue;
        }

        auto dummy = _STD move(const_cast<ref<smr<PackageResource>>>(package));
        dummy.reset();
    }

    _packages.clear();
}

bool PackageManager::isPackageFile(cref<fs::File> file_) const noexcept {

    const auto extension = file_.path().extension();
    if (not extension.empty()) {

        const auto str = extension.string();
        if (not str.ends_with(".imp") && not str.ends_with(".impackage")) {
            return false;
        }

    }

    /**/

    auto source = FileSource(fs::File { file_ });
    return isPackageSource(&source);
}

bool PackageManager::isPackageSource(const non_owning_rptr<Source> source_) const noexcept {

    _::byte buffer[sizeof(PackageHeader)];

    streamsize read;
    source_->get(0, sizeof(PackageHeader), buffer, read);

    if (read < sizeof(PackageHeader)) {
        return false;
    }

    /**/

    // TODO: Check whether we want to check more than just the magic sequence.
    const ptr<const PackageHeader> header = reinterpret_cast<ptr<const PackageHeader>>(buffer);
    return memcmp(header->magicBytes, PackageMagicBytes, sizeof(PackageMagicBytes)) == 0;
}

void PackageManager::addPackage(mref<smr<PackageResource>> package_) {

    #ifdef _DEBUG
    assert(not package_.empty());
    #endif

    const auto guard = package_->acquire(ResourceUsageFlag::eRead);
    const auto packageGuid = PackageGuid { guard->header().guid };

    const auto entry = _packages.find(packageGuid);

    if (entry != _packages.end()) {
        IM_CORE_WARN("Failed to add package, cause guid already exists.");
        return;
    }

    _packages.insert_or_assign(packageGuid, _STD move(package_));
}

smr<PackageResource> PackageManager::getPackage(cref<PackageGuid> guid_) const noexcept {
    const auto entry = _packages.find(guid_);
    if (entry != _packages.end()) {
        return { entry.value() };
    }

    return {};
}
