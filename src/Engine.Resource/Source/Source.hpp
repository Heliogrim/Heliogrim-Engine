#pragma once
#include "Engine.Common/Types.hpp"
#include "Engine.Common/Wrapper.hpp"
#include "Engine.Common/__macro.hpp"
#include <Engine.Common/Concurrent/Future.hpp>

namespace ember::engine::resource {
    class __declspec(novtable) Source {
    public:
        using value_type = Source;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using async_result_value = struct {
            streamsize actualSize;
            ptr<void> buffer;
        };

        using async_write_result = struct {
            streamsize actualSize;
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
        [[nodiscard]] virtual bool get(
            _In_ streamoff offset_,
            _In_ streamsize size_,
            _Out_writes_bytes_(actualSize_) ptr<void> dst_,
            _Out_ ref<streamsize> actualSize_
        ) = 0;

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
        [[nodiscard]] virtual ember::concurrent::future<async_result_value> get(
            _In_ streamoff offset_,
            _In_ streamsize size_
        ) = 0;

    public:
        [[nodiscard]] virtual bool write(
            _In_ streamoff offset_,
            _In_ streamsize size_,
            _In_reads_bytes_(size_) const ptr<void> src_,
            _Out_ ref<streamsize> actualSize_
        ) = 0;

        [[nodiscard]] virtual ember::concurrent::future<async_write_result> write(
            _In_ streamoff offset_,
            _In_ streamsize size_,
            _In_reads_bytes_(size_) const ptr<void> src_
        ) = 0;
    };
}
