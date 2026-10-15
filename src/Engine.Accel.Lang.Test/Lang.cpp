#include "pch.h"

#include <Engine.Common/Make.hpp>
#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Accel.Lang/Representation.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;

namespace AccelLang {
	namespace {
		[[nodiscard]] uptr<Variable> makeVariable(const string_view symbol_, uptr<Annotation> next_ = nullptr) {
			auto var = make_uptr<Variable>();
			var->countTag = 1u;
			var->annotation = make_uptr<SymbolIdAnnotation>(string { symbol_ }, std::move(next_));
			return var;
		}
	}

	TEST(SymbolIdTest, SameNameSameId) {
		EXPECT_EQ(SymbolId::from(StringView { "color" }), SymbolId::from(String { "color" }));
		EXPECT_FALSE(SymbolId::from(StringView { "color" }) == SymbolId::from(StringView { "colour" }));
	}

	TEST(SymbolIdTest, StdHashIsValue) {
		const auto id = SymbolId::from(StringView { "color" });
		EXPECT_EQ(std::hash<SymbolId> {}(id), id.value);
	}

	TEST(AnnotationTest, CopyClonesWholeChain) {
		auto chain = make_uptr<SimpleAnnotation<AnnotationType::eReadonly>>(
			make_uptr<VkBindLocationAnnotation>(1, 2, 3)
		);

		const uptr<Annotation> copy { chain->createCopy() };

		ASSERT_NE(copy, nullptr);
		EXPECT_NE(copy.get(), chain.get());
		EXPECT_EQ(copy->type, AnnotationType::eReadonly);
		ASSERT_NE(copy->next, nullptr);
		EXPECT_NE(copy->next.get(), chain->next.get());
		EXPECT_EQ(copy->next->type, AnnotationType::eVkBindLocation);

		const auto* const bind = static_cast<const VkBindLocationAnnotation*>(copy->next.get());
		EXPECT_EQ(bind->vkSet, 1);
		EXPECT_EQ(bind->vkLocation, 2);
		EXPECT_EQ(bind->vkOffset, 3);
	}

	TEST(VariableTest, CopyDeepCopiesAnnotation) {
		const auto original = makeVariable("a");
		const Variable copy { *original };

		EXPECT_EQ(copy.countTag, 1u);
		ASSERT_NE(copy.annotation, nullptr);
		EXPECT_NE(copy.annotation.get(), original->annotation.get());
		EXPECT_EQ(static_cast<SymbolIdAnnotation*>(copy.annotation.get())->symbolId, "a");
	}

	TEST(IntermediateTest, EnumeratesInboundAndOutboundSymbols) {
		Intermediate im {};
		im.rep.globalScope.inbound.push_back(makeVariable("in0"));
		im.rep.globalScope.inbound.push_back(makeVariable("in1"));
		im.rep.globalScope.outbound.push_back(makeVariable("out0"));

		Vector<nmpt<const Symbol>> in {}, out {};
		im.enumerateInboundSymbols(in);
		im.enumerateOutboundSymbols(out);

		ASSERT_EQ(in.size(), 2uLL);
		ASSERT_EQ(out.size(), 1uLL);
		EXPECT_EQ(in[0]->symbolId, SymbolId::from(StringView { "in0" }));
		EXPECT_EQ(in[1]->symbolId, SymbolId::from(StringView { "in1" }));
		EXPECT_EQ(out[0]->symbolId, SymbolId::from(StringView { "out0" }));
		EXPECT_EQ(im.rep.symbolTable.size(), 3uLL);
	}

	TEST(IntermediateTest, EnumerationIsIdempotent) {
		Intermediate im {};
		im.rep.globalScope.inbound.push_back(makeVariable("in0"));

		Vector<nmpt<const Symbol>> first {}, second {};
		im.enumerateInboundSymbols(first);
		im.enumerateInboundSymbols(second);

		ASSERT_EQ(first.size(), 1uLL);
		ASSERT_EQ(second.size(), 1uLL);
		EXPECT_EQ(first[0].get(), second[0].get());
		EXPECT_EQ(im.rep.symbolTable.size(), 1uLL);
	}

	TEST(IntermediateTest, EnumerationSkipsSymbolIdWrappedBehindOtherAnnotations) {
		Intermediate im {};
		im.rep.globalScope.inbound.push_back(
			makeVariable("in0", nullptr)
		);
		auto wrapped = make_uptr<Variable>();
		wrapped->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(
			make_uptr<SymbolIdAnnotation>(string { "wrapped" })
		);
		im.rep.globalScope.inbound.push_back(std::move(wrapped));

		Vector<nmpt<const Symbol>> in {};
		im.enumerateInboundSymbols(in);

		ASSERT_EQ(in.size(), 2uLL);
		EXPECT_EQ(in[1]->symbolId, SymbolId::from(StringView { "wrapped" }));
	}

	TEST(RepresentationTest, CopyRebuildsVariableSymbolsAgainstOwnScope) {
		Intermediate im {};
		im.rep.globalScope.inbound.push_back(makeVariable("in0"));
		im.rep.globalScope.outbound.push_back(makeVariable("out0"));
		Vector<nmpt<const Symbol>> tmp {};
		im.enumerateInboundSymbols(tmp);
		im.enumerateOutboundSymbols(tmp);

		const Representation copy { im.rep };

		ASSERT_EQ(copy.globalScope.inbound.size(), 1uLL);
		ASSERT_EQ(copy.globalScope.outbound.size(), 1uLL);
		EXPECT_NE(copy.globalScope.inbound[0].get(), im.rep.globalScope.inbound[0].get());
		ASSERT_EQ(copy.symbolTable.size(), 2uLL);
		for (const auto& symbol : copy.symbolTable) {
			const auto* const data = symbol->var.data.get();
			EXPECT_TRUE(data == copy.globalScope.inbound[0].get() || data == copy.globalScope.outbound[0].get());
		}
	}

	TEST(GlslEmitterTest, SupportsOnlyVulkanGlsl) {
		const GlslEmitter emitter {};

		EXPECT_TRUE(emitter.supportsDialect(Dialect::eVulkanGlsl460));
	}

	// GlslEmitter::operator() is an empty stub: no text is produced yet. Enable once emission is implemented.
	TEST(GlslEmitterTest, DISABLED_EmitsTextBlock) {
		const GlslEmitter emitter {};
		Representation rep {};
		Language target {};

		emitter(rep, Dialect::eVulkanGlsl460, target);

		EXPECT_FALSE(target.text.empty());
	}
}
