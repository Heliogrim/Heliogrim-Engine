LENS: security

Scope: trust boundaries of an engine and editor that load assets, archives, shaders, and user files from disk, and will later take network input. Only code on the range; an existing hole next to the diff goes under NOT REVIEWED with `pre-existing`.

Checklist (cite the id):
- X1 Untrusted length or index: a size, count, offset, or index read from an archive, asset, file, or packet used for allocation, indexing, `memcpy`, or pointer arithmetic without a bound check against the actual buffer.
- X2 Integer overflow on the way to X1: multiplication or addition of untrusted sizes without overflow-checked arithmetic.
- X3 Hydrate and deserialize: `access::Structure<X>::hydrate` or `access::Layout<X>::load` that trusts a version, enum, or tag value without rejecting unknown ones; a variant or enum switch over loaded data with no default rejection.
- X4 Path handling: a path built from asset or user data that is not confined to the intended root (no normalization, `..` not rejected, absolute path accepted).
- X5 Lifetime at the boundary: a `ptr<T>` or view into a loaded buffer kept past the buffer's lifetime; a `std::span` or string view returned from a temporary archive.
- X6 Shader and accel sources: shader text, IR, or SPIR-V taken from an asset and passed to the compiler or driver without validation step the pipeline already offers (e.g. spirv-val), or with a size the driver trusts.
- X7 Unchecked externals: a vendored API return value (Vulkan, assimp, spng, ryml, ...) ignored where failure leaves a partially initialized object in use.
- X8 Secrets and logging: a file path, user identity, or token written to the log at a level that ships.

Severity: X1 to X4 blocker; X5 to X7 major; X8 minor.
