#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Session/Session.hpp>

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
         *
         * @param  session_ The session.
         */
        ResourceManager(cref<ptr<Session>> session_) noexcept;

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

        /**
         * Schedules this
         *
         * @author Julius
         * @date 22.09.2021
         */
        void schedule();

    private:
        /**
         * The session this module is associated with
         */
        ptr<Session> _session;

    public:
        /**
         * Gets the session this module is associated with
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A ptr<Session>
         */
        [[nodiscard]] ptr<Session> session() const noexcept;

    private:
        uptr<res::ImporterManager> _importer;

    public:
        [[nodiscard]] cref<res::ImporterManager> importer() const;

        [[nodiscard]] ptr<const res::ImporterManager> importer(_STD nothrow_t) const noexcept;

        [[nodiscard]] ref<res::ImporterManager> importer();

        [[nodiscard]] ptr<res::ImporterManager> importer(_STD nothrow_t) noexcept;

        #ifdef _EDITOR
    private:
        uptr<res::Indexer> _indexer;

    public:
        [[nodiscard]] ptr<res::Indexer> indexer() const noexcept;
        #endif

    private:
        uptr<res::LoaderManager> _loader;

    public:
        [[nodiscard]] cref<res::LoaderManager> loader() const;

        [[nodiscard]] ptr<const res::LoaderManager> loader(_STD nothrow_t) const noexcept;

        [[nodiscard]] ref<res::LoaderManager> loader();

        [[nodiscard]] ptr<res::LoaderManager> loader(_STD nothrow_t) noexcept;

    private:
        uptr<res::Locator> _locator;

    public:
        [[nodiscard]] cref<res::Locator> locator() const;

        [[nodiscard]] ptr<const res::Locator> locator(_STD nothrow_t) const noexcept;

        [[nodiscard]] ref<res::Locator> locator();

        [[nodiscard]] ptr<res::Locator> locator(_STD nothrow_t) noexcept;
    };
}
