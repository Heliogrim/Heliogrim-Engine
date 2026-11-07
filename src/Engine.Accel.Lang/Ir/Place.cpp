#include "Place.hpp"

#include <algorithm>
#include <charconv>

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr auto depthLimit = 64uz;

	[[nodiscard]] const ir::Expr* at(ref<const ir::Module> module_, ir::ExprRef ref_) {
		return ref_.index < module_.exprs.size() ? &module_.exprs[ref_.index] : nullptr;
	}

	[[nodiscard]] Opt<long long> constantOf(ref<const ir::Module> module_, ir::ExprRef ref_) {
		const auto* e = at(module_, ref_);
		return e != nullptr && e->kind == ir::ExprKind::eLiteral ? ir::integerLiteral(e->name) : None;
	}

	/* an expression only built from literals, operators and names the typechecker vouches for */
	[[nodiscard]] bool stable(
		ref<const ir::Module> module_,
		ir::ExprRef ref_,
		const std::function<bool(ref<const ir::Expr>)>& immutable_,
		size_t depth_ = 0uz
	) {
		const auto* e = at(module_, ref_);
		if (e == nullptr || depth_ > depthLimit) {
			return false;
		}
		switch (e->kind) {
			case ir::ExprKind::eLiteral: return true;
			case ir::ExprKind::eNameRef:
			case ir::ExprKind::eBuiltin:
				return e->operands.empty() && immutable_(*e);
			case ir::ExprKind::eField:
			case ir::ExprKind::eIndex:
			case ir::ExprKind::eUnary:
			case ir::ExprKind::eBinary:
			case ir::ExprKind::eTernary:
				return std::ranges::all_of(
					e->operands,
					[&](ir::ExprRef o_) { return stable(module_, o_, immutable_, depth_ + 1uz); }
				);
			default: return false;
		}
	}

	/* same spelling and the same resolved identities, none of them unprovable */
	[[nodiscard]] bool sameBound(
		ref<const ir::Module> module_,
		ir::ExprRef a_,
		ref<const Vector<size_t>> aIds_,
		ir::ExprRef b_,
		ref<const Vector<size_t>> bIds_
	) {
		return aIds_ == bIds_ && std::ranges::none_of(aIds_, [](size_t i_) { return i_ == ir::unknownIdentity; }) && ir::sameExpr(module_, a_, b_);
	}

	/* `a_.hi` is what `b_.lo` starts at, and that value cannot change between the two borrows */
	[[nodiscard]] bool abuts(
		ref<const ir::Module> module_,
		ir::ExprRef hi_,
		ref<const Vector<size_t>> hiIds_,
		ir::ExprRef lo_,
		ref<const Vector<size_t>> loIds_,
		const std::function<bool(ref<const ir::Expr>)>& immutable_
	) {
		return sameBound(module_, hi_, hiIds_, lo_, loIds_) && stable(module_, hi_, immutable_);
	}

	[[nodiscard]] bool stepsDisjoint(
		ref<const ir::Module> module_,
		ref<const ir::PlaceStep> a_,
		ref<const ir::PlaceStep> b_,
		const std::function<bool(ref<const ir::Expr>)>& immutable_
	) {
		using Kind = ir::PlaceStep::Kind;
		if (a_.kind == Kind::eField && b_.kind == Kind::eField) {
			if (a_.field == b_.field) {
				return false;
			}
			/* vector swizzles overlap when they share a component; any other name pair is two struct fields */
			const auto components = [](ref<const String> n_) -> Opt<unsigned> {
				static constexpr std::string_view sets[] { "xyzw", "rgba", "stpq" };
				unsigned mask = 0u;
				for (const auto c : n_) {
					for (const auto set : sets) {
						if (const auto p = set.find(c); p != std::string_view::npos) {
							mask |= 1u << p;
						}
					}
				}
				const auto ok = not n_.empty() && std::ranges::any_of(sets, [&](std::string_view s_) {
					return std::ranges::all_of(n_, [&](char c) { return s_.contains(c); });
				});
				return ok ? Opt<unsigned> { mask } : None;
			};
			const auto x = components(a_.field);
			const auto y = components(b_.field);
			return not (x && y) || (*x & *y) == 0u;
		}
		if (a_.kind == Kind::eIndex && b_.kind == Kind::eIndex) {
			const auto x = constantOf(module_, a_.index);
			const auto y = constantOf(module_, b_.index);
			return x.has_value() && y.has_value() && *x != *y;
		}
		if (a_.kind == Kind::eRange && b_.kind == Kind::eRange) {
			const auto alo = constantOf(module_, a_.lo);
			const auto ahi = constantOf(module_, a_.hi);
			const auto blo = constantOf(module_, b_.lo);
			const auto bhi = constantOf(module_, b_.hi);
			if (alo && ahi && blo && bhi && (*ahi <= *blo || *bhi <= *alo)) {
				return true;
			}
			return abuts(module_, a_.hi, a_.hiIds, b_.lo, b_.loIds, immutable_) || abuts(module_, b_.hi, b_.hiIds, a_.lo, a_.loIds, immutable_);
		}
		if (a_.kind == Kind::eRange && b_.kind == Kind::eIndex) {
			return stepsDisjoint(module_, b_, a_, immutable_);
		}
		if (a_.kind == Kind::eIndex && b_.kind == Kind::eRange) {
			const auto x = constantOf(module_, a_.index);
			const auto lo = constantOf(module_, b_.lo);
			const auto hi = constantOf(module_, b_.hi);
			return x && lo && hi && (*x < *lo || *x >= *hi);
		}
		return false;
	}

	[[nodiscard]] bool sameStep(ref<const ir::Module> module_, ref<const ir::PlaceStep> a_, ref<const ir::PlaceStep> b_) {
		if (a_.kind != b_.kind) {
			return false;
		}
		switch (a_.kind) {
			case ir::PlaceStep::Kind::eField: return a_.field == b_.field;
			case ir::PlaceStep::Kind::eIndex: return sameBound(module_, a_.index, a_.indexIds, b_.index, b_.indexIds);
			default: return sameBound(module_, a_.lo, a_.loIds, b_.lo, b_.loIds) && sameBound(module_, a_.hi, a_.hiIds, b_.hi, b_.hiIds);
		}
	}

	[[nodiscard]] String render(ref<const ir::Module> module_, ir::ExprRef ref_, size_t depth_ = 0uz) {
		const auto* e = at(module_, ref_);
		if (e == nullptr || depth_ > depthLimit) {
			return "?";
		}
		const auto sub = [&](size_t i_) { return i_ < e->operands.size() ? render(module_, e->operands[i_], depth_ + 1uz) : String("?"); };
		switch (e->kind) {
			case ir::ExprKind::eLiteral:
			case ir::ExprKind::eNameRef:
			case ir::ExprKind::eBuiltin:
				if (e->operands.empty()) {
					return e->name;
				}
				[[fallthrough]];
			case ir::ExprKind::eCall: {
				String out = e->name + "(";
				for (auto i = 0uz; i < e->operands.size(); ++i) {
					out += (i == 0uz ? "" : ", ") + sub(i);
				}
				return out + ")";
			}
			case ir::ExprKind::eField: return sub(0uz) + "." + e->name;
			case ir::ExprKind::eIndex: return sub(0uz) + "[" + sub(1uz) + "]";
			case ir::ExprKind::eRange: return sub(0uz) + "[" + sub(1uz) + " .. " + sub(2uz) + "]";
			case ir::ExprKind::eUnary: return e->name + sub(0uz);
			case ir::ExprKind::eBinary: return "(" + sub(0uz) + " " + e->name + " " + sub(1uz) + ")";
			default: return "?";
		}
	}
}

Opt<long long> ir::integerLiteral(StringView text_) {
	if (not text_.empty() && (text_.back() == 'u' || text_.back() == 'U')) {
		text_.remove_suffix(1uz);
	}
	auto base = 10;
	if (text_.size() > 2uz && text_[0] == '0' && (text_[1] == 'x' || text_[1] == 'X')) {
		text_.remove_prefix(2uz);
		base = 16;
	} else if (text_.size() > 1uz && text_[0] == '0') {
		text_.remove_prefix(1uz);
		base = 8;
	}
	long long value = 0;
	const auto res = std::from_chars(text_.data(), text_.data() + text_.size(), value, base);
	if (text_.empty() || res.ec != std::errc {} || res.ptr != text_.data() + text_.size()) {
		return None;
	}
	return value;
}

bool ir::sameExpr(ref<const Module> module_, ExprRef a_, ExprRef b_) {
	static constexpr auto walk = [](auto&& self, ref<const Module> m_, ExprRef a, ExprRef b, size_t depth_) -> bool {
		const auto* x = at(m_, a);
		const auto* y = at(m_, b);
		if (x == nullptr || y == nullptr || depth_ > depthLimit) {
			return false;
		}
		if (x->kind != y->kind || x->name != y->name || x->operands.size() != y->operands.size()) {
			return false;
		}
		for (auto i = 0uz; i < x->operands.size(); ++i) {
			if (not self(self, m_, x->operands[i], y->operands[i], depth_ + 1uz)) {
				return false;
			}
		}
		return true;
	};
	return walk(walk, module_, a_, b_, 0uz);
}

bool ir::disjoint(
	ref<const Module> module_,
	ref<const PlacePath> a_,
	ref<const PlacePath> b_,
	const std::function<bool(ref<const Expr>)>& immutable_
) {
	if (a_.root != b_.root || a_.rootScope != b_.rootScope) {
		return true;
	}
	const auto common = std::min(a_.steps.size(), b_.steps.size());
	for (auto i = 0uz; i < common; ++i) {
		if (sameStep(module_, a_.steps[i], b_.steps[i])) {
			continue;
		}
		return stepsDisjoint(module_, a_.steps[i], b_.steps[i], immutable_);
	}
	return false;
}

String ir::describePlace(ref<const Module> module_, ref<const PlacePath> path_) {
	String out = path_.root;
	for (const auto& step : path_.steps) {
		switch (step.kind) {
			case PlaceStep::Kind::eField: out += "." + step.field; break;
			case PlaceStep::Kind::eIndex: out += "[" + render(module_, step.index) + "]"; break;
			default: out += "[" + render(module_, step.lo) + " .. " + render(module_, step.hi) + "]"; break;
		}
	}
	return out;
}
