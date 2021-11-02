#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::proxy {

    /**
     * Forward Declaration
     */
    class OwningProxiedScenePayload;

    class ProxiedScenePayload {
    public:
        using value_type = ProxiedScenePayload;

        using payload_type = wptr<void>;

    public:
        constexpr ProxiedScenePayload() noexcept = default;

        ProxiedScenePayload(_Inout_ mref<payload_type> payload_) noexcept;

        ProxiedScenePayload(_In_ cref<OwningProxiedScenePayload> parent_) noexcept;

        ProxiedScenePayload(_In_ cref<ProxiedScenePayload> other_) = default;

        ProxiedScenePayload(_Inout_ mref<ProxiedScenePayload> other_) noexcept;

        ~ProxiedScenePayload() = default;

    public:
        ref<ProxiedScenePayload> operator=(cref<ProxiedScenePayload> other_) = default;

        ref<ProxiedScenePayload> operator=(mref<ProxiedScenePayload> other_) noexcept;

        ref<ProxiedScenePayload> operator=(cref<OwningProxiedScenePayload> parent_) noexcept;

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

    class OwningProxiedScenePayload {
    public:
        using value_type = ProxiedScenePayload;

        using payload_type = sptr<void>;

    public:
        constexpr OwningProxiedScenePayload() noexcept = default;

        OwningProxiedScenePayload(_Inout_ mref<payload_type> payload_) noexcept;

        template <typename PayloadType_>
        OwningProxiedScenePayload(_Inout_ mref<sptr<PayloadType_>> payload_) noexcept :
            _payload(_STD move(payload_)) {}

        OwningProxiedScenePayload(_In_ cref<OwningProxiedScenePayload> other_) = delete;

        OwningProxiedScenePayload(_Inout_ mref<OwningProxiedScenePayload> other_) noexcept;

        ~OwningProxiedScenePayload() = default;

    public:
        ref<OwningProxiedScenePayload> operator=(cref<OwningProxiedScenePayload>) = delete;

        ref<OwningProxiedScenePayload> operator=(mref<OwningProxiedScenePayload> other_) noexcept;

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
         * @returns A a pointer to the owned payload.
         */
        template <typename PayloadType_>
        [[nodiscard]] ptr<PayloadType_> payload() const noexcept {
            return _payload.get();
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
         * @returns A pointer to the owned payload.
         */
        template <typename PayloadType_>
        [[nodiscard]] ptr<PayloadType_> payload() noexcept {
            return _payload.get();
        }
    };

}
