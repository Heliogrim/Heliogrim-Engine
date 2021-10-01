#pragma once
#include <Engine.Common/Cast.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/HopScotch.hpp>

#include "FileTypeId.hpp"
#include "Importer/Importer.hpp"

namespace ember::engine::res {

    namespace {

        struct fileType_importer_pair {
            FileTypeId fileTypeId;
            ptr<ImporterBase> importer;
        };

        struct fileType_importer_hasher :
            private _STD hash<FileTypeId> {
            [[nodiscard]] _STD size_t operator()(cref<fileType_importer_pair> value_) const {
                return static_cast<cref<_STD hash<FileTypeId>>>(*this)(value_.fileTypeId);
            }
        };

        struct fileType_importer_equal :
            private _STD equal_to<FileTypeId> {
            [[nodiscard]] bool operator()(cref<fileType_importer_pair> left_,
                cref<fileType_importer_pair> right_) const {
                return static_cast<cref<_STD equal_to<FileTypeId>>>(*this)(left_.fileTypeId, right_.fileTypeId);
            }
        };

        struct fileType_importer_less :
            private _STD less<FileTypeId> {
            [[nodiscard]] bool operator()(cref<fileType_importer_pair> left_,
                cref<fileType_importer_pair> right_) const {
                return static_cast<cref<_STD less<FileTypeId>>>(*this)(left_.fileTypeId, right_.fileTypeId);
            }
        };
    }

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
        /**
         * Tidy this up
         *
         * @author Julius
         * @date 12.09.2021
         */
        void tidy();

    private:
        using map_type = hopscotch_set<
            fileType_importer_pair, fileType_importer_hasher, fileType_importer_equal, fileType_importer_less,
            _STD allocator<fileType_importer_pair>, 4, true
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
        bool registerImporter(cref<FileTypeId> fileTypeId_, ptr<ImporterBase> importer_) noexcept;

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
        bool unregisterImporter(ptr<ImporterBase> importer_) noexcept;

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
        [[nodiscard]] ptr<ImporterBase> importer(cref<FileTypeId> fileType_, cref<File> file_) const;

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
        [[nodiscard]] typename Importer<ImportType_, void>::import_result_type import(cref<FileTypeId> fileType_,
            cref<File> file_) const {

            const ptr<ImporterBase> im = importer(fileType_, file_);

            if (im == nullptr) {
                throw _STD runtime_error("No suitable importer found for FileTypeId and File.");
            }

            return static_cast<const ptr<Importer<ImportType_, void>>>(im)->import(fileType_, file_);
        }
    };
}
