#pragma once

namespace ember {
    class Graphics {
    public:
        using value_type = Graphics;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  internal_ The internal.
         */
        explicit Graphics(const ptr<void> internal_) :
            _internal(internal_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  other_ The other.
         */
        Graphics(const_reference_type other_) = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  other_ The other.
         */
        Graphics(mref<value_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        ~Graphics() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) noexcept = delete;

    private:
        /**
         * Internal
         */
        const ptr<void> _internal;

    public:
        /**
         * Gets the internal state
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @returns A const reference to the internal state pointer
         */
        [[nodiscard]] cref<decltype(_internal)> unwrap() const noexcept;
    };
}
