#pragma once
#include <Engine.Resource/Importer/Importer.hpp>

#include "SoundImporterDescriptor.hpp"
#include "SoundImportType.hpp"

namespace ember::engine::sfx {

    class SoundImporter :
        public res::Importer<SoundImportType, SoundImporterDescriptor> {
    public:
        using base_type = Importer<SoundImportType, SoundImporterDescriptor>;

        using value_type = SoundImporter;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using base_type::descriptor_type;
        using base_type::import_result_type;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        SoundImporter() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        ~SoundImporter() noexcept override = default;
    };
}
