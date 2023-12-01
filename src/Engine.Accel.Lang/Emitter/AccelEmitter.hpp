#pragma once
#include "../Emitter.hpp"

namespace hg::engine::accel::lang {
    class AccelEmitter final :
        public Emitter {
    public:
        [[nodiscard]] bool supportsDialect(const Dialect dialect_) const noexcept override {
            return dialect_ == Dialect::eAccelGlsl;
        }

        void operator()(cref<Representation> rep_, Dialect dialect_, ref<Language> target_) const override;
    };
}
