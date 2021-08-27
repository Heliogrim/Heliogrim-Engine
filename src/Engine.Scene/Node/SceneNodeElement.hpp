#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scene {

    class SceneNodeElementBase {
    public:
        using value_type = SceneNodeElementBase;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using payload_type = wptr<void>;

    public:
        ~SceneNodeElementBase() = default;

    protected:
        /**
         * The payload
         */
        payload_type _payload;

    public:
        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A cref&lt;payload_type&gt;
         */
        [[nodiscard]] cref<payload_type> payload() const noexcept;

        /**
         * Gets the payload
         *
         * @tparam PayloadType_ Type of the payload type.
         *
         * @returns A cref&lt;wptr&lt;_STD remove_reference_t&lt;PayloadType_&gt;&gt;&gt;
         */
        template <typename PayloadType_>
        [[nodiscard]] cref<wptr<_STD remove_reference_t<PayloadType_>>> payload() const noexcept {
            return *static_cast<ptr<const wptr<_STD remove_reference_t<PayloadType_>>>>(
                static_cast<ptr<const void>>(&_payload)
            );
        }

        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A ref&lt;payload_type&gt;
         */
        [[nodiscard]] ref<payload_type> payload() noexcept;

        /**
         * Gets the payload
         *
         * @tparam PayloadType_ Type of the payload type.
         *
         * @returns A ref&lt;wptr&lt;_STD remove_reference_t&lt;PayloadType_&gt;&gt;&gt;
         */
        template <typename PayloadType_>
        [[nodiscard]] ref<wptr<_STD remove_reference_t<PayloadType_>>> payload() noexcept {
            return *static_cast<ptr<wptr<_STD remove_reference_t<PayloadType_>>>>(
                static_cast<ptr<void>>(&_payload)
            );
        }
    };

    template <typename PayloadType_>
    class SceneNodeElement :
        public SceneNodeElementBase {
    public:
        using base_type = SceneNodeElementBase;

        using value_type = SceneNodeElement<PayloadType_>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using payload_type = wptr<_STD remove_reference_t<PayloadType_>>;

    public:
        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A cref&lt;payload_type&gt;
         */
        [[nodiscard]] cref<payload_type> payload() const noexcept {
            return *static_cast<ptr<const payload_type>>(
                static_cast<ptr<const void>>(&_payload)
            );
        }

        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A ref&lt;payload_type&gt;
         */
        [[nodiscard]] ref<payload_type> payload() noexcept {
            return *static_cast<ptr<payload_type>>(
                static_cast<ptr<void>>(&_payload)
            );
        }
    };
}
