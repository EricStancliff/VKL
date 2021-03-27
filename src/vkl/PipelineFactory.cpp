#include <vkl/PipelineFactory.h>
#include <iostream>
#include <vkl/Pipeline.h>

namespace vkl
{
	PipelineMetaFactory& PipelineMetaFactory::instance()
	{
		static PipelineMetaFactory factory;
		return factory;
	}

	std::span<const std::pair<std::type_index, std::shared_ptr<const PipelineDescription>>> PipelineMetaFactory::allDescriptions() const
	{
		return _descriptions;
	}
	
	bool PipelineMetaFactory::registerPipeline(std::type_index type, std::shared_ptr<const PipelineDescription> description)
	{
		auto find = std::lower_bound(_descriptions.begin(), _descriptions.end(), type, [&](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs;
			});
		if (find != _descriptions.end() && find->first == type)
		{
			assert(false);
			std::cerr << "Pipeline already registered";
			return false;
		}
		_descriptions.emplace(find, std::move(std::make_pair(type, std::move(description))));
		return true;
	}
	
	std::shared_ptr<const PipelineDescription> PipelineMetaFactory::description(std::type_index type)
	{
		auto find = std::lower_bound(_descriptions.begin(), _descriptions.end(), type, [&](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs;
			});
		if (find != _descriptions.end() && find->first == type)
		{
			return find->second;
		}
		return nullptr;
	}
	
	/*****************************************************************************************************************/
	PipelineManager::PipelineManager(const Device& device, const SwapChain& swapChain, const RenderPass& renderPass)
	{
		auto descriptions = PipelineMetaFactory::instance().allDescriptions();
		for(auto description : descriptions)
		{
			const auto& pipelineDesc = description.second;
			std::type_index typeIndex = description.first;

			auto findWhere = std::lower_bound(_pipelines.begin(), _pipelines.end(), typeIndex, [&](const Pipeline& pipePair, const std::type_index& index) {
				return pipePair.type() < index;
				});

			Pipeline pipeline(device, swapChain, *pipelineDesc, renderPass, typeIndex);
			_pipelines.emplace_back(std::move(pipeline));

			}

	}
	const Pipeline* PipelineManager::pipelineForType(std::type_index type) const
	{
		auto findWhere = std::lower_bound(_pipelines.begin(), _pipelines.end(), type, [&](const Pipeline& pipePair, const std::type_index& index) {
			return pipePair.type() < index;
			});
		if (findWhere == _pipelines.end())
			return nullptr;
		return &(*findWhere);
	}
	void PipelineManager::cleanUp(const Device& device)
	{
		for (auto&& pipeline : _pipelines)
			pipeline.cleanUp(device);
		_pipelines.clear();
	}

}