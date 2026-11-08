#pragma once
#include "../Emitter.hpp"
#include "../Ir/Diagnostic.hpp"
#include "../Ir/Layout.hpp"
#include "../Ir/Module.hpp"
#include "GlslRequirements.hpp"

namespace hg::engine::accel::lang {
    struct GlslEmitResult {
        ir::Diagnostics diagnostics;
        /* what the emitted text needs from the device; empty on error */
        GlslRequirements requirements;
    };

    class GlslEmitter final :
        public Emitter {
    public:
        [[nodiscard]] bool supportsDialect(const Dialect dialect_) const noexcept override {
            return dialect_ == Dialect::eVulkanGlsl460;
        }

        /* a representation carries no code: emits the empty module (version line and an empty `main`) */
        void operator()(cref<Representation> rep_, Dialect dialect_, ref<Language> target_) const override;

        /**
         * Appends one text block of Vulkan GLSL 460 for a resolved module and its layout. Set, binding, push and
         * input location numbers come from `layout_` only; output locations are numbered by id. Emits the bindings
         * (implicit ones emit nothing), the named structs reachable from them and from function code (once per name),
         * the functions with body and `main` (empty if the module has none). Identifiers are kept, builtins map to
         * their GLSL names. On any error `target_` is left unchanged.
         * Only the `#extension` lines the used types need are written (see `GlslRequirements`).
         */
        [[nodiscard]] GlslEmitResult emit(ref<const ir::Module> mod_, ref<const ir::Layout> layout_, ref<Language> target_) const;
    };
}
