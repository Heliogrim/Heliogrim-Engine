#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "Source.hpp"

namespace ember::engine::resource {
    class ParallelSource :
        public Source {
    public:
        using base_type = Source;

        using value_type = ParallelSource;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 10.09.2021
         */
        ParallelSource() noexcept = delete;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  other_ The other.
         */
        ParallelSource(const_reference_type other_) = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  other_ The other.
         */
        ParallelSource(mref<value_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 10.09.2021
         */
        ~ParallelSource() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type other_) noexcept = default;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type> other_) noexcept;

    private:
        Vector<ptr<Source>> _sources;

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

        [[nodiscard]] ember::concurrent::future<async_result_value> get(streamoff offset_, streamsize size_) override;

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
