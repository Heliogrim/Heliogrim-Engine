#pragma once
#include "Source.hpp"
#include "../File.hpp"

namespace ember::engine::resource {
    class FileSource :
        public Source {
    public:
        using base_type = FileSource;

        using value_type = FileSource;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        FileSource() noexcept = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  other_ The other.
         */
        FileSource(mref<FileSource> other_) noexcept = default;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  other_ The other.
         */
        FileSource(cref<FileSource> other_) noexcept = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  file_ The file.
         * @param  size_ (Optional) The size.
         * @param  offset_ (Optional) The offset.
         */
        FileSource(mref<fs::File> file_, streamsize size_ = 0i64, streamoff offset_ = 0i64) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        ~FileSource() noexcept override = default;

    public:
        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type> other_) noexcept;

        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(cref<value_type> other_) noexcept = default;

    private:
        /**
         * The underlying file to access
         */
        fs::File _file;

        /**
         * The length of the source data to read at maximum
         *
         *  _size <= 0  :: read and write to whole file
         *  _size > 0   :: read and write only until size_ reached ( could cut write sequence, will not write over extent )
         */
        streamsize _size;

        /**
         * The offset from the beginning of the file to read data
         */
        streamoff _offset;

    public:
        [[nodiscard]] bool isAsync() const noexcept override;

        [[nodiscard]] bool isReady() const noexcept override;

        [[nodiscard]] bool isReadable() const noexcept override;

        [[nodiscard]] bool isWritable() const noexcept override;

    public:
        [[nodiscard]] bool get(
            streamoff offset_,
            streamsize size_,
            ptr<void> dst_,
            ref<streamsize> actualSize_
        ) override;

        [[nodiscard]] ember::concurrent::future<async_result_value> get(
            streamoff offset_,
            streamsize size_
        ) override;

    public:
        [[nodiscard]] bool write(
            streamoff offset_,
            streamsize size_,
            const ptr<void> src_,
            ref<streamsize> actualSize_
        ) override;

        [[nodiscard]] ember::concurrent::future<async_write_result> write(
            streamoff offset_,
            streamsize size_,
            const ptr<void> src_
        ) override;
    };
}
