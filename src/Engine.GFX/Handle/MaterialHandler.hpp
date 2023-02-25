#pragma once
#include "../Inc.hpp"
#include "../Library/Mapping/MaterialMapping.hpp"

namespace clockwork::__internal::gfx {
    class Graphics;
}

namespace clockwork::__internal::gfx::material {
    class Material;
}

namespace clockwork::__internal::gfx::handle {
    class MaterialHandler {
        friend class Graphics;

    public:
        using index_type = _STD string;
        using value_type = material::Material;
        using reference_type = value_type&;
        using const_reference_type = const reference_type;

        /**
         * \brief 
         * \param index_ 
         * \return 
         */
        [[nodiscard]] const_reference_type get(const index_type& index_) const;

        /**
         * \brief 
         * \param index_ 
         * \return 
         */
        reference_type get(const index_type& index_);

        /**
         * \brief 
         * \param obj_ 
         * \return 
         */
        reference_type store(const_reference_type obj_);

        /**
         * \brief 
         * \param index_ 
         */
        void remove(const index_type& index_);

    private:
        /**
         * \brief 
         */
        mapping::__ptr__MaterialMapping _mapping;

        MaterialHandler();

        ~MaterialHandler();
    };

    TYPE_UNIQUE(MaterialHandler, MaterialHandler);
}
