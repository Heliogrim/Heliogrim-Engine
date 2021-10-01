#pragma once
#include "Engine.Common/Types.hpp"
#include "Engine.Common/Wrapper.hpp"
#include "Engine.Common/__macro.hpp"
#include <Engine.Common/Concurrent/Future.hpp>

namespace ember::engine::res {
    class __declspec(novtable) Source {
    public:
        using value_type = Source;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using async_result_value = struct {
            u64 actualSize;
            ptr<void> buffer;
        };

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        Source() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        virtual ~Source() noexcept = default;

    public:
        /**
         * Check whether this source has async behaviour
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @returns True if asynchronous, false if not.
         */
        [[nodiscard]] virtual bool isAsync() const noexcept = 0;

        /**
         * Check whether this source is ready to operate
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @returns True if ready, false if not.
         */
        [[nodiscard]] virtual bool isReady() const noexcept = 0;

        /**
         * Check whether this source is readable
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @returns True if readable, false if not.
         */
        [[nodiscard]] virtual bool isReadable() const noexcept = 0;

        /**
         * Check whether this source is writable
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @returns True if writable, false if not.
         */
        [[nodiscard]] virtual bool isWritable() const noexcept = 0;

    public:
        /**
         * Get some data from the underlying source by offset_ and size_
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  offset_ The offset.
         * @param  size_ The size.
         * @param  dst_ Destination for the.
         * @param  actualSize_ Size of the actual.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] virtual bool get(IN u64 offset_, IN u64 size_, ptr<void> dst_, OUT ref<u64> actualSize_) = 0;

        /**
         * Get some data async from the underlying source by offset_ and size_
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  offset_ The offset.
         * @param  size_ The size.
         *
         * @returns A concurrent::future&lt;async_result_value&gt;
         *  
         *  TODO: could be defaulted with promise wrapping sync get call.
         */
        [[nodiscard]] virtual concurrent::future<async_result_value> get(IN u64 offset_, IN u64 size_) = 0;
    };
}
