#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "Entity.hpp"

namespace ember {

    namespace {

        /**
         * A typed entity base.
         *
         * @author Julius
         * @date 21.08.2021
         */
        class TypedEntityBase {
        public:
            /**
             * Default constructor
             *
             * @author Julius
             * @date 21.08.2021
             */
            TypedEntityBase() = delete;

            /**
             * Destructor
             *
             * @author Julius
             * @date 21.08.2021
             */
            ~TypedEntityBase() noexcept = default;

        public:
            /**
             * Cast this typed entity to a general entity
             *
             *  Attention: This is a lossy one way cast.
             *
             * @author Julius
             * @date 21.08.2021
             */
            [[nodiscard]] operator ember::Entity() const noexcept;

            /**
             * Cast this typed entity to a general entity
             *
             *  Attention: This is a lossy one way cast.
             *
             * @author Julius
             * @date 21.08.2021
             */
            [[nodiscard]] operator ember::Entity() noexcept;
        };
    }

    template <typename Ty>
    concept HasTypedEntityMethods = requires(Ty obj) {
        { obj.construct() } -> std::same_as<void>;
        { obj.assemble() } -> std::same_as<void>;
        { obj.disassemble() } -> std::same_as<void>;
        { obj.destruct() } -> std::same_as<void>;
    };

    template <typename Ty>
    concept IsTypedEntity = ember::HasStaticType<Ty> && HasTypedEntityMethods<Ty>;

    template <class DerivedTypedEntity>
    class TypedEntity :
        public TypedEntityBase {
    public:
        using base_type = TypedEntityBase;

        using value_type = TypedEntity<DerivedTypedEntity>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using derived_type = DerivedTypedEntity;

        //static_assert(_STD is_base_of_v<base_type, DerivedTypedEntity>, "Derived Entity have to inherit from TypedEntity."); 
        static_assert(IsTypedEntity<DerivedTypedEntity>,
            "Derived Entity should comply with the constraints.");
    };

}
