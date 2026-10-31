#include "pch.h"

#include <Engine.Accel.Effect/Stage/Stage.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Accel.Storage/HashedState.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg;
using namespace hg::engine::accel;

namespace AccelCompile {
	namespace {
		[[nodiscard]] EffectProfile profile(StringView definition_) {
			EffectProfile result {};
			result._name = "Default";
			result._definitions.emplace_back(DefinitionType::eText, String { definition_ });
			return result;
		}

		[[nodiscard]] smr<lang::Intermediate> intermediate(StringView symbolId_) {
			auto im = make_smr<lang::Intermediate>();
			im->lang.text.emplace_back(lang::TextBlock { "void main() {}" });
			auto var = make_uptr<lang::Variable>();
			var->type = lang::Type { .category = lang::TypeCategory::eScalar, .scalarType = lang::ScalarType::eF32 };
			var->annotation = make_uptr<lang::SymbolIdAnnotation>(String { symbolId_ });
			im->rep.globalScope.outbound.emplace_back(std::move(var));
			return im;
		}
	}

	TEST(ProfileKeyTest, SameNameDifferentDefinitionDiffers) {
		EXPECT_NE(calcStorageHash(profile("#define A 1")), calcStorageHash(profile("#define A 2")));
		EXPECT_EQ(calcStorageHash(profile("#define A 1")), calcStorageHash(profile("#define A 1")));
	}

	TEST(ProfileKeyTest, SameNameDifferentModuleDiffers) {
		auto a = profile("");
		auto b = profile("");
		a._modules.emplace_back(*intermediate("depth"));
		b._modules.emplace_back(*intermediate("color"));
		EXPECT_NE(calcStorageHash(a), calcStorageHash(b));
	}

	TEST(StageKeyTest, EqualContentDifferentAddressSameKey) {
		Stage a { StageFlagBits::eFragment };
		Stage b { StageFlagBits::eFragment };
		Stage c { StageFlagBits::eFragment };
		a.setIntermediate(intermediate("depth"));
		b.setIntermediate(intermediate("depth"));
		c.setIntermediate(intermediate("color"));

		EXPECT_EQ(calcStorageHash(a), calcStorageHash(b));
		EXPECT_NE(calcStorageHash(a), calcStorageHash(c));

		Stage vertex { StageFlagBits::eVertex };
		vertex.setIntermediate(intermediate("depth"));
		EXPECT_NE(calcStorageHash(a), calcStorageHash(vertex));
	}
}
