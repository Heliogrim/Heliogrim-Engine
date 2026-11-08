#pragma once

#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/* name identity: scope slot of an immutable local, `globalIdentity` for constants/builtins, otherwise unknown (never equal) */
	inline constexpr size_t globalIdentity = ~0uz;
	inline constexpr size_t unknownIdentity = ~1uz;

	struct PlaceStep {
		enum class Kind : u8 {
			eField,
			eIndex,
			eRange
		} kind = Kind::eField;

		String field {};
		ExprRef index {};
		ExprRef lo {};
		ExprRef hi {};
		/* identity of every name in index/lo/hi (DFS order) as resolved when the path was built, see below */
		Vector<size_t> indexIds {};
		Vector<size_t> loIds {};
		Vector<size_t> hiIds {};
	};

	struct PlacePath {
		String root {};
		/* scope index of the root local; ~0 for bindings, constants and builtins */
		size_t rootScope = ~0uz;
		Vector<PlaceStep> steps {};
	};

	/**
	 * Whether two places provably share no storage. Distinct roots, different struct fields, different constant
	 * indices, non-intersecting constant ranges, and ranges where one's `hi` is structurally the other's `lo`
	 * (built only from names `immutable_` accepts, literals and operators) are disjoint; anything unprovable,
	 * and a prefix of the other, overlaps.
	 */
	[[nodiscard]] bool disjoint(
		ref<const Module> mod_,
		ref<const PlacePath> a_,
		ref<const PlacePath> b_,
		FnRef<bool(ref<const Expr>)> immutable_
	);

	/* structural equality of two expressions (kind, name, operands); locations and types are ignored */
	[[nodiscard]] bool sameExpr(ref<const Module> mod_, ExprRef a_, ExprRef b_);

	/* value of a GLSL integer literal (`8`, `8u`, `0x8`, `010`); none for anything else */
	[[nodiscard]] Opt<long long> integerLiteral(StringView text_);

	/* source-like rendering, e.g. `buf.data[0 .. n]` */
	[[nodiscard]] String describePlace(ref<const Module> mod_, ref<const PlacePath> path_);
}
