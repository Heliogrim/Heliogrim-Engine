#pragma once
#include "../Inc.hpp"
#include "../Library/Mapping/ModelMapping.hpp"

namespace clockwork::__internal::gfx {
    class Graphics;
}

namespace clockwork::__internal::gfx::model {
    class Model;
}

namespace clockwork::__internal::gfx::handle {
    class ModelHandler {
        friend class Graphics;

    public:
        using index_type = _STD string;
        using value_type = model::Model;
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
        mapping::__ptr__ModelMapping _mapping;

        ModelHandler();

        ~ModelHandler();
    };

    TYPE_UNIQUE(ModelHandler, ModelHandler);
}
