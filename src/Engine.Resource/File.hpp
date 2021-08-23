#pragma once

#include <direct.h>
#include <fstream>

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/List.hpp>

namespace ember {
    /** Values that represent file types */
    enum FileType {
        eNone,
        eAsset,
        eMaterial,
        eTexture
    };

    class File {
    public:

        /** Default constructor */
        File() :
            _url(std::string()) { }

        /**
         * Constructor
         *
         * @param url_ The file.
         */
        File(const std::string& url_) :
            _url(url_) { }

        /**
         * Gets the file
         *
         * @return The file.
         */
        std::string url() const {
            return _url;
        }

        /**
         * Check whether file exists
         *
         * @return A const bool.
         */
        bool exists() const {
            // TODO: check existents even for directories
            struct stat buffer;
            return (stat(_url.c_str(), &buffer) == 0);
        }

        /**
         * Check whether URL is a directory or a file
         *	Could be problematic cause binary handle is equal between directory and file
         *
         * @return A const bool.
         */
        bool isDirectory() const {
            struct stat buffer;
            stat(_url.c_str(), &buffer);
            return (buffer.st_mode & S_IFDIR);
        }

        /**
         * Gets the size
         *
         * @return A const size_t.
         */
        size_t size() const {
            // TODO: evaluate size of file or directory
            return 0;
        }

        /**
         * Gets the parent
         *
         * @return A const File.
         */
        File parent() const {
            const size_t posx = _url.find_last_of(seperator);
            const std::string parentUrl = _url.substr(0, posx);
            return File(parentUrl);
        }

        /**
         * Gets all subfiles from this directory
         *
         * @exception std::exception Thrown when file is not a directory or can not fetch the subfiles.
         *
         * @return A const std::vector&lt;File&gt;
         */
        [[nodiscard]] std::vector<File> files() const {
            // Could return without effect
            if (!isDirectory())
                throw std::exception("URL is not a directory. Can not fetch files.");

            std::vector<File> files = std::vector<File>(0);

            return files;
        }

        /**
         * Sets a file
         *
         * @param url_ The file.
         */
        void setUrl(const std::string& url_) {
            _url = url_;
        }

        /**
         * Make directory
         *
         * @return A const void.
         */
        void mkdir() const {
            if (exists())
                throw std::exception("Directory already exists");

            /* include <direct.h> */
            _mkdir(_url.c_str());
        }

        /**
         * Make directories
         *
         * @return A const void.
         */
        void mkdirs() const {
            // Generate parent URLs from current file URL
            std::vector<std::string> parents = std::vector<std::string>(0);
            File current = *this;
            do {
                File parent = current.parent();

                if (parent._url == current._url)
                    break;

                parents.push_back(parent.url());
                current = parent;
            } while (!current._url.empty());

            parents.erase(parents.end() - 1);

            // Can not auto iterate cause order of entries has effect
            for (uint32_t i = UINT32_T(parents.size() - 1); i > 0; i --) {
                std::string& entry = parents[i];
                File node = File(entry);

                if (!node.isDirectory())
                    throw std::exception("Part of path is not a directory.");

                if (node.exists())
                    continue;
                else
                    node.mkdir();
            }
        }

        /**
         * Creates the file
         *
         * @return The new file.
         */
        void createFile() const {
            if (exists() || isDirectory())
                throw std::exception("File already exists or is a directory.");

            /* at the moment std::ofstream will create our file for us */
            std::ofstream stream;
            stream.open(_url);
            stream.close();
        }

        /**
         * Moves the given destination
         *
         * @exception std::exception Thrown when an exception error condition occurs.
         *
         * @param dst_ Destination for the.
         *
         * @return A const void.
         */
        void move(const File& dst_) {
            if (!dst_.exists() || !dst_.isDirectory())
                throw std::exception("Target is not a existing directory.");

            // TODO: move file to destination
        }

        /**
         * Cast that converts the given  to a string
         *
         * @return The result of the operation.
         */
        operator std::string() const {
            return _url;
        }

        const static char seperator = '\\';

    protected:
        std::string _url;
    };
}
