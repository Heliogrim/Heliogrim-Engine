#include "pch.h"

#include <Engine.Accel.Compile/Pass/VkVertexFormat.hpp>

using namespace hg;
using namespace hg::engine::accel;

namespace AccelCompile {
	TEST(VertexFormatTest, LegacyFormatsStayTheSame) {
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eU8), vk::Format::eR8Uint);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eU8Vec2), vk::Format::eR8G8Unorm);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eU8Vec3), vk::Format::eR8G8B8Unorm);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eU8Vec4), vk::Format::eR8G8B8A8Unorm);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eF32Vec3), vk::Format::eR32G32B32Sfloat);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eF32Vec4), vk::Format::eR32G32B32A32Sfloat);
	}

	TEST(VertexFormatTest, EveryVertexUsableTypeHasAFormat) {
		using enum lang::ScalarType;
		constexpr std::array<lang::ScalarType, 9> elements { eU8, eS8, eU16, eS16, eU32, eS32, eF16, eF32, eF64 };

		for (const auto element : elements) {
			for (u8 n = 1; n <= 4; ++n) {
				EXPECT_NE(vertexFormatOf(lang::vectorOf(element, n)), vk::Format::eUndefined)
					<< "element 0x" << std::hex << static_cast<u32>(element) << " x" << std::dec << static_cast<u32>(n);
			}
		}
	}

	TEST(VertexFormatTest, WideTypesMapToTheirWidth) {
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eS16Vec2), vk::Format::eR16G16Sint);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eU32Vec3), vk::Format::eR32G32B32Uint);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eF16Vec4), vk::Format::eR16G16B16A16Sfloat);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eF32Vec2), vk::Format::eR32G32Sfloat);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eF64), vk::Format::eR64Sfloat);
	}

	TEST(VertexFormatTest, MatricesAndWideIntegersHaveNoFormat) {
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eF32Mat4x4), vk::Format::eUndefined);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eU64), vk::Format::eUndefined);
		EXPECT_EQ(vertexFormatOf(lang::ScalarType::eS64Vec2), vk::Format::eUndefined);
	}
}
