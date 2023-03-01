#pragma once
#include "SoundImporter.hpp"

namespace hg::engine::sfx {
    class WavImporter :
        public SoundImporter {
    public:
        using base_type = SoundImporter;

        using value_type = WavImporter;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using base_type::descriptor_type;
        using base_type::import_result_type;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        WavImporter() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        ~WavImporter() noexcept override = default;

    public:
        [[nodiscard]] bool canImport(cref<res::FileTypeId> typeId_, cref<fs::File> file_) const noexcept override;

        [[nodiscard]] descriptor_type descriptor() const noexcept override;

        [[nodiscard]] import_result_type import(cref<res::FileTypeId> typeId_, cref<fs::File> file_) const override;
    };
}
