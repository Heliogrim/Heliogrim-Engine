#include "VkAccelerationPassFactory.hpp"

#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>

#include <Engine.Reflect/IsType.hpp>

#include "MeshPass.hpp"
#include "VkGraphicsPass.hpp"

using namespace hg::engine::accel;
using namespace hg;

tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildGraphicsPass(
	mref<smr<const GraphicsPipeline>> pipeline_
) const noexcept {

	if (not IsType<VkGraphicsPipeline>(*pipeline_)) {
		return tl::make_unexpected(AccelerationPassFactoryError {});
	}

	return tl::make_unexpected(AccelerationPassFactoryError {});
}

tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildGraphicsPass(
	cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
	cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_
) const noexcept {
	return buildGraphicsPass(outputSymbols_, inputSymbols_, {}, 0uL, 0uL);
}

tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildGraphicsPass(
	cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
	cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_,
	mref<Vector<vk::SubpassDependency>> subpassDependencies_,
	u32 viewMask_,
	u32 correlationMask_
) const noexcept {

	auto result = make_smr<VkGraphicsPass>();

	// TODO: We can optimize the resulting passes by using the symbol scope, to check whether we want to use internal linking
	// TODO:    which could indicate that the current pass will be a subpass anyway.

	bool hasDepthStencil = false;

	/* Generate attachment based on descriptions linked by output symbols */

	u32 attachIndex = 0uL;
	for (const auto& symbol : outputSymbols_) {

		assert(IsType<render::graph::TextureDescription>(*symbol->description));
		const auto* const td = Cast<render::graph::TextureDescription>(symbol->description.get());

		/**/

		smr<const render::graph::Symbol> inputSymbol = nullptr;
		const render::graph::TextureDescription* itd = nullptr; {
			const auto inputIt = std::ranges::find_if(
				inputSymbols_,
				[name = symbol->name](const auto& inputSymbol_) {
					return inputSymbol_->name == name;
				}
			);

			if (inputIt != inputSymbols_.end()) {
				inputSymbol = clone(*inputIt);
				itd = Cast<render::graph::TextureDescription>(inputSymbol->description.get());
			}
		}

		/**/

		const auto targetFormat = td->_textureFormat.value;

		vk::ImageLayout sourceLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout targetLayout;
		vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;

		if (isDepthStencilFormat(targetFormat)) {
			targetLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			hasDepthStencil = true;
		} else if (isDepthFormat(targetFormat)) {
			targetLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			hasDepthStencil = true;
		} else if (isStencilFormat(targetFormat)) {
			targetLayout = vk::ImageLayout::eStencilAttachmentOptimal;
			hasDepthStencil = true;
		} else {
			targetLayout = vk::ImageLayout::eColorAttachmentOptimal;
			//targetLayout = vk::ImageLayout::ePresentSrcKHR;
			// Warning: targetLayout = vk::ImageLayout::eAttachmentOptimal;
		}

		if (not inputSymbol.empty()) {

			const auto sourceFormat = itd->_textureFormat.value;

			if (isDepthStencilFormat(sourceFormat)) {
				sourceLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			} else if (isDepthFormat(sourceFormat)) {
				sourceLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			} else if (isStencilFormat(sourceFormat)) {
				sourceLayout = vk::ImageLayout::eStencilAttachmentOptimal;
			} else {
				sourceLayout = vk::ImageLayout::eColorAttachmentOptimal;
				// Warning: sourceLayout = vk::ImageLayout::eAttachmentOptimal;
			}

			loadOp = vk::AttachmentLoadOp::eLoad;
		}

		/**/

		result->_attachments.push_back(
			vk::AttachmentDescription {
				{},
				gfx::api::vkTranslateFormat(targetFormat),
				vk::SampleCountFlagBits::e1,
				loadOp,
				vk::AttachmentStoreOp::eStore,
				isStencilFormat(targetFormat) ? loadOp : vk::AttachmentLoadOp::eDontCare,
				isStencilFormat(targetFormat) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
				sourceLayout,
				targetLayout
			}
		);

		result->_references.emplace_back(
			attachIndex++,
			targetLayout
		);
	}

	/* Generate exclusive attachment based on descriptions linked by input symbols */

	for (const auto& symbol : inputSymbols_) {

		assert(IsType<render::graph::TextureDescription>(*symbol->description));
		const auto* const td = Cast<render::graph::TextureDescription>(symbol->description.get());

		/**/

		if (std::ranges::find_if(
			outputSymbols_,
			[name = symbol->name](const auto& outputSymbol_) {
				return outputSymbol_->name == name;
			}
		) != outputSymbols_.end()) {
			continue;
		}

		/**/

		vk::ImageLayout sourceLayout;
		vk::ImageLayout targetLayout;
		constexpr vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eLoad;

		const auto sourceFormat = td->_textureFormat.value;

		if (isDepthStencilFormat(sourceFormat)) {
			sourceLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			hasDepthStencil = true;
		} else if (isDepthFormat(sourceFormat)) {
			sourceLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			hasDepthStencil = true;
		} else if (isStencilFormat(sourceFormat)) {
			sourceLayout = vk::ImageLayout::eStencilAttachmentOptimal;
			hasDepthStencil = true;
		} else {
			sourceLayout = vk::ImageLayout::eColorAttachmentOptimal;
			// Warning: sourceLayout = vk::ImageLayout::eAttachmentOptimal;
		}

		// Enforce no transition to input only symbols
		targetLayout = sourceLayout;

		/**/

		result->_attachments.push_back(
			vk::AttachmentDescription {
				{},
				gfx::api::vkTranslateFormat(sourceFormat),
				vk::SampleCountFlagBits::e1,
				loadOp,
				vk::AttachmentStoreOp::eDontCare,
				isStencilFormat(sourceFormat) ? loadOp : vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				sourceLayout,
				targetLayout
			}
		);

		result->_references.emplace_back(
			attachIndex++,
			targetLayout
		);
	}

	/**/

	Vector<vk::AttachmentReference> colorRefs {};
	Vector<vk::AttachmentReference> depthStencilRefs {};

	for (const auto& ref : result->_references) {
		switch (ref.layout) {
			case vk::ImageLayout::eDepthAttachmentOptimal:
			case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:
			case vk::ImageLayout::eDepthReadOnlyOptimal:
			case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
			case vk::ImageLayout::eStencilAttachmentOptimal:
			case vk::ImageLayout::eStencilReadOnlyOptimal: {
				depthStencilRefs.push_back(ref);
				break;
			}
			default: {
				colorRefs.push_back(ref);
			}
		}
	}

	/**/

	vk::SubpassDescription subpass {
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		0uL, nullptr,
		static_cast<u32>(colorRefs.size()), colorRefs.data(),
		nullptr, hasDepthStencil ? depthStencilRefs.data() : nullptr,
		0uL, nullptr
	};

	/**/

	if (result->_dependencies.empty()) {

		const auto hasColorRef = subpass.colorAttachmentCount > 0;
		const auto hasDepthStencilRef = subpass.pDepthStencilAttachment != nullptr;

		if (hasColorRef) {

			result->_dependencies.emplace_back(
				VK_SUBPASS_EXTERNAL,
				0,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eMemoryRead,
				vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
				vk::DependencyFlagBits::eByRegion
			);

			result->_dependencies.push_back(
				vk::SubpassDependency {
					0,
					VK_SUBPASS_EXTERNAL,
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::PipelineStageFlagBits::eTopOfPipe,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlagBits::eMemoryRead,
					vk::DependencyFlagBits::eByRegion
				}
			);
		}

		if (hasDepthStencilRef) {

			const auto isReadOnly = result->_attachments[depthStencilRefs.front().attachment].storeOp !=
				vk::AttachmentStoreOp::eStore;

			result->_dependencies.push_back(
				vk::SubpassDependency {
					VK_SUBPASS_EXTERNAL,
					0,
					vk::PipelineStageFlagBits::eFragmentShader,
					vk::PipelineStageFlagBits::eEarlyFragmentTests,
					vk::AccessFlagBits::eShaderRead,
					isReadOnly ?
					vk::AccessFlagBits::eDepthStencilAttachmentRead :
					vk::AccessFlagBits::eDepthStencilAttachmentRead |
					vk::AccessFlagBits::eDepthStencilAttachmentWrite,
					vk::DependencyFlagBits::eByRegion
				}
			);

			result->_dependencies.push_back(
				vk::SubpassDependency {
					0,
					VK_SUBPASS_EXTERNAL,
					vk::PipelineStageFlagBits::eLateFragmentTests,
					vk::PipelineStageFlagBits::eFragmentShader,
					isReadOnly ?
					vk::AccessFlagBits::eDepthStencilAttachmentRead :
					vk::AccessFlagBits::eDepthStencilAttachmentRead |
					vk::AccessFlagBits::eDepthStencilAttachmentWrite,
					vk::AccessFlagBits::eShaderRead,
					vk::DependencyFlagBits::eByRegion
				}
			);

		}

	}

	if (not subpassDependencies_.empty()) {
		result->_dependencies.append_range(subpassDependencies_);
	}

	/**/

	vk::RenderPassCreateInfo rpci {
		vk::RenderPassCreateFlags(),
		static_cast<u32>(result->_attachments.size()),
		result->_attachments.data(),
		1uL,
		&subpass,
		static_cast<u32>(result->_dependencies.size()),
		result->_dependencies.data()
	};

	/**/

	if (viewMask_ != 0uL || correlationMask_ != 0uL) {
		result->_viewMasks.emplace_back(viewMask_);
		result->_correlationMasks.emplace_back(correlationMask_);
	}

	vk::RenderPassMultiviewCreateInfo mvi {};
	if (not result->_viewMasks.empty()) {

		mvi.subpassCount = result->_viewMasks.size();
		mvi.pViewMasks = result->_viewMasks.data();

		mvi.dependencyCount = 0;
		mvi.pViewOffsets = nullptr;

		mvi.correlationMaskCount = result->_correlationMasks.size();
		mvi.pCorrelationMasks = result->_correlationMasks.data();

		rpci.pNext = &mvi;
	}

	/**/

	// Warning: Replace

	const auto& device = Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice();
	result->_vkGraphicsPass = reinterpret_cast<_::VkGraphicsRenderPass>(
		device.createRenderPass(rpci).operator VkRenderPass()
	);

	/**/

	return result;
}

tl::expected<smr<const RaytracingPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildRaytracingPass(
	mref<smr<const RaytracingPipeline>> pipeline_
) const noexcept {
	return tl::make_unexpected(AccelerationPassFactoryError {});
}

tl::expected<smr<const RaytracingPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildRaytracingPass(
	cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
	cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_
) const noexcept {
	return tl::make_unexpected(AccelerationPassFactoryError {});
}

tl::expected<smr<const MeshPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildMeshPass(
	mref<smr<const MeshPipeline>> pipeline_
) const noexcept {
	return tl::make_unexpected(AccelerationPassFactoryError {});
}

tl::expected<smr<const MeshPass>, AccelerationPassFactoryError> VkAccelerationPassFactory::buildMeshPass(
	cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
	cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_
) const noexcept {
	return tl::make_unexpected(AccelerationPassFactoryError {});
}
