#pragma once
#include <map>
#include <Engine.Common/Cast.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "FileTypeId.hpp"
#include "Importer/Importer.hpp"

namespace hg::engine::res {
    class ImporterManager {
    public:
        using value_type = ImporterManager;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        ImporterManager() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        ~ImporterManager() noexcept;

    private:
        using map_type = std::map<
            FileTypeId,
            sptr<ImporterBase>
        >;

        map_type _mapping;

    public:
        /**
         * Registers the given importer for the referenced fileTypeId_
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @param  fileTypeId_ Identifier for the file type.
         * @param  importer_ The importer.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool registerImporter(cref<FileTypeId> fileTypeId_, cref<sptr<ImporterBase>> importer_) noexcept;

        /**
         * Unregisters the given importer for all FileTypeIds
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @param  importer_ The importer.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool unregisterImporter(sptr<ImporterBase> importer_) noexcept;

        /**
         * Unregisters the importer for the referenced fileTypeId_
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @param  fileTypeId_ Identifier for the file type.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool unregisterImporter(cref<FileTypeId> fileTypeId_) noexcept;

    private:
        /**
         * Query for a suitable importer based on FileTypeId and File
         *
         * @author Julius
         * @date 22.09.2021
         *
         * @param  fileType_ Type of the file.
         * @param  file_ The file.
         *
         * @returns A ptr&lt;ImporterBase&gt;
         */
        [[nodiscard]] sptr<ImporterBase> importer(cref<FileTypeId> fileType_, cref<fs::File> file_) const;

    public:
        /**
         * Imports the given file
         *
         * @tparam ImportType_ Type of the import type.
         * @param  fileType_ Type of the file.
         * @param  file_ The file.
         *
         * @returns An Importer<ImportType_, void>::import_result_type.
         */
        template <class ImportType_>
        [[nodiscard]] typename Importer<ImportType_, void>::import_result_type import(
            cref<FileTypeId> fileType_,
            cref<fs::File> file_
        ) const {

            const sptr<ImporterBase> im = importer(fileType_, file_);

            if (im == nullptr) {
                throw _STD runtime_error("No suitable importer found for FileTypeId and File.");
            }

            return static_cast<const ptr<Importer<ImportType_, void>>>(im.get())->import(fileType_, file_);
        }
    };
}
