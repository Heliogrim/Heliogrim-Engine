#include "pch.h"

#include <array>
#include <map>

#include <Engine.Accel.Lang/Ir/Lexer.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>

using namespace hg;
using namespace hg::engine::accel::lang::ir;

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

		[[nodiscard]] ModuleResolver includesFrom(std::map<String, String, std::less<>> sources_) {
			return [sources = std::move(sources_)](StringView name_) -> Opt<String> {
				const auto it = sources.find(name_);
				if (it == sources.end()) {
					return None;
				}
				return Opt<String> { it->second };
			};
		}

		[[nodiscard]] Module parseOk(StringView source_, ref<const ModuleResolver> includes_ = noIncludes) {
			auto res = parseModule("main", source_, EntryKind::eFragment, includes_);
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
		const auto module = parseOk("toggle Shading : { Forward, Deferred } = Deferred;");
		ASSERT_EQ(module.toggles.size(), 1uz);
		EXPECT_EQ(module.toggles[0].name, "Shading");
		EXPECT_EQ(module.toggles[0].values, (Vector<String> { "Forward", "Deferred" }));
		EXPECT_EQ(module.toggles[0].defaultValue, "Deferred");
		EXPECT_EQ(module.name, "main");
		EXPECT_EQ(module.entry, EntryKind::eFragment);
	}

	TEST(ParserTest, ParsesBoundBlockWithAnonymousStruct) {
		const auto module = parseOk(R"(@input (mode = bind, id = "camera") block camera { f32mat4 vp; };)");
		ASSERT_EQ(module.bindings.size(), 1uz);
		const auto& binding = module.bindings[0];
		EXPECT_EQ(binding.id, "camera");
		EXPECT_EQ(binding.localName, "camera");
		EXPECT_EQ(binding.mode, BindMode::eBind);
		EXPECT_TRUE(binding.input);

		ASSERT_LT(binding.type.index, module.types.size());
		const auto& block = module.types[binding.type.index];
		EXPECT_EQ(block.kind, TypeKind::eStruct);
		ASSERT_EQ(block.fields.size(), 1uz);
		EXPECT_EQ(block.fields[0].name, "vp");

		ASSERT_LT(block.fields[0].type.index, module.types.size());
		const auto& mat = module.types[block.fields[0].type.index];
		EXPECT_EQ(mat.kind, TypeKind::eMatrix);
		EXPECT_EQ(mat.scalar, ScalarKind::eFloat);
		EXPECT_EQ(mat.n, 4u);
		EXPECT_EQ(mat.m, 4u);
	}

	TEST(ParserTest, ParsesWhenBlockAffectsOnlyInnerDecls) {
		const auto module = parseOk(
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"@when (Shading == Forward) {\n"
			"\t@input (mode = forward) f32vec3 light;\n"
			"\tfn Lit() -> f32vec4;\n"
			"}\n"
			"@input (mode = forward) f32vec3 normal;\n"
		);
		ASSERT_EQ(module.bindings.size(), 2uz);
		EXPECT_EQ(module.bindings[0].localName, "light");
		EXPECT_EQ(module.bindings[0].when.kind, ConditionKind::eEquals);
		EXPECT_EQ(module.bindings[0].when.toggle, "Shading");
		EXPECT_EQ(module.bindings[0].when.value, "Forward");
		EXPECT_EQ(module.bindings[1].localName, "normal");
		EXPECT_EQ(module.bindings[1].when.kind, ConditionKind::eTrue);

		ASSERT_EQ(module.functions.size(), 1uz);
		EXPECT_EQ(module.functions[0].when.kind, ConditionKind::eEquals);
	}

	TEST(ParserTest, ForwardDeclHasNoBody) {
		const auto module = parseOk("fn GetLocalToWorld() -> f32mat4;\nfn Mix(a: f32, b: f32vec4[4]) -> f32;");
		ASSERT_EQ(module.functions.size(), 2uz);
		EXPECT_EQ(module.functions[0].name, "GetLocalToWorld");
		EXPECT_FALSE(module.functions[0].body.has_value());
		EXPECT_TRUE(module.functions[0].params.empty());
		EXPECT_EQ(module.types[module.functions[0].ret.index].kind, TypeKind::eMatrix);

		ASSERT_EQ(module.functions[1].params.size(), 2uz);
		EXPECT_EQ(module.functions[1].params[0].name, "a");
		EXPECT_EQ(module.functions[1].params[0].type, module.functions[1].ret);// f32 interned once
		const auto& arr = module.types[module.functions[1].params[1].type.index];
		EXPECT_EQ(arr.kind, TypeKind::eArray);
		EXPECT_EQ(arr.count, 4u);
		EXPECT_EQ(module.types[arr.element.index].kind, TypeKind::eVector);
	}

	TEST(ParserTest, IncludeAppendsDeclarationsFirst) {
		const auto includes = includesFrom(
			{ { "common", "toggle Shadows : { Off, On } = Off;\nstruct Light { f32vec3 dir; }\nfn Shared() -> f32;" } }
		);
		const auto module = parseOk(
			"#include <common>\n"
			"@when (Shadows == On) fn Own(l: Light) -> f32;\n",
			includes
		);
		ASSERT_EQ(module.toggles.size(), 1uz);
		EXPECT_EQ(module.toggles[0].name, "Shadows");
		ASSERT_EQ(module.functions.size(), 2uz);
		EXPECT_EQ(module.functions[0].name, "Shared");
		EXPECT_EQ(module.functions[1].name, "Own");
		EXPECT_EQ(module.types[module.functions[1].params[0].type.index].name, "Light");
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

	TEST(ParserTest, FunctionBodyIsDiagnosticUntilTask5) {
		const auto diags = parseFail("fn F() -> f32 {\n\treturn 1.0;\n}\nfn G() -> f32 {");
		ASSERT_EQ(diags.size(), 2uz);
		EXPECT_TRUE(mentions(diags[0], "function bodies are parsed from Task 5 on")) << diags[0].message;
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_EQ(diags[0].column, 15u);
		EXPECT_EQ(diags[1].line, 4u);
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
		const auto module = parseOk(
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"toggle Shadows : { Off, On } = Off;\n"
			"@when (any(Shading == Deferred, !(Shadows != On)))\n"
			"@input (mode = forward) f32vec3 light;\n"
			"@input (mode = forward) f32vec3 normal;\n"
			"@when (Shadows == On) {\n"
			"\t@when (none(Shading == Forward)) @output (mode = forward, id = \"color\") u8vec4 color;\n"
			"}\n"
		);
		ASSERT_EQ(module.bindings.size(), 3uz);

		const auto& light = module.bindings[0].when;
		EXPECT_EQ(light.kind, ConditionKind::eOr);
		ASSERT_EQ(light.children.size(), 2uz);
		EXPECT_EQ(light.children[1].kind, ConditionKind::eNot);
		EXPECT_TRUE(evaluate(light, { { "Shading", "Forward" }, { "Shadows", "On" } }));
		EXPECT_FALSE(evaluate(light, { { "Shading", "Forward" }, { "Shadows", "Off" } }));

		EXPECT_EQ(module.bindings[1].when.kind, ConditionKind::eTrue);

		// nested @when combines with the enclosing block
		const auto& color = module.bindings[2];
		EXPECT_FALSE(color.input);
		EXPECT_EQ(color.id, "color");
		EXPECT_EQ(color.localName, "color");
		EXPECT_TRUE(evaluate(color.when, { { "Shading", "Deferred" }, { "Shadows", "On" } }));
		EXPECT_FALSE(evaluate(color.when, { { "Shading", "Forward" }, { "Shadows", "On" } }));
		EXPECT_FALSE(evaluate(color.when, { { "Shading", "Deferred" }, { "Shadows", "Off" } }));
	}

	TEST(ParserTest, ParsesBindingClassesImplicitAndRedefine) {
		const auto module = parseOk(
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
		ASSERT_EQ(module.bindings.size(), 7uz);
		EXPECT_EQ(module.bindings[0].bindClass, BindClass::eStorageBuffer);
		const auto& camera = module.types[module.bindings[0].type.index];
		EXPECT_EQ(camera.name, "Camera");
		ASSERT_EQ(camera.fields.size(), 2uz);
		EXPECT_EQ(module.types[camera.fields[1].type.index].kind, TypeKind::eArray);
		EXPECT_EQ(module.types[camera.fields[1].type.index].count, 0u);// unsized
		EXPECT_EQ(module.bindings[1].bindClass, BindClass::ePushConstant);
		EXPECT_EQ(module.bindings[2].bindClass, BindClass::eTexture);
		EXPECT_EQ(module.types[module.bindings[2].type.index].kind, TypeKind::eTexture);
		EXPECT_EQ(module.bindings[3].bindClass, BindClass::eSampler);
		EXPECT_EQ(module.types[module.bindings[3].type.index].kind, TypeKind::eSampler);
		EXPECT_EQ(module.bindings[4].bindClass, BindClass::eUniformBuffer);

		const auto& lights = module.types[module.bindings[4].type.index];
		ASSERT_EQ(lights.fields.size(), 3uz);
		EXPECT_EQ(module.types[lights.fields[0].type.index].scalar, ScalarKind::eInt);
		EXPECT_EQ(module.types[lights.fields[1].type.index].scalar, ScalarKind::eBool);
		EXPECT_EQ(module.types[lights.fields[2].type.index].bits, 8u);

		EXPECT_EQ(module.bindings[5].id, "uv");// forward without id uses the declared name
		EXPECT_EQ(module.bindings[5].bindClass, BindClass::eNone);
		EXPECT_EQ(module.bindings[5].mode, BindMode::eForward);

		EXPECT_TRUE(module.bindings[6].implicit);
		EXPECT_FALSE(module.bindings[6].input);
		EXPECT_FALSE(module.bindings[5].implicit);

		ASSERT_EQ(module.functions.size(), 2uz);
		EXPECT_TRUE(module.functions[0].redefineAllowed);
		EXPECT_FALSE(module.functions[1].redefineAllowed);
		EXPECT_EQ(module.functions[1].ret.index, TypeRef {}.index);// no return type
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

	TEST(ParserTest, DiamondIncludeIsAppendedOnce) {
		const auto includes = includesFrom(
			{
				{ "base", "toggle Shadows : { Off, On } = Off;" },
				{ "left", "#include <base>\nfn L() -> f32;" },
				{ "right", "#include <base>\n@when (Shadows == On) fn R() -> f32;" }
			}
		);
		const auto module = parseOk("#include <left>\n#include <right>\n", includes);
		EXPECT_EQ(module.toggles.size(), 1uz);
		ASSERT_EQ(module.functions.size(), 2uz);
		EXPECT_EQ(module.functions[0].name, "L");
		EXPECT_EQ(module.functions[1].name, "R");
	}
}
