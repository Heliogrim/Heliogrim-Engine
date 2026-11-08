#include "pch.h"

#include <array>
#include <map>

#include <Engine.Accel.Lang/Ir/Lexer.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Vector<Token> lexOk(StringView source_) {
			auto res = lex(source_);
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? std::move(res).value() : Vector<Token> {};
		}

		[[nodiscard]] Diagnostics lexFail(StringView source_) {
			auto res = lex(source_);
			EXPECT_FALSE(res.has_value());
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}
	}

	TEST(LexerTest, TracksLineAndColumn) {
		const auto tokens = lexOk("a\n\n\tbc");
		ASSERT_EQ(tokens.size(), 3uz);// a, bc, eof
		EXPECT_EQ(tokens[0].loc.line, 1u);
		EXPECT_EQ(tokens[0].loc.column, 1u);
		EXPECT_EQ(tokens[1].text, "bc");
		EXPECT_EQ(tokens[1].loc.line, 3u);
		EXPECT_EQ(tokens[1].loc.column, 2u);// tab is one column
		EXPECT_EQ(tokens[2].kind, TokenKind::eEof);
	}

	TEST(LexerTest, DotDotAfterInteger) {
		for (const auto* src : { "0u..8u", "1..2" }) {
			const auto tokens = lexOk(src);
			ASSERT_EQ(tokens.size(), 4uz) << src;
			EXPECT_EQ(tokens[0].kind, TokenKind::eInteger);
			EXPECT_EQ(tokens[1].kind, TokenKind::eDotDot);
			EXPECT_EQ(tokens[2].kind, TokenKind::eInteger);
		}
	}

	TEST(ParserTest, ParsesConstArray) {
		auto res = parseModule(
			"M",
			"const K: f32vec3[2] = f32vec3[2](f32vec3(1.0), f32vec3(2.0));\n"
			"const L: f32vec3[1] =f32vec3[](f32vec3(1.0));\n"
			"fn main() {}\n",
			EntryKind::eFragment,
			ModuleResolver { [](StringView) -> Opt<String> { return None; } }
		);
		ASSERT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
		ASSERT_EQ(res->constants.size(), 2uz);
		EXPECT_EQ(res->constants[0].name, "K");
		EXPECT_FALSE(res->constants[0].specId.has_value());
		EXPECT_EQ(res->constants[1].name, "L");
	}

	TEST(ParserTest, PackingOnOpaqueBindingIsDiagnostic) {
		const auto res = parseModule(
			"M",
			"@input (mode = bind, id = \"smp\", class = sampler, packing = std140) sampler smp;\nfn main() {}\n",
			EntryKind::eFragment,
			ModuleResolver { [](StringView) -> Opt<String> { return None; } }
		);
		ASSERT_FALSE(res.has_value());
		EXPECT_NE(res.error().front().message.find("packing"), String::npos);
		EXPECT_EQ(res.error().front().line, 1u);
	}

	TEST(ParserTest, UnknownAccessValueIsLocatedDiagnostic) {
		const auto res = parseModule(
			"M",
			"@input (mode = bind, id = \"buf\", class = storage, access = rw) block buf { u32[] data; };\nfn main() {}\n",
			EntryKind::eFragment,
			ModuleResolver { [](StringView) -> Opt<String> { return None; } }
		);
		ASSERT_FALSE(res.has_value());
		EXPECT_NE(res.error().front().message.find("access"), String::npos);
		EXPECT_EQ(res.error().front().line, 1u);
	}

	TEST(ParserTest, ParsesSpecConstant) {
		auto res = parseModule(
			"M",
			"@spec (id = \"sample-count\") const SAMPLE_COUNT: u32 = 1024u;\nfn main() {}\n",
			EntryKind::eFragment,
			ModuleResolver { [](StringView) -> Opt<String> { return None; } }
		);
		ASSERT_TRUE(res.has_value());
		ASSERT_EQ(res->constants.size(), 1uz);
		EXPECT_EQ(res->constants[0].name, "SAMPLE_COUNT");
		ASSERT_TRUE(res->constants[0].specId.has_value());
		EXPECT_EQ(*res->constants[0].specId, "sample-count");
	}

	TEST(LexerTest, SkipsComments) {
		const auto tokens = lexOk("a // line\n/* block\n x */ b");
		ASSERT_EQ(tokens.size(), 3uz);
		EXPECT_EQ(tokens[0].text, "a");
		EXPECT_EQ(tokens[1].text, "b");
		EXPECT_EQ(tokens[1].loc.line, 3u);
		EXPECT_EQ(tokens[1].loc.column, 7u);
	}

	TEST(LexerTest, UnterminatedBlockCommentIsDiagnostic) {
		const auto diags = lexFail("a\n  /* never closed");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].severity, Severity::eError);
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 3u);
	}

	TEST(LexerTest, UnterminatedStringIsDiagnostic) {
		const auto diags = lexFail("x = \"abc\ny");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_EQ(diags[0].column, 5u);
	}

	TEST(LexerTest, KeywordsAreDistinctFromIdentifiers) {
		const auto tokens = lexOk("fn struct block toggle when return if else let implicit fnx Let");
		const std::array<TokenKind, 13> expected {
			TokenKind::eKwFn, TokenKind::eKwStruct, TokenKind::eKwBlock, TokenKind::eKwToggle, TokenKind::eKwWhen,
			TokenKind::eKwReturn, TokenKind::eKwIf, TokenKind::eKwElse, TokenKind::eKwLet, TokenKind::eKwImplicit,
			TokenKind::eIdentifier, TokenKind::eIdentifier, TokenKind::eEof
		};
		ASSERT_EQ(tokens.size(), expected.size());
		for (auto i = 0uz; i < tokens.size(); ++i) {
			EXPECT_EQ(tokens[i].kind, expected[i]) << i;
		}
	}

	TEST(LexerTest, MultiCharPunctuationIsOneToken) {
		const auto tokens = lexOk("-> && || == != <= >= - & ! @when");
		const std::array<TokenKind, 13> expected {
			TokenKind::eArrow, TokenKind::eAndAnd, TokenKind::eOrOr, TokenKind::eEqEq, TokenKind::eNotEq,
			TokenKind::eLessEq, TokenKind::eGreaterEq, TokenKind::eMinus, TokenKind::eAmp, TokenKind::eBang,
			TokenKind::eAt, TokenKind::eKwWhen, TokenKind::eEof
		};
		ASSERT_EQ(tokens.size(), expected.size());
		for (auto i = 0uz; i < tokens.size(); ++i) {
			EXPECT_EQ(tokens[i].kind, expected[i]) << i;
		}
	}

	TEST(LexerTest, NumbersAndStrings) {
		const auto tokens = lexOk("12 3.5 1e3 \"a b\"");
		ASSERT_EQ(tokens.size(), 5uz);
		EXPECT_EQ(tokens[0].kind, TokenKind::eInteger);
		EXPECT_EQ(tokens[1].kind, TokenKind::eFloat);
		EXPECT_EQ(tokens[1].text, "3.5");
		EXPECT_EQ(tokens[2].kind, TokenKind::eFloat);
		EXPECT_EQ(tokens[3].kind, TokenKind::eString);
		EXPECT_EQ(tokens[3].text, "a b");// without quotes
	}

	TEST(LexerTest, MalformedNumberIsDiagnostic) {
		const auto diags = lexFail("1.2.3 12ab 1e");
		ASSERT_EQ(diags.size(), 3uz);
		EXPECT_EQ(diags[0].column, 1u);
		EXPECT_EQ(diags[1].column, 7u);
		EXPECT_EQ(diags[2].column, 12u);
	}

	TEST(LexerTest, IncludeDirectiveIsDirectiveThenTarget) {
		const auto tokens = lexOk("#include <Common/View>\nfn");
		ASSERT_EQ(tokens.size(), 4uz);
		EXPECT_EQ(tokens[0].kind, TokenKind::eInclude);
		EXPECT_EQ(tokens[1].kind, TokenKind::eIncludeTarget);
		EXPECT_EQ(tokens[1].text, "Common/View");
		EXPECT_EQ(tokens[1].loc.column, 11u);
		EXPECT_EQ(tokens[2].kind, TokenKind::eKwFn);
		EXPECT_EQ(tokens[2].loc.line, 2u);
	}

	TEST(LexerTest, BadIncludeIsDiagnostic) {
		EXPECT_EQ(lexFail("#include foo").size(), 1uz);
		EXPECT_EQ(lexFail("#include <foo").size(), 1uz);
		EXPECT_EQ(lexFail("#define x").size(), 1uz);
	}

	TEST(LexerTest, UnknownCharacterIsLocatedAndLexingContinues) {
		const auto diags = lexFail("a\n b $ c ` d");
		ASSERT_EQ(diags.size(), 2uz);
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 4u);
		EXPECT_EQ(diags[1].column, 8u);
	}
}

/**/

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		/* returns the owning callable; a ModuleResolver only borrows it */
		[[nodiscard]] auto includesFrom(std::map<String, String, std::less<>> sources_) {
			return [sources = std::move(sources_)](StringView name_) -> Opt<String> {
				const auto it = sources.find(name_);
				if (it == sources.end()) {
					return None;
				}
				return Opt<String> { it->second };
			};
		}

		[[nodiscard]] Module parseOk(StringView source_, ref<const ModuleResolver> includes_ = noIncludes, EntryKind entry_ = EntryKind::eFragment) {
			auto res = parseModule("main", source_, entry_, includes_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] Diagnostics parseFail(StringView source_, ref<const ModuleResolver> includes_ = noIncludes) {
			auto res = parseModule("main", source_, EntryKind::eFragment, includes_);
			EXPECT_FALSE(res.has_value());
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] bool mentions(ref<const Diagnostic> diag_, StringView text_) {
			return diag_.message.find(text_) != String::npos;
		}
	}

	TEST(ParserTest, ParsesToggleWithDefault) {
		const auto mod = parseOk("toggle Shading : { Forward, Deferred } = Deferred;");
		ASSERT_EQ(mod.toggles.size(), 1uz);
		EXPECT_EQ(mod.toggles[0].name, "Shading");
		EXPECT_EQ(mod.toggles[0].values, (Vector<String> { "Forward", "Deferred" }));
		EXPECT_EQ(mod.toggles[0].defaultValue, "Deferred");
		EXPECT_EQ(mod.name, "main");
		EXPECT_EQ(mod.entry, EntryKind::eFragment);
	}

	TEST(ParserTest, ParsesBoundBlockWithAnonymousStruct) {
		const auto mod = parseOk(R"(@input (mode = bind, id = "camera") block camera { f32mat4 vp; };)");
		ASSERT_EQ(mod.bindings.size(), 1uz);
		const auto& binding = mod.bindings[0];
		EXPECT_EQ(binding.id, "camera");
		EXPECT_EQ(binding.localName, "camera");
		EXPECT_EQ(binding.mode, BindMode::eBind);
		EXPECT_TRUE(binding.input);

		ASSERT_LT(binding.type.index, mod.types.size());
		const auto& block = mod.types[binding.type.index];
		EXPECT_EQ(block.kind, TypeKind::eStruct);
		ASSERT_EQ(block.fields.size(), 1uz);
		EXPECT_EQ(block.fields[0].name, "vp");

		ASSERT_LT(block.fields[0].type.index, mod.types.size());
		const auto& mat = mod.types[block.fields[0].type.index];
		EXPECT_EQ(mat.kind, TypeKind::eMatrix);
		EXPECT_EQ(mat.scalar, ScalarKind::eFloat);
		EXPECT_EQ(mat.n, 4u);
		EXPECT_EQ(mat.m, 4u);
	}

	TEST(ParserTest, ParsesWhenBlockAffectsOnlyInnerDecls) {
		const auto mod = parseOk(
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"@when (Shading == Forward) {\n"
			"\t@input (mode = forward) f32vec3 light;\n"
			"\tfn Lit() -> f32vec4;\n"
			"}\n"
			"@input (mode = forward) f32vec3 normal;\n"
		);
		ASSERT_EQ(mod.bindings.size(), 2uz);
		EXPECT_EQ(mod.bindings[0].localName, "light");
		EXPECT_EQ(mod.bindings[0].when.kind, ConditionKind::eEquals);
		EXPECT_EQ(mod.bindings[0].when.toggle, "Shading");
		EXPECT_EQ(mod.bindings[0].when.value, "Forward");
		EXPECT_EQ(mod.bindings[1].localName, "normal");
		EXPECT_EQ(mod.bindings[1].when.kind, ConditionKind::eTrue);

		ASSERT_EQ(mod.functions.size(), 1uz);
		EXPECT_EQ(mod.functions[0].when.kind, ConditionKind::eEquals);
	}

	TEST(ParserTest, ForwardDeclHasNoBody) {
		const auto mod = parseOk("fn GetLocalToWorld() -> f32mat4;\nfn Mix(a: f32, b: f32vec4[4]) -> f32;");
		ASSERT_EQ(mod.functions.size(), 2uz);
		EXPECT_EQ(mod.functions[0].name, "GetLocalToWorld");
		EXPECT_FALSE(mod.functions[0].body.has_value());
		EXPECT_TRUE(mod.functions[0].params.empty());
		EXPECT_EQ(mod.types[mod.functions[0].ret.index].kind, TypeKind::eMatrix);

		ASSERT_EQ(mod.functions[1].params.size(), 2uz);
		EXPECT_EQ(mod.functions[1].params[0].name, "a");
		EXPECT_EQ(mod.functions[1].params[0].type, mod.functions[1].ret);// f32 interned once
		const auto& arr = mod.types[mod.functions[1].params[1].type.index];
		EXPECT_EQ(arr.kind, TypeKind::eArray);
		EXPECT_EQ(arr.count, 4u);
		EXPECT_EQ(mod.types[arr.element.index].kind, TypeKind::eVector);
	}

	TEST(ParserTest, IncludeAppendsDeclarationsFirst) {
		const auto includes = includesFrom(
			{ { "common", "toggle Shadows : { Off, On } = Off;\nstruct Light { f32vec3 dir; }\nfn Shared() -> f32;" } }
		);
		const auto mod = parseOk(
			"#include <common>\n"
			"@when (Shadows == On) fn Own(l: Light) -> f32;\n",
			includes
		);
		ASSERT_EQ(mod.toggles.size(), 1uz);
		EXPECT_EQ(mod.toggles[0].name, "Shadows");
		ASSERT_EQ(mod.functions.size(), 2uz);
		EXPECT_EQ(mod.functions[0].name, "Shared");
		EXPECT_EQ(mod.functions[1].name, "Own");
		EXPECT_EQ(mod.types[mod.functions[1].params[0].type.index].name, "Light");
	}

	TEST(ParserTest, IncludeCycleIsDiagnostic) {
		const auto includes = includesFrom(
			{ { "a", "#include <b>\nfn A() -> f32;" }, { "b", "#include <a>\nfn B() -> f32;" } }
		);
		const auto diags = parseFail("#include <a>\n", includes);
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "cycle")) << diags[0].message;
		EXPECT_TRUE(mentions(diags[0], "main -> a -> b -> a")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_EQ(diags[0].column, 11u);
	}

	TEST(ParserTest, UnknownToggleInWhenIsDiagnosticWithLocation) {
		const auto diags = parseFail("toggle Shading : { Forward } = Forward;\n@when (Shading == Forward && Nope == X)\nfn F() -> f32;");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "Nope")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 30u);
	}

	TEST(ParserTest, UnknownAnnotationKeyIsDiagnostic) {
		const auto diags = parseFail("@input (mode = forward, colour = \"x\") f32 a;");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "colour")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_EQ(diags[0].column, 25u);
	}

	TEST(ParserTest, UnknownIncludeIsDiagnostic) {
		const auto diags = parseFail("\n#include <missing>\nfn F() -> f32;");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "missing")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 11u);
	}

	/* Rulings pinned beyond the brief */

	TEST(ParserTest, BadFunctionBodyDoesNotCascade) {
		const auto diags = parseFail(
			"fn F() -> f32 {\n\treturn ) ;\n\tif (1.0 { return 2.0; }\n}\n"
			"fn G() -> f32 { return 1.0; }\n"
			"fn H() -> f32 {"
		);
		ASSERT_EQ(diags.size(), 3uz);
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 9u);
		EXPECT_EQ(diags[1].line, 3u);
		EXPECT_EQ(diags[2].line, 6u);// unterminated, at H's '{'
		EXPECT_EQ(diags[2].column, 15u);
	}

	TEST(ParserTest, ZeroValueAndDuplicateTogglesAreDiagnostics) {
		const auto diags = parseFail(
			"toggle Empty : { };\n"
			"toggle A : { X, Y } = X;\n"
			"toggle A : { X, Y } = X;\n"
			"toggle B : { P, P } = P;\n"
			"toggle C : { P, Q } = R;\n"
		);
		ASSERT_EQ(diags.size(), 4uz);
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_TRUE(mentions(diags[1], "'A'")) << diags[1].message;
		EXPECT_EQ(diags[1].line, 3u);
		EXPECT_TRUE(mentions(diags[2], "'P'")) << diags[2].message;
		EXPECT_EQ(diags[2].line, 4u);
		EXPECT_TRUE(mentions(diags[3], "'R'")) << diags[3].message;
		EXPECT_EQ(diags[3].line, 5u);
	}

	TEST(ParserTest, WhenSingleDeclarationCoversOnlyThatDeclaration) {
		const auto mod = parseOk(
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"toggle Shadows : { Off, On } = Off;\n"
			"@when (any(Shading == Deferred, !(Shadows != On)))\n"
			"@input (mode = forward) f32vec3 light;\n"
			"@input (mode = forward) f32vec3 normal;\n"
			"@when (Shadows == On) {\n"
			"\t@when (none(Shading == Forward)) @output (mode = forward, id = \"color\") u8vec4 color;\n"
			"}\n"
		);
		ASSERT_EQ(mod.bindings.size(), 3uz);

		const auto& light = mod.bindings[0].when;
		EXPECT_EQ(light.kind, ConditionKind::eOr);
		ASSERT_EQ(light.children.size(), 2uz);
		EXPECT_EQ(light.children[1].kind, ConditionKind::eNot);
		EXPECT_TRUE(evaluate(light, { { "Shading", "Forward" }, { "Shadows", "On" } }));
		EXPECT_FALSE(evaluate(light, { { "Shading", "Forward" }, { "Shadows", "Off" } }));

		EXPECT_EQ(mod.bindings[1].when.kind, ConditionKind::eTrue);

		// nested @when combines with the enclosing block
		const auto& color = mod.bindings[2];
		EXPECT_FALSE(color.input);
		EXPECT_EQ(color.id, "color");
		EXPECT_EQ(color.localName, "color");
		EXPECT_TRUE(evaluate(color.when, { { "Shading", "Deferred" }, { "Shadows", "On" } }));
		EXPECT_FALSE(evaluate(color.when, { { "Shading", "Forward" }, { "Shadows", "On" } }));
		EXPECT_FALSE(evaluate(color.when, { { "Shading", "Deferred" }, { "Shadows", "Off" } }));
	}

	TEST(ParserTest, ParsesBindingClassesImplicitAndRedefine) {
		const auto mod = parseOk(
			"struct Camera { f32mat4 vp; f32vec4[] planes; }\n"
			"@input (mode = bind, id = \"camera\", class = storage) Camera cam;\n"
			"@input (mode = bind, id = \"push\", class = push) block push { u32 cascade; };\n"
			"@input (mode = bind, id = \"material.albedo\", class = texture) texture2d albedo;\n"
			"@input (mode = bind, id = \"material.sampler\", class = sampler) sampler smp;\n"
			"@input (mode = bind, id = \"lights\", class = uniform) block lights { s32 count; bool on; u8 tag; };\n"
			"@input (mode = forward) f32vec2 uv;\n"
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"@redefine (allow = true) fn Vertex() -> f32vec4;\n"
			"fn main();\n"
		);
		ASSERT_EQ(mod.bindings.size(), 7uz);
		EXPECT_EQ(mod.bindings[0].bindClass, BindClass::eStorageBuffer);
		const auto& camera = mod.types[mod.bindings[0].type.index];
		EXPECT_EQ(camera.name, "Camera");
		ASSERT_EQ(camera.fields.size(), 2uz);
		EXPECT_EQ(mod.types[camera.fields[1].type.index].kind, TypeKind::eArray);
		EXPECT_EQ(mod.types[camera.fields[1].type.index].count, 0u);// unsized
		EXPECT_EQ(mod.bindings[1].bindClass, BindClass::ePushConstant);
		EXPECT_EQ(mod.bindings[2].bindClass, BindClass::eTexture);
		EXPECT_EQ(mod.types[mod.bindings[2].type.index].kind, TypeKind::eTexture);
		EXPECT_EQ(mod.bindings[3].bindClass, BindClass::eSampler);
		EXPECT_EQ(mod.types[mod.bindings[3].type.index].kind, TypeKind::eSampler);
		EXPECT_EQ(mod.bindings[4].bindClass, BindClass::eUniformBuffer);

		const auto& lights = mod.types[mod.bindings[4].type.index];
		ASSERT_EQ(lights.fields.size(), 3uz);
		EXPECT_EQ(mod.types[lights.fields[0].type.index].scalar, ScalarKind::eInt);
		EXPECT_EQ(mod.types[lights.fields[1].type.index].scalar, ScalarKind::eBool);
		EXPECT_EQ(mod.types[lights.fields[2].type.index].bits, 8u);

		EXPECT_EQ(mod.bindings[5].id, "uv");// forward without id uses the declared name
		EXPECT_EQ(mod.bindings[5].bindClass, BindClass::eNone);
		EXPECT_EQ(mod.bindings[5].mode, BindMode::eForward);

		EXPECT_TRUE(mod.bindings[6].implicit);
		EXPECT_FALSE(mod.bindings[6].input);
		EXPECT_FALSE(mod.bindings[5].implicit);

		ASSERT_EQ(mod.functions.size(), 2uz);
		EXPECT_TRUE(mod.functions[0].redefineAllowed);
		EXPECT_FALSE(mod.functions[1].redefineAllowed);
		EXPECT_EQ(mod.functions[1].ret.index, TypeRef {}.index);// no return type
	}

	TEST(ParserTest, ReportsSeveralDiagnosticsAndRecovers) {
		const auto diags = parseFail(
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"@input (mode = sideways) f32 a;\n"
			"@when (Shading == Sideways) {\n"
			"\t@input (mode = forward) f32 b;\n"
			"\tstruct Inner { f32 x; }\n"
			"}\n"
			"@input (mode = forward) vec3 c;\n"
			"@shiny fn F() -> f32;\n"
			"f32 global;\n"
			"#include <late>\n"
		);
		ASSERT_EQ(diags.size(), 7uz);
		EXPECT_TRUE(mentions(diags[0], "sideways")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_TRUE(mentions(diags[1], "Sideways")) << diags[1].message;
		EXPECT_EQ(diags[1].line, 3u);
		EXPECT_EQ(diags[2].line, 5u);// struct inside @when
		EXPECT_TRUE(mentions(diags[3], "vec3")) << diags[3].message;
		EXPECT_EQ(diags[3].line, 7u);
		EXPECT_TRUE(mentions(diags[4], "shiny")) << diags[4].message;
		EXPECT_EQ(diags[4].line, 8u);
		EXPECT_EQ(diags[5].line, 9u);
		EXPECT_EQ(diags[6].line, 10u);// include after declarations
	}

	TEST(ParserTest, IncludedDiagnosticsNameTheModule) {
		const auto includes = includesFrom({ { "broken", "\n  @input (mode = bind) nope x;" }, { "lexbad", "$" } });
		const auto diags = parseFail("#include <broken>\n#include <lexbad>\n", includes);
		ASSERT_EQ(diags.size(), 2uz);
		EXPECT_TRUE(mentions(diags[0], "in 'broken'")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 24u);
		EXPECT_TRUE(mentions(diags[1], "in 'lexbad'")) << diags[1].message;
	}

	/* Nesting depth: one limit (256) bounds every recursive production; deep input yields one located diagnostic */

	namespace {
		[[nodiscard]] String repeat(StringView text_, size_t count_) {
			String out {};
			for (auto i = 0uz; i < count_; ++i) {
				out += text_;
			}
			return out;
		}

		constexpr StringView toggleAB = "toggle A : { B } = B;\n";

		void expectOnlyDepthError(ref<const Diagnostics> diags_, u32 line_, u32 column_) {
			ASSERT_EQ(diags_.size(), 1uz);
			EXPECT_TRUE(mentions(diags_[0], "too deep")) << diags_[0].message;
			EXPECT_EQ(diags_[0].line, line_);
			EXPECT_EQ(diags_[0].column, column_);
		}

		[[nodiscard]] auto includeChain(size_t length_, String leaf_ = "fn Leaf();") {
			// module mN includes m(N+1) until the chain ends (never cyclic)
			return [length_, leaf = std::move(leaf_)](StringView name_) -> Opt<String> {
				const auto n = std::stoull(String(name_.substr(1uz)));
				if (n >= length_) {
					return Opt<String> { leaf };
				}
				return Opt<String> { "#include <m" + std::to_string(n + 1uLL) + ">\n" };
			};
		}
	}

	TEST(ParserTest, DeepParenthesesAreOneDepthDiagnostic) {
		const auto deep = String(toggleAB) + "@when (" + repeat("(", 5000uz) + "A == B" + repeat(")", 5000uz) + ") fn F();";
		// unaryCondition #257 sits on the 257th inner '(' (columns start at 8)
		expectOnlyDepthError(parseFail(deep), 2u, 264u);

		const auto legal = String(toggleAB) + "@when (" + repeat("(", 100uz) + "A == B" + repeat(")", 100uz) + ") fn F();";
		EXPECT_EQ(parseOk(legal).functions.size(), 1uz);
	}

	TEST(ParserTest, DeepNegationIsOneDepthDiagnostic) {
		expectOnlyDepthError(parseFail(String(toggleAB) + "@when (" + repeat("!", 5000uz) + "A == B) fn F();"), 2u, 264u);
		EXPECT_EQ(parseOk(String(toggleAB) + "@when (" + repeat("!", 100uz) + "A == B) fn F();").functions.size(), 1uz);
	}

	TEST(ParserTest, DeepCombinatorsAreOneDepthDiagnostic) {
		const auto nested = [](size_t levels_) {
			return String(toggleAB) + "@when (" + repeat("any(all(none(", levels_) + "A == B" + repeat(")))", levels_) + ") fn F();";
		};
		// unaryCondition #257 = 3 * 85 + 2: the `all` of the 86th "any(all(none(" group (13 columns each)
		expectOnlyDepthError(parseFail(nested(2000uz)), 2u, 8u + 85u * 13u + 4u);

		EXPECT_EQ(parseOk(nested(33uz)).functions.size(), 1uz);// 99 levels
	}

	TEST(ParserTest, DeepWhenBlocksAreOneDepthDiagnostic) {
		// balanced: 5000 nested blocks; block #256 parses the condition of #257 at depth 257
		const auto balanced = String(toggleAB) + repeat("@when (A == B) {\n", 5000uz) + "fn F();\n" + repeat("}\n", 5000uz) + "fn G();";
		expectOnlyDepthError(parseFail(balanced), 258u, 8u);

		// unbalanced: the enclosing blocks are not reported as unterminated on top
		expectOnlyDepthError(parseFail(String(toggleAB) + repeat("@when (A == B) {\n", 5000uz)), 258u, 8u);

		const auto legal = String(toggleAB) + repeat("@when (A == B) {\n", 100uz) + "fn F();\n" + repeat("}\n", 100uz);
		const auto mod = parseOk(legal);
		ASSERT_EQ(mod.functions.size(), 1uz);
		EXPECT_TRUE(evaluate(mod.functions[0].when, { { "A", "B" } }));
	}

	TEST(ParserTest, DeepIncludeChainIsOneDepthDiagnostic) {
		const auto diags = parseFail("#include <m1>\n", includeChain(5000uz));
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "too deep")) << diags[0].message;
		EXPECT_TRUE(mentions(diags[0], "in 'm256'")) << diags[0].message;// including m257 is nesting level 257
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_EQ(diags[0].column, 11u);

		EXPECT_EQ(parseOk("#include <m1>\n", includeChain(100uz)).functions.size(), 1uz);
	}

	TEST(ParserTest, IncludeAndBlockNestingShareOneBudget) {
		// m200 sits at include depth 200, so its 57th @when condition reaches level 257
		const auto leaf = String(toggleAB) + repeat("@when (A == B) {\n", 100uz) + "fn F();\n" + repeat("}\n", 100uz);
		const auto diags = parseFail("#include <m1>\n", includeChain(200uz, leaf));
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "in 'm200'")) << diags[0].message;
		EXPECT_TRUE(mentions(diags[0], "too deep")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 58u);
		EXPECT_EQ(diags[0].column, 8u);
	}

	TEST(ParserTest, DiamondIncludeIsAppendedOnce) {
		const auto includes = includesFrom(
			{
				{ "base", "toggle Shadows : { Off, On } = Off;" },
				{ "left", "#include <base>\nfn L() -> f32;" },
				{ "right", "#include <base>\n@when (Shadows == On) fn R() -> f32;" }
			}
		);
		const auto mod = parseOk("#include <left>\n#include <right>\n", includes);
		EXPECT_EQ(mod.toggles.size(), 1uz);
		ASSERT_EQ(mod.functions.size(), 2uz);
		EXPECT_EQ(mod.functions[0].name, "L");
		EXPECT_EQ(mod.functions[1].name, "R");
	}
}

/**/

#include <Engine.Accel.Lang/Ir/Analysis.hpp>

namespace AccelLang {
	namespace {
		/* a missing function or body throws out_of_range, which fails the test */
		[[nodiscard]] ref<const Stmt> bodyRoot(ref<const Module> mod_, size_t function_) {
			return mod_.stmts.at(mod_.functions.at(function_).body.value_or(BodyRef {}).root.index);
		}

		[[nodiscard]] ref<const Stmt> stmtAt(ref<const Module> mod_, StmtRef ref_) {
			return mod_.stmts.at(ref_.index);
		}

		[[nodiscard]] ref<const Expr> exprAt(ref<const Module> mod_, ExprRef ref_) {
			return mod_.exprs.at(ref_.index);
		}

		constexpr auto unset = ~0uL;
	}

	TEST(ParserBodyTest, ParsesShadowVertexMain) {
		const auto mod = parseOk(
			"@input (mode = bind, id = \"camera\", class = storage) block camera { f32mat4[4] cascadeViewProj; f32vec4 cascadeDepth; };\n"
			"@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n"
			"@input (mode = bind, id = \"push\", class = push) block push { u32 cascade; };\n"
			"@input (mode = forward) f32vec3 local_position;\n"
			"fn main() {\n"
			"\tlet pos = model.model[instance_index] * f32vec4(local_position, 1.0);\n"
			"\tposition = camera.cascadeViewProj[push.cascade] * pos;\n"
			"}\n"
		, noIncludes, EntryKind::eVertex);
		ASSERT_EQ(mod.functions.size(), 1uz);
		const auto& root = bodyRoot(mod, 0uz);
		EXPECT_EQ(root.kind, StmtKind::eBlock);
		ASSERT_EQ(root.children.size(), 2uz);

		const auto& let = stmtAt(mod, root.children[0]);
		EXPECT_EQ(let.kind, StmtKind::eDeclareLocal);
		EXPECT_EQ(let.name, "pos");
		EXPECT_EQ(exprAt(mod, let.lhs).kind, ExprKind::eBinary);

		const auto& assign = stmtAt(mod, root.children[1]);
		EXPECT_EQ(assign.kind, StmtKind::eAssign);
		EXPECT_EQ(exprAt(mod, assign.lhs).kind, ExprKind::eBuiltin);
		const auto& rhs = exprAt(mod, assign.rhs);
		ASSERT_EQ(rhs.operands.size(), 2uz);
		EXPECT_TRUE(exprAt(mod, rhs.operands[1]).local);// pos

		const auto usage = usageOf(mod, 0u);
		EXPECT_TRUE(usage.calledFunctions.empty());
		EXPECT_EQ(usage.readBindings, (Vector<u32> { 0u, 1u, 2u, 3u }));// camera, model, push, local_position
		EXPECT_TRUE(usage.writtenBindings.empty());// `position` is a builtin
	}

	TEST(ParserBodyTest, UnresolvedNameIsDiagnostic) {
		const auto diags = parseFail("fn main() {\n\tposition = f32vec4(nope, 1.0);\n}\n");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "Unknown name 'nope'")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 21u);
	}

	TEST(ParserBodyTest, WhenStatementKeepsCondition) {
		const auto mod = parseOk(
			"toggle Shadows : { Off, On } = Off;\n"
			"fn main() {\n"
			"\t@when (Shadows == On) position = f32vec4(1.0);\n"
			"\t@when (Shadows == Off) { position = f32vec4(0.5); }\n"
			"}\n"
		, noIncludes, EntryKind::eVertex);
		const auto& root = bodyRoot(mod, 0uz);
		ASSERT_EQ(root.children.size(), 2uz);

		const auto& single = stmtAt(mod, root.children[0]);
		EXPECT_EQ(single.kind, StmtKind::eWhen);
		EXPECT_EQ(single.when.kind, ConditionKind::eEquals);
		EXPECT_EQ(single.when.toggle, "Shadows");
		EXPECT_EQ(single.when.value, "On");
		ASSERT_EQ(single.children.size(), 1uz);
		EXPECT_EQ(stmtAt(mod, single.children[0]).kind, StmtKind::eAssign);

		const auto& block = stmtAt(mod, root.children[1]);
		EXPECT_EQ(block.kind, StmtKind::eWhen);
		EXPECT_EQ(block.when.value, "Off");
		ASSERT_EQ(block.children.size(), 1uz);
		EXPECT_EQ(stmtAt(mod, block.children[0]).kind, StmtKind::eBlock);

		const auto diags = parseFail("toggle Shadows : { Off, On } = Off;\nfn main() {\n\t@when (Nope == On) frag_depth = 1.0;\n}\n");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "Unknown toggle 'Nope'")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 3u);
		EXPECT_EQ(diags[0].column, 9u);
	}

	TEST(ParserBodyTest, EmptyMainIsValid) {
		const auto mod = parseOk("@output (mode = forward, id = \"depth\") implicit f32 depth;\nfn main() {}\n");
		ASSERT_EQ(mod.bindings.size(), 1uz);
		EXPECT_TRUE(mod.bindings[0].implicit);
		ASSERT_EQ(mod.functions.size(), 1uz);
		const auto& root = bodyRoot(mod, 0uz);
		EXPECT_EQ(root.kind, StmtKind::eBlock);
		EXPECT_TRUE(root.children.empty());
		EXPECT_TRUE(usageOf(mod, 0u).writtenBindings.empty());
	}

	/* Rulings pinned beyond the brief */

	TEST(ParserBodyTest, LocalShadowingBindingIsNotABindingUse) {
		const auto mod = parseOk(
			"@input (mode = forward) f32vec4 color;\n"
			"@output (mode = forward) f32vec4 outColor;\n"
			"fn main() {\n"
			"\tlet color = f32vec4(1.0);\n"
			"\toutColor = color;\n"
			"}\n"
		);
		const auto& assign = stmtAt(mod, bodyRoot(mod, 0uz).children.at(1));
		const auto& read = exprAt(mod, assign.rhs);
		EXPECT_EQ(read.kind, ExprKind::eNameRef);
		EXPECT_TRUE(read.local);
		EXPECT_EQ(read.binding.index, unset);
		EXPECT_EQ(exprAt(mod, assign.lhs).binding.index, 1u);

		const auto usage = usageOf(mod, 0u);
		EXPECT_TRUE(usage.readBindings.empty());
		EXPECT_EQ(usage.writtenBindings, (Vector<u32> { 1u }));
	}

	TEST(ParserBodyTest, LocalScopeEndsAtBlockEnd) {
		const auto diags = parseFail("fn main() {\n\t{ let a = 1.0; }\n\tfrag_depth = a;\n}\n");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "'a'")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 3u);
		EXPECT_EQ(diags[0].column, 15u);

		// an inner shadow ends with its block; the parameter is visible again
		const auto mod = parseOk("fn F(x: f32) -> f32 {\n\t{ let x = 2.0; frag_depth = x; }\n\treturn x;\n}\n");
		const auto& root = bodyRoot(mod, 0uz);
		ASSERT_EQ(root.children.size(), 2uz);
		EXPECT_TRUE(exprAt(mod, stmtAt(mod, root.children[1]).lhs).local);

		// duplicates in one scope are diagnostics, parameters included
		const auto dups = parseFail("fn G(a: f32, a: f32);\nfn H() {\n\tlet b = 1.0;\n\tlet b = 2.0;\n}\n");
		ASSERT_EQ(dups.size(), 2uz);
		EXPECT_EQ(dups[0].line, 1u);
		EXPECT_EQ(dups[0].column, 14u);
		EXPECT_EQ(dups[1].line, 4u);
		EXPECT_EQ(dups[1].column, 6u);
	}

	TEST(ParserBodyTest, CallsResolveForwardAndUnknownCallIsDiagnostic) {
		const auto mod = parseOk(
			"fn Decl() -> f32;\n"
			"fn main() {\n"
			"\tfrag_depth = Later(1.0) + Decl();\n"
			"}\n"
			"fn Later(v: f32) -> f32 { return v; }\n"
			"fn Decl() -> f32 { return 0.5; }\n"
		);
		ASSERT_EQ(mod.functions.size(), 4uz);
		const auto usage = usageOf(mod, 1u);
		EXPECT_EQ(usage.calledFunctions, (Vector<u32> { 2u, 3u }));// Decl resolves to its body, not the forward decl
		const std::array<u32, 1> roots { 1u };
		EXPECT_EQ(reachableFunctions(mod, roots), (Vector<u32> { 1u, 2u, 3u }));

		const auto diags = parseFail("fn main() {\n\tMissing(1.0);\n}\n");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "Unknown function 'Missing'")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 2u);
	}

	TEST(ParserBodyTest, BuiltinNamesAndSample) {
		const auto mod = parseOk(
			"@input (mode = bind, id = \"albedo\", class = texture) texture2d albedo;\n"
			"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
			"@input (mode = forward) f32vec2 uv;\n"
			"@output (mode = forward) f32vec4 color;\n"
			"fn main() {\n"
			"\tposition = f32vec4(f32(vertex_index), f32(instance_index), 0.0, 1.0);\n"
			"\tframe_depth_is_not_a_builtin();\n"
			"}\n"
			"fn frame_depth_is_not_a_builtin() {\n"
			"\tposition = f32vec4(1.0);\n"
			"\tcolor = sample(albedo, smp, uv);\n"
			"}\n"
		, noIncludes, EntryKind::eVertex);
		const auto& first = stmtAt(mod, bodyRoot(mod, 0uz).children.at(0));
		const auto& position = exprAt(mod, first.lhs);
		EXPECT_EQ(position.kind, ExprKind::eBuiltin);
		EXPECT_EQ(position.name, "position");
		const auto& ctor = exprAt(mod, first.rhs);
		ASSERT_EQ(ctor.operands.size(), 4uz);
		const auto& vertexIndex = exprAt(mod, exprAt(mod, ctor.operands[0]).operands.at(0));
		EXPECT_EQ(vertexIndex.kind, ExprKind::eBuiltin);
		EXPECT_EQ(vertexIndex.name, "vertex_index");

		const auto& sample = exprAt(mod, stmtAt(mod, bodyRoot(mod, 1uz).children.at(1)).rhs);
		EXPECT_EQ(sample.kind, ExprKind::eBuiltin);
		EXPECT_EQ(sample.name, "sample");
		EXPECT_EQ(sample.operands.size(), 3uz);

		const auto usage = usageOf(mod, 1u);
		EXPECT_EQ(usage.readBindings, (Vector<u32> { 0u, 1u, 2u }));
		EXPECT_EQ(usage.writtenBindings, (Vector<u32> { 3u }));// frag_depth is not a binding
		EXPECT_TRUE(usageOf(mod, 0u).writtenBindings.empty());
	}

	TEST(ParserBodyTest, IfElseAndPrecedenceStructure) {
		const auto mod = parseOk(
			"fn F(a: f32, b: f32) -> f32 {\n"
			"\tif (a + 1.0 < b * 2.0 - a) return 0.0; else if (a >= 1.0) { return -a; } else return a - b - 1.0;\n"
			"\tif (!(a == b)) frag_depth = a;\n"
			"\treturn a;\n"
			"}\n"
		);
		const auto& root = bodyRoot(mod, 0uz);
		ASSERT_EQ(root.children.size(), 3uz);

		const auto& outer = stmtAt(mod, root.children[0]);
		EXPECT_EQ(outer.kind, StmtKind::eIf);
		ASSERT_EQ(outer.children.size(), 2uz);
		const auto& cond = exprAt(mod, outer.lhs);// (a + 1.0) < ((b * 2.0) - a)
		EXPECT_EQ(cond.kind, ExprKind::eBinary);
		EXPECT_EQ(cond.name, "<");
		EXPECT_EQ(exprAt(mod, cond.operands.at(0)).name, "+");
		const auto& minus = exprAt(mod, cond.operands.at(1));
		EXPECT_EQ(minus.name, "-");
		EXPECT_EQ(exprAt(mod, minus.operands.at(0)).name, "*");
		EXPECT_EQ(stmtAt(mod, outer.children[0]).kind, StmtKind::eReturn);

		const auto& elseIf = stmtAt(mod, outer.children[1]);
		EXPECT_EQ(elseIf.kind, StmtKind::eIf);
		ASSERT_EQ(elseIf.children.size(), 2uz);
		EXPECT_EQ(exprAt(mod, elseIf.lhs).name, ">=");
		EXPECT_EQ(stmtAt(mod, elseIf.children[0]).kind, StmtKind::eBlock);
		const auto& last = exprAt(mod, stmtAt(mod, elseIf.children[1]).lhs);// (a - b) - 1.0
		EXPECT_EQ(last.name, "-");
		EXPECT_EQ(exprAt(mod, last.operands.at(0)).name, "-");
		EXPECT_EQ(exprAt(mod, last.operands.at(1)).kind, ExprKind::eLiteral);

		const auto& noElse = stmtAt(mod, root.children[1]);
		EXPECT_EQ(noElse.kind, StmtKind::eIf);
		EXPECT_EQ(noElse.children.size(), 1uz);
		const auto& negation = exprAt(mod, noElse.lhs);
		EXPECT_EQ(negation.kind, ExprKind::eUnary);
		EXPECT_EQ(negation.name, "!");
		EXPECT_EQ(exprAt(mod, negation.operands.at(0)).name, "==");

		const auto& ret = stmtAt(mod, root.children[2]);
		EXPECT_EQ(ret.kind, StmtKind::eReturn);
		EXPECT_TRUE(exprAt(mod, ret.lhs).local);

		const auto empty = parseOk("fn G() { return; }");
		EXPECT_EQ(stmtAt(empty, bodyRoot(empty, 0uz).children.at(0)).lhs.index, unset);

		// the condition and both arms count for usageOf
		const auto used = parseOk(
			"@input (mode = forward) f32 level;\n@output (mode = forward) f32 o;\n"
			"fn H() { if (level < 0.5) { o = 1.0; } else o = 0.0; }"
		);
		EXPECT_EQ(usageOf(used, 0u).readBindings, (Vector<u32> { 0u }));
		EXPECT_EQ(usageOf(used, 0u).writtenBindings, (Vector<u32> { 1u }));
	}

	TEST(ParserBodyTest, BothLocalDeclarationForms) {
		const auto mod = parseOk(
			"struct Light { f32vec3 dir; }\n"
			"fn main() {\n"
			"\tlet v: f32vec4 = f32vec4(1.0);\n"
			"\tf32vec4 w = v;\n"
			"\tlet u = w;\n"
			"\tf32[4] arr;\n"
			"\tlet t: f32;\n"
			"\tLight l = Light(w.xyz);\n"
			"\tfrag_depth = arr[0] + t + u.x + l.dir.y;\n"
			"}\n"
		);
		const auto& root = bodyRoot(mod, 0uz);
		ASSERT_EQ(root.children.size(), 7uz);

		const auto& v = stmtAt(mod, root.children[0]);
		const auto& w = stmtAt(mod, root.children[1]);
		const auto& u = stmtAt(mod, root.children[2]);
		const auto& arr = stmtAt(mod, root.children[3]);
		const auto& t = stmtAt(mod, root.children[4]);
		const auto& l = stmtAt(mod, root.children[5]);
		for (const auto* decl : { &v, &w, &u, &arr, &t, &l }) {
			EXPECT_EQ(decl->kind, StmtKind::eDeclareLocal);
		}
		EXPECT_EQ(v.name, "v");
		EXPECT_EQ(mod.types.at(v.type.index).kind, TypeKind::eVector);
		EXPECT_EQ(w.name, "w");
		EXPECT_EQ(w.type, v.type);
		EXPECT_TRUE(exprAt(mod, w.lhs).local);
		EXPECT_EQ(u.type, v.type);// inferred by typecheck
		EXPECT_NE(u.lhs.index, unset);
		EXPECT_EQ(mod.types.at(arr.type.index).kind, TypeKind::eArray);
		EXPECT_EQ(arr.lhs.index, unset);// no initializer
		EXPECT_EQ(mod.types.at(t.type.index).kind, TypeKind::eScalar);
		EXPECT_EQ(t.lhs.index, unset);
		EXPECT_EQ(mod.types.at(l.type.index).name, "Light");

		const auto diags = parseFail("fn main() {\n\tlet z;\n\tnope q = 1.0;\n}\n");
		ASSERT_EQ(diags.size(), 2uz);
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 6u);
		EXPECT_TRUE(mentions(diags[1], "Unknown type 'nope'")) << diags[1].message;
		EXPECT_EQ(diags[1].line, 3u);
	}

	TEST(ParserBodyTest, FieldAndIndexWritesCountAsBaseBindingWrites) {
		const auto mod = parseOk(
			"@output (mode = bind, id = \"camera\", class = storage) block camera { f32mat4 vp; };\n"
			"@output (mode = bind, id = \"a\", class = storage) f32[8] a;\n"
			"@input (mode = forward) u32 i;\n"
			"@input (mode = forward) f32mat4 x;\n"
			"fn main() {\n"
			"\tcamera.vp = x;\n"
			"\ta[i] = 1.0;\n"
			"}\n"
		);
		const auto usage = usageOf(mod, 0u);
		EXPECT_EQ(usage.writtenBindings, (Vector<u32> { 0u, 1u }));// camera, a
		EXPECT_EQ(usage.readBindings, (Vector<u32> { 2u, 3u }));// i, x

		const auto& index = exprAt(mod, stmtAt(mod, bodyRoot(mod, 0uz).children.at(1)).lhs);
		EXPECT_EQ(index.kind, ExprKind::eIndex);
		ASSERT_EQ(index.operands.size(), 2uz);
		EXPECT_EQ(exprAt(mod, index.operands[0]).binding.index, 1u);
		EXPECT_EQ(exprAt(mod, index.operands[1]).binding.index, 2u);

		const auto diags = parseFail("fn F() -> f32;\nfn main() {\n\t1.0 = frag_depth;\n\tF() = 1.0;\n}\n");
		ASSERT_EQ(diags.size(), 2uz);
		EXPECT_TRUE(mentions(diags[0], "Cannot assign")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 3u);
		EXPECT_EQ(diags[0].column, 2u);
		EXPECT_EQ(diags[1].line, 4u);
	}

	TEST(ParserBodyTest, ConstructorAndSwizzleShape) {
		const auto mod = parseOk(
			"struct Light { f32vec3 dir; f32 power; }\n"
			"@input (mode = forward) f32vec4 a;\n"
			"@output (mode = forward) f32vec4 o;\n"
			"fn main() {\n"
			"\to = f32vec4(a.xy, 0.2, -a.w);\n"
			"\tlet l = Light(a.rgb, 1.0);\n"
			"}\n"
		);
		const auto& root = bodyRoot(mod, 0uz);
		const auto& ctor = exprAt(mod, stmtAt(mod, root.children.at(0)).rhs);
		EXPECT_EQ(ctor.kind, ExprKind::eConstruct);
		ASSERT_LT(ctor.type.index, mod.types.size());
		EXPECT_EQ(mod.types[ctor.type.index].kind, TypeKind::eVector);
		EXPECT_EQ(mod.types[ctor.type.index].n, 4u);
		EXPECT_EQ(mod.types[ctor.type.index].bits, 32u);
		EXPECT_EQ(ctor.type, mod.bindings[0].type);// interned once
		ASSERT_EQ(ctor.operands.size(), 3uz);

		const auto& swizzle = exprAt(mod, ctor.operands[0]);
		EXPECT_EQ(swizzle.kind, ExprKind::eField);
		EXPECT_EQ(swizzle.name, "xy");
		ASSERT_EQ(swizzle.operands.size(), 1uz);
		EXPECT_EQ(exprAt(mod, swizzle.operands[0]).binding.index, 0u);

		const auto& literal = exprAt(mod, ctor.operands[1]);
		EXPECT_EQ(literal.kind, ExprKind::eLiteral);
		EXPECT_EQ(literal.name, "0.2");

		const auto& negated = exprAt(mod, ctor.operands[2]);// postfix binds tighter than unary
		EXPECT_EQ(negated.kind, ExprKind::eUnary);
		EXPECT_EQ(exprAt(mod, negated.operands.at(0)).kind, ExprKind::eField);

		const auto& light = exprAt(mod, stmtAt(mod, root.children.at(1)).lhs);
		EXPECT_EQ(light.kind, ExprKind::eConstruct);
		EXPECT_EQ(mod.types[light.type.index].name, "Light");
		EXPECT_EQ(light.operands.size(), 2uz);
	}

	TEST(ParserBodyTest, SwitchIsDiagnostic) {
		const auto diags = parseFail("fn main() {\n\tswitch (vertex_index) { }\n\tfrag_depth = 2.0;\n}\n");
		EXPECT_FALSE(diags.empty());
	}

	TEST(ParserBodyTest, EofInsideBodyIsOneDiagnostic) {
		const auto open = parseFail("fn main() {\n\tif (true) {\n\t\tfrag_depth = 1.0;");
		ASSERT_EQ(open.size(), 1uz);
		EXPECT_TRUE(mentions(open[0], "Unterminated")) << open[0].message;
		EXPECT_EQ(open[0].line, 2u);
		EXPECT_EQ(open[0].column, 12u);

		const auto midExpr = parseFail("fn main() {\n\tfrag_depth = f32(1.0");
		ASSERT_EQ(midExpr.size(), 1uz);
		EXPECT_TRUE(mentions(midExpr[0], "end of input")) << midExpr[0].message;
		EXPECT_EQ(midExpr[0].line, 2u);
	}

	TEST(ParserBodyTest, DeepExpressionsAreOneDepthDiagnostic) {
		const auto body = [](ref<const String> expr_) {
			return "fn main() {\n\tfrag_depth = " + expr_ + ";\n}\n";
		};
		// statement is level 1, so unary #256 (column 15 + 255) is level 257
		expectOnlyDepthError(parseFail(body(repeat("(", 5000uz) + "1.0" + repeat(")", 5000uz))), 2u, 270u);
		expectOnlyDepthError(parseFail(body(repeat("- ", 5000uz) + "1.0")), 2u, 15u + 255u * 2u);
		expectOnlyDepthError(parseFail(body(repeat("!", 5000uz) + "true")), 2u, 270u);
		expectOnlyDepthError(parseFail(body(repeat("f32(", 5000uz) + "1.0" + repeat(")", 5000uz))), 2u, 15u + 255u * 4u);

		EXPECT_EQ(parseOk(body(repeat("(", 100uz) + "1.0" + repeat(")", 100uz))).functions.size(), 1uz);
		EXPECT_EQ(parseOk(body(repeat("- ", 100uz) + "1.0")).functions.size(), 1uz);
		EXPECT_EQ(parseOk("fn main() {\n\tlet b = " + repeat("!", 100uz) + "true;\n}\n").functions.size(), 1uz);
	}

	TEST(ParserBodyTest, DeepStatementsAreOneDepthDiagnostic) {
		// statement #257 is the 257th '{'
		expectOnlyDepthError(parseFail("fn main() {\n" + repeat("{", 5000uz) + repeat("}", 5000uz) + "\n}\n"), 2u, 257u);
		// unbalanced: enclosing blocks are not reported as unterminated on top
		expectOnlyDepthError(parseFail("fn main() {\n" + repeat("{", 5000uz)), 2u, 257u);
		// if #256 is level 256, its condition `true` (column 1 + 255 * 10 + 4) level 257
		expectOnlyDepthError(parseFail("fn main() {\n" + repeat("if (true) ", 5000uz) + "frag_depth = 1.0;\n}\n"), 2u, 2555u);

		EXPECT_EQ(parseOk("fn main() {\n" + repeat("{", 100uz) + repeat("}", 100uz) + "\n}\n").functions.size(), 1uz);
		EXPECT_EQ(parseOk("fn main() {\n" + repeat("if (true) ", 100uz) + "frag_depth = 1.0;\n}\n").functions.size(), 1uz);
	}

	TEST(LexerTest, FloatExponent) {
		const auto tokens = lexOk("2.3283064365386963e-10 1e-4 <<= >>= ++ += ? ~ ^");
		ASSERT_EQ(tokens.size(), 10uz);
		EXPECT_EQ(tokens[0].kind, TokenKind::eFloat);
		EXPECT_EQ(tokens[0].text, "2.3283064365386963e-10");
		EXPECT_EQ(tokens[1].kind, TokenKind::eFloat);
		EXPECT_EQ(tokens[1].text, "1e-4");
	}

	TEST(ParserBodyTest, ParsesForWithCompoundStep) {
		const auto mod = parseOk(
			"fn f(i: u32) -> f32 { return 1.0; }\n"
			"fn main() {\n\tlet n = 4u;\n\tlet acc = 0.0;\n"
			"\tfor (let i = 0u; i < n; ++i) { acc += f(i); }\n\tfrag_depth = acc;\n}\n"
		);
		const auto& root = bodyRoot(mod, 1uz);
		const auto& loop = stmtAt(mod, root.children.at(2));
		ASSERT_EQ(loop.kind, StmtKind::eFor);
		ASSERT_EQ(loop.children.size(), 3uz);
		EXPECT_EQ(stmtAt(mod, loop.children[0]).kind, StmtKind::eDeclareLocal);
		EXPECT_EQ(exprAt(mod, loop.lhs).kind, ExprKind::eBinary);
		const auto& step = stmtAt(mod, loop.children[2]);
		EXPECT_EQ(step.kind, StmtKind::eAssign);
		EXPECT_EQ(step.name, "+=");
		const auto& body = stmtAt(mod, loop.children[1]);
		EXPECT_EQ(stmtAt(mod, body.children.at(0)).name, "+=");
	}

	TEST(ParserBodyTest, ParsesWhileBreakContinue) {
		const auto mod = parseOk(
			"fn main() {\n\tlet x = 0.0;\n\twhile (x < 3.0) { x += 1.0; if (x > 1.0) { continue; } break; }\n\tfrag_depth = x;\n}\n"
		);
		const auto& loop = stmtAt(mod, bodyRoot(mod, 0uz).children.at(1));
		ASSERT_EQ(loop.kind, StmtKind::eWhile);
		const auto& body = stmtAt(mod, loop.children.at(0));
		EXPECT_EQ(stmtAt(mod, body.children.at(2)).kind, StmtKind::eBreak);
		const auto& branch = stmtAt(mod, body.children.at(1));
		EXPECT_EQ(stmtAt(mod, stmtAt(mod, branch.children.at(0)).children.at(0)).kind, StmtKind::eContinue);
	}

	TEST(ParserBodyTest, OperatorPrecedence) {
		const auto mod = parseOk("fn main() {\n\tlet a = 1u;\n\tlet b = 2u;\n\tlet c = 3u;\n\tlet r = a | b & c << 1u;\n}\n");
		const auto& top = exprAt(mod, stmtAt(mod, bodyRoot(mod, 0uz).children.at(3)).lhs);
		ASSERT_EQ(top.name, "|");
		const auto& mid = exprAt(mod, top.operands.at(1));
		ASSERT_EQ(mid.name, "&");
		EXPECT_EQ(exprAt(mod, mid.operands.at(1)).name, "<<");
	}

	TEST(ParserBodyTest, ParsesTernary) {
		const auto mod = parseOk("fn main() {\n\tlet x = true;\n\tlet y = 1.0;\n\tlet z = 2.0;\n\tlet r = x ? y : z + 1.0;\n}\n");
		const auto& ternary = exprAt(mod, stmtAt(mod, bodyRoot(mod, 0uz).children.at(3)).lhs);
		ASSERT_EQ(ternary.kind, ExprKind::eTernary);
		ASSERT_EQ(ternary.operands.size(), 3uz);
		EXPECT_EQ(exprAt(mod, ternary.operands[2]).name, "+");
	}

	TEST(ParserTest, ImplicitOnFunctionIsDiagnostic) {
		const auto diags = parseFail("@output (mode = forward, id = \"depth\") implicit fn main() {}\n");
		ASSERT_FALSE(diags.empty());
		EXPECT_TRUE(mentions(diags[0], "apply to bindings only")) << diags[0].message;
	}

	TEST(ParserTest, SpecOnNonConstIsDiagnostic) {
		const auto diags = parseFail("@spec (id = \"n\") @input (mode = bind, id = \"m\", class = uniform) block m { u32 n; };\n");
		ASSERT_FALSE(diags.empty());
		EXPECT_TRUE(mentions(diags[0], "@spec applies to const declarations only")) << diags[0].message;
	}

	TEST(ParserTest, EarlyFragmentTestsValueMustBeBool) {
		const auto diags = parseFail("@fragment (early_fragment_tests = yes)\nfn main() {}\n");
		ASSERT_FALSE(diags.empty());
		EXPECT_TRUE(mentions(diags[0], "Invalid value 'yes' for annotation key 'early_fragment_tests'")) << diags[0].message;
	}
}
