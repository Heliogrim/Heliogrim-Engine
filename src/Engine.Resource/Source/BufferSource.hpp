#pragma once
#include <Engine.Common/Buffer.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Source.hpp"

namespace ember::engine::res {

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

    private:
        Buffer _buffer;

    public:
        [[nodiscard]] bool isAsync() const noexcept override;

        [[nodiscard]] bool isReady() const noexcept override;

        [[nodiscard]] bool isReadable() const noexcept override;

        [[nodiscard]] bool isWritable() const noexcept override;

    public:
        [[nodiscard]] bool get(u64 offset_, u64 size_, ptr<void> dst_, ref<u64> actualSize_) override;

        [[nodiscard]] concurrent::future<async_result_value> get(u64 offset_, u64 size_) override;
    };
}
