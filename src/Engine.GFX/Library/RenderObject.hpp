#pragma once

#include "Inc.hpp"

#include <list>
#include <mutex>
#include <shared_mutex>
#include "Model.hpp"
#include "Material.hpp"
#include "Uniform/UniformPart.hpp"

namespace clockwork::__internal::gfx::types
{
	/**
	 * Note that function calls to this handler class are heavy cause
	 *	of mutal exclusion to protect graphics from inconsistent data.
	 *	Nearly every call will be blocking
	 */
	class RenderObject
	{
	public:
		RenderObject(const __Model& model_) :
			_model(model_),
			_partsAsBlock(0)
		{
			preparePartPartList();
		}

		__Model model() const
		{
			return _model;
		}

		/**
		 * Returns all UniformParts as a block of data
		 *	Costs a lot performance cause of vector*list back insert
		 *	
		 *	Algorithm:
		 *		foreach Lod(i) of Model.Lods
		 *			foreach Part of Lod(i).Parts
		 *				push data
		 *			end
		 *		end
		 *	
		 *	Result:
		 *		n : Count of Lods
		 *	
		 *		[0] => Lod(0) Part(0) ...
		 *		[1] => Lod(1) Part(0) ...
		 *		[n-1] => Lod(n-1) Part(0) ...
		 *		[n] => Lod(0) Part(1) ...
		 *		[n+1] => Lod(1) Part(1) ...
		 *		[2n] => Lod(n) Part(1) ...
		 */
		UniformPart* parts()
		{
			_partsAsBlock = std::vector<UniformPart>(0);

			for (uint8_t i = 0; i < _partsByParts.size(); i++)
				_partsAsBlock.insert(_partsAsBlock.end(), _partsByParts[i].begin(), _partsByParts[i].end());
			return _partsAsBlock.data();
		}

		/**
		 * Does not guarantee for correctness
		 */
		gfx::Instance* instances()
		{
			return std::vector<Instance>(_instances.begin(), _instances.end()).data();
		}

		size_t instanceCount()
		{
			std::shared_lock<std::shared_mutex> lock(_mtx);
			return _instances.size();
		}

		size_t partCount()
		{
			std::shared_lock<std::shared_mutex> lock(_mtx);
			size_t count = 0;
			for (const std::list<UniformPart>& list : _partsByParts)
				count += list.size();
			return count;
		}

		size_t partCount(const size_t lodIndex_, const size_t partIndex_)
		{
			std::shared_lock<std::shared_mutex> lock(_mtx);
			const size_t lodCount = _model->lods.size();
			return _partsByParts[partIndex_ * lodCount + lodIndex_].size();
		}

		size_t partLodCount()
		{
			std::shared_lock<std::shared_mutex> lock(_mtx);
			return _partsByParts.size();
		}

		gfx::Instance createInstance(const vec3& pos_, const vec3& rotation_, const float scale_)
		{
			Instance instance = Instance();

			instance.lod = 0;
			instance.position = pos_;
			instance.rotation = rotation_;
			instance.scale = scale_;

			const uint8_t mpc = UINT8_T(_model->getLod(instance.lod).parts.size());
			std::vector<UniformPart> parts = std::vector<UniformPart>(mpc);

			for (uint8_t i = 0; i < mpc; i ++)
			{
				const ModelPart& mp = _model->getLod(instance.lod).parts[i];
				const UniformPart part = {
					pos_,
					rotation_,
					scale_,
					UINT32_T(_materialSet[i]->materialReference()->indexedOffset())
				};

				parts[i] = part;
			}

			addInstance(instance, parts);

			return instance;
		}

		void storeMaterialSet(__Material* materials_, const size_t count_)
		{
			_materialSet = std::vector<__Material>(count_);
			for (size_t i = 0; i < count_; i ++)
				_materialSet[i] = materials_[i];
		}

		bool ready()
		{
			if (_model == nullptr)
				return false;

			if (_materialSet.empty())
				return false;

			if (partCount() <= 0)
				return false;

			bool rdy = _model->ready();

			for (const __Material& entry : _materialSet)
			{
				rdy = entry->isLoaded();
				if (!rdy) break;
			}

			return rdy;
		}

	private:
		__Model _model;

		std::shared_mutex _mtx;

		std::list<Instance> _instances;
		std::vector<std::list<UniformPart>> _partsByParts;

		// temporary
		std::vector<UniformPart> _partsAsBlock;

		std::vector<__Material> _materialSet;

		/**
		 * Adds a instance to list and inserts parts into right positions
		 */
		void addInstance(gfx::Instance& instance_, const std::vector<UniformPart>& parts_)
		{
			std::unique_lock<std::shared_mutex> lock(_mtx);

			const uint8_t& lod = instance_.lod;
			const size_t lodCount = _model->lods.size();

			instance_.parts.resize(parts_.size());

			for (size_t i = 0; i < parts_.size(); i ++)
			{
				std::list<UniformPart>& list = _partsByParts[i * lodCount + lod];
				auto result = list.insert(list.end(), parts_[i]);

				instance_.parts[i] = &result._Ptr->_Myval;
			}

			_instances.insert(_instances.end(), instance_);
		}

		/**
		 * Remove instance from instance list and removes parts from part storage
		 */
		void deleteInstance(const Instance& instance_)
		{
			std::unique_lock<std::shared_mutex> lock(_mtx);

			_instances.remove(instance_);

			for (auto& entry : instance_.parts)
				for (auto& list : _partsByParts)
					list.remove(*entry);
		}

		/**
		 * Prepare _partsByParts list for subsorted lists
		 */
		void preparePartPartList()
		{
			_partsByParts = std::vector<std::list<UniformPart>>(
				_model->lods.size() * _model->lods[0].parts.size(),
				std::list<UniformPart>(0)
			);
		}
	};

	TYPE_SHARED(RenderObject, RenderObject);
}
