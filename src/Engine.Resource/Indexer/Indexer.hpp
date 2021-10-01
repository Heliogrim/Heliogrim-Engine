#pragma once

#include <set>
#include <Engine.Common/Collection/List.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Watcher.hpp"
#include "../File.hpp"
#include "../FileTypeId.hpp"

namespace ember::engine::res {

    class Indexer {
    public:
        using value_type = Indexer;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        Indexer() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        ~Indexer() noexcept;

    public:
        /**
         * Tidies this 
         *
         * @author Julius
         * @date 23.09.2021
         */
        void tidy();

    private:
        vector<File> _roots;

    public:
        /**
         * Adds a file as root node to indexer
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  root_ The root.
         */
        void addRoot(cref<File> root_) noexcept;

        /**
         * Removes a file used as root from indexer
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  root_ The root.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool removeRoot(cref<File> root_) noexcept;

    public:
        /**
         * Scans the given root to index files
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  root_ The root to scan.
         */
        void scan(cref<File> root_);

        /**
         * Scans the stored list of roots to index files
         *
         * @author Julius
         * @date 23.09.2021
         */
        void scan();

    private:
        vector<ptr<Watcher>> _watcher;

    public:
        /**
         * Watches the given file for any changes
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  root_ The root.
         */
        void watch(cref<File> root_);

        /**
         * Watches the stored list of roots for any changes
         *
         * @author Julius
         * @date 23.09.2021
         */
        void watch();

    private:
        _STD vector<_STD function<bool(cref<File>)>> _callbacks;
        _STD set<File> _ack;

    public:
        void on(cref<std::function<bool(cref<File> file_)>> callback_);

        void on(mref<std::function<bool(cref<File> file_)>> callback_);

    private:
        /**
         * Indexes the given file
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  file_ The file.
         */
        void reindex(cref<File> file_);

        /**
         * Removes the indexing for the given file
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @param  file_ The file.
         */
        void removeIndex(cref<File> file_);

    };
}
