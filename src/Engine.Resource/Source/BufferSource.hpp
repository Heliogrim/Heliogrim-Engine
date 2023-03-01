#pragma once
#include <Engine.Common/Buffer.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Source.hpp"

namespace hg::engine::resource {
    class BufferSource :
        public Source {
    public:
        using base_type = Source;

        using value_type = BufferSource;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        BufferSource() noexcept = delete;

        BufferSource(mref<Buffer> buffer_);

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  other_ The other.
         */
        BufferSource(mref<value_type> other_) noexcept = default;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  other_ The other.
         */
        BufferSource(cref<value_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        ~BufferSource() noexcept override = default;

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

        //private:
    public:
        Buffer _buffer;

    public:
        [[nodiscard]] bool isAsync() const noexcept override;

        [[nodiscard]] bool isReady() const noexcept override;

        [[nodiscard]] bool isReadable() const noexcept override;

        [[nodiscard]] bool isWritable() const noexcept override;

    public:
        [[nodiscard]] streamsize size() const override;

        [[nodiscard]] streamsize estimatedSize() const noexcept override;

    public:
        [[nodiscard]] bool get(
            streamoff offset_,
            streamsize size_,
            ptr<void> dst_,
            ref<streamsize> actualSize_
        ) override;

        [[nodiscard]] hg::concurrent::future<async_result_value> get(
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

        [[nodiscard]] hg::concurrent::future<async_write_result> write(
            streamoff offset_,
            streamsize size_,
            const ptr<void> src_
        ) override;
    };
}
