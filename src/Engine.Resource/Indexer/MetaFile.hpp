#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../File.hpp"

namespace ember::engine::res {

    class MetaFile {
    public:
        using value_type = MetaFile;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  file_ The file.
         */
        MetaFile(cref<File> file_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        ~MetaFile() noexcept;

    public:
        [[nodiscard]] static MetaFile of(cref<File> file_);

    private:
        File _file;

    private:
        char _checksum[256];

    public:
        [[nodiscard]] vector<asset_guid> referencedBy() const;
    };
}
