#pragma once

#include <direct.h>
#include <filesystem>
#include <fstream>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

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
            _url(string()) { }

        /**
         * Constructor
         *
         * @param url_ The file.
         */
        File(cref<string> url_) :
            _url(url_) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.09.2021
         *
         * @param  url_ URL_ of the resource.
         */
        File(cref<string_view> url_) :
            _url(_STD string(url_)) {}

        /**
         * Gets the file
         *
         * @return The file.
         */
        cref<string> url() const {
            return _url;
        }

        /**
         * Check whether file or directory exists
         *
         * @return A const bool.
         */
        bool exists() const {
            return _STD filesystem::exists(_url);
        }

        /**
         * Check whether URL is a directory or a file
         *
         * @return A const bool.
         */
        bool isDirectory() const {
            return _STD filesystem::is_directory(_url);
        }

        /**
         * Gets the size
         *
         * @return A const size_t.
         */
        size_t size() const {
            return _STD filesystem::file_size(_url);
        }

        /**
         * Gets the parent
         *
         * @return A const File.
         */
        File parent() const {
            const size_t posx = _url.find_last_of(seperator);
            const string parentUrl = _url.substr(0, posx);
            return File(parentUrl);
        }

        /**
         * Gets all subfiles from this directory
         *
         * @exception std::exception Thrown when file is not a directory or can not fetch the subfiles.
         *
         * @return A const std::vector&lt;File&gt;
         */
        [[nodiscard]] Vector<File> files() const {
            // Could return without effect
            if (!isDirectory()) {
                throw std::exception("URL is not a directory. Can not fetch files.");
            }

            Vector<File> files = Vector<File>(0);
            const auto iter = _STD filesystem::directory_iterator { _url };

            for (const auto& entry : iter) {
                if (entry.is_regular_file()) {
                    files.push_back(File { entry.path().string() });
                }
            }

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
         */
        void mkdir() const {
            if (exists())
                throw std::exception("Directory already exists");

            /* include <direct.h> */
            _mkdir(_url.c_str());
        }

        /**
         * Make directories
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
        [[nodiscard]] operator string() const noexcept {
            return _url;
        }

        const static char seperator = '\\';

    protected:
        std::string _url;

    public:
        [[nodiscard]] bool operator==(cref<File> other_) const noexcept {
            return _url == other_._url;
        }

        [[nodiscard]] bool operator!=(cref<File> other_) const noexcept {
            return _url != other_._url;
        }
    };
}

namespace std {

    template <>
    struct hash<ember::File> {
        [[nodiscard]] size_t operator()(const ember::File& value_) const noexcept {
            return _STD hash<string> {}(value_.url());
        }
    };

    template <>
    struct less<ember::File> {
        [[nodiscard]] bool operator()(const ember::File& left_, const ember::File& right_) const noexcept {
            return _STD less<string> {}(left_.url(), right_.url());
        }
    };
}
