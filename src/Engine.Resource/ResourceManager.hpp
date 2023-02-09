#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/NoThrow.hpp>

#include "ImporterManager.hpp"
#include "LoaderManager.hpp"
#include "Locator.hpp"

#ifdef _EDITOR
#include "Indexer/Indexer.hpp"
#endif

namespace ember::engine {
    class ResourceManager {
    public:
        using value_type = ResourceManager;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 30.08.2021
         */
        ResourceManager() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.08.2021
         */
        ~ResourceManager() noexcept;

    public:
        /**
         * Run setup routine for resource module
         *
         * @author Julius
         * @date 30.08.2021
         */
        void setup();

        void destroy();

        /**
         * Schedules this
         *
         * @author Julius
         * @date 22.09.2021
         */
        void schedule();

    private:
        uptr<res::ImporterManager> _importer;

    public:
        [[nodiscard]] cref<res::ImporterManager> importer() const;

        [[nodiscard]] ptr<const res::ImporterManager> importer(nothrow_t) const noexcept;

        [[nodiscard]] ref<res::ImporterManager> importer();

        [[nodiscard]] ptr<res::ImporterManager> importer(nothrow_t) noexcept;

        #ifdef _EDITOR

    private:
        uptr<res::Indexer> _indexer;

    public:
        [[nodiscard]] ptr<res::Indexer> indexer() const noexcept;
        #endif

    private:
        uptr<resource::LoaderManager> _loader;

    public:
        [[nodiscard]] cref<resource::LoaderManager> loader() const;

        [[nodiscard]] ptr<const resource::LoaderManager> loader(nothrow_t) const noexcept;

        [[nodiscard]] ref<resource::LoaderManager> loader();

        [[nodiscard]] ptr<resource::LoaderManager> loader(nothrow_t) noexcept;

    private:
        uptr<resource::Locator> _locator;

    public:
        [[nodiscard]] cref<resource::Locator> locator() const;

        [[nodiscard]] ptr<const resource::Locator> locator(nothrow_t) const noexcept;

        [[nodiscard]] ref<resource::Locator> locator();

        [[nodiscard]] ptr<resource::Locator> locator(nothrow_t) noexcept;
    };
}
