#include "VkDescriptorWriter.hpp"

#include <utility>
#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/SparseBuffer.hpp>
#include <Engine.GFX/Buffer/SparseBufferView.hpp>
#include <Engine.GFX/Buffer/StorageBufferView.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Texture/SparseTexture.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/TextureLikeObject.hpp>
#include <Engine.GFX/Texture/TextureSampler.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

using namespace hg::engine::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

VkDescriptorWriter::VkDescriptorWriter(cref<vk::DescriptorSet> vkSet_) :
	_vkSet(vkSet_),
	_addressPatched(false),
	_descriptorBufferInfos(),
	_descriptorImageInfos(),
	_descriptorSamplerInfos(),
	_writes() {}

ptrdiff_t VkDescriptorWriter::emplaceBufferInfo(mref<vk::DescriptorBufferInfo> info_) {
	_descriptorBufferInfos.emplace_back(std::move(info_));
	return _descriptorBufferInfos.size() - 1uLL;
}

ptrdiff_t VkDescriptorWriter::emplaceImageInfo(mref<vk::DescriptorImageInfo> info_) {
	_descriptorImageInfos.emplace_back(std::move(info_));
	return _descriptorImageInfos.size() - 1uLL;
}

ptrdiff_t VkDescriptorWriter::emplaceSamplerInfo(mref<vk::DescriptorImageInfo> info_) {
	_descriptorSamplerInfos.emplace_back(std::move(info_));
	return _descriptorSamplerInfos.size() - 1uLL;
}

void VkDescriptorWriter::patchInfoAddress() {

	for (auto& write : _writes) {

		switch (write.descriptorType) {
			case vk::DescriptorType::eInlineUniformBlock:
			case vk::DescriptorType::eStorageBuffer:
			case vk::DescriptorType::eUniformBuffer: {
				write.pBufferInfo = _descriptorBufferInfos.data() + reinterpret_cast<ptrdiff_t>(write.pBufferInfo);
				break;
			}
			case vk::DescriptorType::eSampledImage: {
				write.pImageInfo = _descriptorImageInfos.data() + reinterpret_cast<ptrdiff_t>(write.pImageInfo);
				break;
			}
			case vk::DescriptorType::eSampler: {
				write.pImageInfo = _descriptorSamplerInfos.data() + reinterpret_cast<ptrdiff_t>(write.pImageInfo);
				break;
			}
			default: {
				std::unreachable();
			}
		}
	}

	_addressPatched = true;
}

void VkDescriptorWriter::update(cref<vk::Device> device_) {

	if (not _addressPatched) {
		patchInfoAddress();
	}

	device_.updateDescriptorSets(
		static_cast<u32>(_writes.size()),
		_writes.data(),
		0,
		nullptr
	);
}

void VkDescriptorWriter::reset(cref<vk::DescriptorSet> nextVkSet_) {

	_vkSet = nextVkSet_;

	_addressPatched = false;
	_descriptorBufferInfos.clear();
	_descriptorImageInfos.clear();
	_descriptorSamplerInfos.clear();
	_writes.clear();
}

void VkDescriptorWriter::storeUniform(index_type idx_, cref<Buffer> buffer_) {

	const auto marker = emplaceBufferInfo(
		{
			buffer_.buffer,
			0uLL,
			buffer_.size
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		nullptr,
		reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
		nullptr
	);
}

void VkDescriptorWriter::storeUniform(index_type idx_, cref<SparseBuffer> buffer_) {

	const auto marker = emplaceBufferInfo(
		{
			buffer_.vkBuffer(),
			0uLL,
			buffer_.size()
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		nullptr,
		reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
		nullptr

	);
}

void VkDescriptorWriter::storeUniform(index_type idx_, cref<SparseBufferView> view_) {

	const auto marker = emplaceBufferInfo(
		{
			view_.owner()->vkBuffer(),
			view_.offset(),
			view_.size()
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		nullptr,
		reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
		nullptr

	);
}

void VkDescriptorWriter::storeUniform(index_type idx_, nmpt<const gfx::UniformBufferView> view_) {

	const auto obj = view_->object();
	auto write = switchType(
		obj.get(),
		[this, idx_](const ptr<const Buffer> buffer_) {

			const auto marker = emplaceBufferInfo(
				{
					buffer_->buffer,
					0uLL,
					buffer_->size
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
				nullptr
			};
		},
		[this, idx_](const ptr<const SparseBuffer> buffer_) {

			const auto marker = emplaceBufferInfo(
				{
					buffer_->vkBuffer(),
					0uL,
					buffer_->size()
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
				nullptr
			};
		},
		[this, idx_](const ptr<const SparseBufferView> view_) {

			const auto marker = emplaceBufferInfo(
				{
					view_->owner()->vkBuffer(),
					view_->offset(),
					view_->size()
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
				nullptr
			};
		}
	);

	_writes.emplace_back(write);
}

void VkDescriptorWriter::storeStorage(index_type idx_, cref<Buffer> buffer_) {

	const auto marker = emplaceBufferInfo(
		{
			buffer_.buffer,
			0uLL,
			buffer_.size
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eStorageBuffer,
		nullptr,
		reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
		nullptr
	);
}

void VkDescriptorWriter::storeStorage(index_type idx_, cref<SparseBuffer> buffer_) {

	const auto marker = emplaceBufferInfo(
		{
			buffer_.vkBuffer(),
			0uL,
			buffer_.size()
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eStorageBuffer,
		nullptr,
		reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
		nullptr
	);
}

void VkDescriptorWriter::storeStorage(index_type idx_, cref<SparseBufferView> view_) {

	const auto marker = emplaceBufferInfo(
		{
			view_.owner()->vkBuffer(),
			view_.offset(),
			view_.size()
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eStorageBuffer,
		nullptr,
		reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
		nullptr
	);
}

void VkDescriptorWriter::storeStorage(index_type idx_, nmpt<const gfx::StorageBufferView> view_) {

	const auto obj = view_->object();
	auto write = switchType(
		obj.get(),
		[this, idx_](const ptr<const Buffer> buffer_) {

			const auto marker = emplaceBufferInfo(
				{
					buffer_->buffer,
					0uLL,
					buffer_->size
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
				nullptr
			};
		},
		[this, idx_](const ptr<const SparseBuffer> buffer_) {

			const auto marker = emplaceBufferInfo(
				{
					buffer_->vkBuffer(),
					0uL,
					buffer_->size()
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
				nullptr
			};
		},
		[this, idx_](const ptr<const SparseBufferView> view_) {

			const auto marker = emplaceBufferInfo(
				{
					view_->owner()->vkBuffer(),
					view_->offset(),
					view_->size()
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				reinterpret_cast<ptr<vk::DescriptorBufferInfo>>(marker),
				nullptr
			};
		}
	);

	_writes.emplace_back(write);
}

void VkDescriptorWriter::storeTexture(index_type idx_, nmpt<const gfx::TextureLikeObject> texture_) {

	auto write = switchType(
		texture_.get(),
		[this, idx_](const ptr<const Texture> texture_) {

			const auto marker = emplaceImageInfo(
				{
					nullptr,
					texture_->vkView(),
					// Warning: Unsecure / Error prone
					texture_->buffer()._vkLayout
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eSampledImage,
				reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
				nullptr,
				nullptr
			};
		},
		[this, idx_](const ptr<const TextureView> texture_) {

			const auto marker = emplaceImageInfo(
				{
					nullptr,
					texture_->_vkImageView != nullptr ?
						reinterpret_cast<VkImageView>(texture_->_vkImageView) :
						texture_->owner()->vkView().operator VkImageView(),
					// Warning: Unsecure / Error prone
					texture_->owner()->buffer()._vkLayout
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eSampledImage,
				reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
				nullptr,
				nullptr
			};
		},
		[this, idx_](const ptr<const SparseTexture> texture_) {

			const auto marker = emplaceImageInfo(
				{
					nullptr,
					texture_->_vkImageView,
					// Warning: Unsecure / Error prone
					vk::ImageLayout::eUndefined
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eSampledImage,
				reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
				nullptr,
				nullptr
			};
		},
		[this, idx_](const ptr<const SparseTextureView> texture_) {

			const auto marker = emplaceImageInfo(
				{
					nullptr,
					reinterpret_cast<::VkImageView>(texture_->vkImageView()),
					// Warning: Unsecure / Error prone
					vk::ImageLayout::eShaderReadOnlyOptimal
				}
			);

			return vk::WriteDescriptorSet {
				_vkSet,
				idx_,
				0uL,
				1uL,
				vk::DescriptorType::eSampledImage,
				reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
				nullptr,
				nullptr
			};
		}
	);

	_writes.emplace_back(write);
}

void VkDescriptorWriter::storeTextureSampler(index_type idx_, nmpt<const gfx::TextureSampler> sampler_) {

	const auto marker = emplaceSamplerInfo(
		{
			sampler_->vkSampler(),
			nullptr,
			vk::ImageLayout {}
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0uL,
		1uL,
		vk::DescriptorType::eSampler,
		reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
		nullptr,
		nullptr
	);
}

void VkDescriptorWriter::store(index_type idx_, cref<Texture> texture_, cref<vk::Sampler> sampler_) {
	storeAs(idx_, texture_, sampler_, texture_.buffer()._vkLayout);
}

void VkDescriptorWriter::storeAs(
	index_type idx_,
	cref<Texture> texture_,
	cref<vk::Sampler> sampler_,
	cref<vk::ImageLayout> layout_
) {

	const auto marker = emplaceImageInfo({ sampler_, texture_.vkView(), layout_ });

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
		reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
		nullptr,
		nullptr
	);
}

void VkDescriptorWriter::store(index_type idx_, const ptr<const SparseTexture> texture_, cref<vk::Sampler> sampler_) {
	storeAs(idx_, texture_, sampler_, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void VkDescriptorWriter::storeAs(
	index_type idx_,
	const ptr<const SparseTexture> texture_,
	cref<vk::Sampler> sampler_,
	cref<vk::ImageLayout> layout_
) {

	const auto marker = emplaceImageInfo({ sampler_, texture_->_vkImageView, layout_ });

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
		reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
		nullptr,
		nullptr
	);
}

void VkDescriptorWriter::store(
	index_type idx_,
	const ptr<const SparseTextureView> texture_,
	cref<vk::Sampler> sampler_
) {
	const auto marker = emplaceImageInfo(
		{
			sampler_,
			reinterpret_cast<VkImageView>(texture_->vkImageView()),
			vk::ImageLayout::eShaderReadOnlyOptimal
		}
	);

	_writes.emplace_back(
		_vkSet,
		idx_,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
		reinterpret_cast<ptr<vk::DescriptorImageInfo>>(marker),
		nullptr,
		nullptr
	);
}
