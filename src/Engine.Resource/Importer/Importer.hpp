#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "../File.hpp"
#include "../FileTypeId.hpp"

namespace ember::engine::res {
    class __declspec(novtable) ImporterBase {
    public:
        template <class ImportType_, class DescriptorType_>
        friend class Importer;

    public:
        using value_type = ImporterBase;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        ImporterBase() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        virtual ~ImporterBase() noexcept = default;

    public:
        /**
         * Determine if we can import
         *
         * @author Julius
         * @date 16.09.2021
         *
         * @param  typeId_ Identifier for the type.
         * @param  file_ The file.
         *
         * @returns True if we can import, false if not.
         */
        [[nodiscard]] virtual bool canImport(cref<FileTypeId> typeId_, cref<fs::File> file_) const noexcept = 0;
    };

    template <class ImportType_, class DescriptorType_>
    class __declspec(novtable) Importer :
        public ImporterBase {
    public:
        using base_type = ImporterBase;

        using value_type = Importer<ImportType_, DescriptorType_>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using descriptor_type = DescriptorType_;
        using import_type = ImportType_;

        template <typename Type_, typename = _STD enable_if_t<!_STD is_void_v<Type_>>>
        using wrapper_type = ember::concurrent::future<Type_>;
        using import_result_type = wrapper_type<import_type>;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 10.09.2021
         */
        Importer() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 10.09.2021
         */
        ~Importer() noexcept override = default;

    public:
        /**
         * Gets the descriptor of importer
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @returns A descriptor_type.
         */
        [[nodiscard]] virtual descriptor_type descriptor() const noexcept = 0;

        /**
         * Imports a file
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  typeId_ Identifier for the type.
         * @param  file_ The file.
         *
         * @returns An import_result_type.
         */
        [[nodiscard]] virtual import_result_type import(cref<FileTypeId> typeId_, cref<fs::File> file_) const = 0;
    };
}
