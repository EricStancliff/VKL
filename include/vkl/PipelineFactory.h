#pragma once
#include <vkl/Common.h>
#include <typeindex>
#include <memory>
#include <cassert>

namespace vkl
{

	class VKL_EXPORT PipelineMetaFactory
	{
	public:
		~PipelineMetaFactory() = default;
		PipelineMetaFactory(const PipelineMetaFactory&) = delete;
		PipelineMetaFactory(PipelineMetaFactory&&) noexcept = default;
		PipelineMetaFactory& operator=(PipelineMetaFactory&&) noexcept = default;
		PipelineMetaFactory& operator=(const PipelineMetaFactory&) = delete;

		static PipelineMetaFactory& instance();

		template <typename T, typename F>
		bool registerPipeline(F func)
		{
			auto typeIndex = std::type_index(typeid(T));
			auto description = std::make_shared<PipelineDescription>();
			func(*description);
			return registerPipeline(typeIndex, std::move(description));
		}

		template <typename T>
		std::shared_ptr<const PipelineDescription> description() const
		{
			return description(std::type_index(typeid(T)));
		}

		std::span<const std::pair<std::type_index, std::shared_ptr<const PipelineDescription>>> allDescriptions() const;

		bool registerPipeline(std::type_index type, std::shared_ptr<const PipelineDescription> description);
		std::shared_ptr<const PipelineDescription> description(std::type_index type);

	protected:
		PipelineMetaFactory() = default;

		std::vector<std::pair<std::type_index, std::shared_ptr<const PipelineDescription>>> _descriptions;
	};
	
	/************************************************************************************************************/

	class VKL_EXPORT PipelineManager
	{
	public:
		PipelineManager() = delete;
		~PipelineManager() = default;
		PipelineManager(const PipelineManager&) = delete;
		PipelineManager(PipelineManager&&) noexcept = default;
		PipelineManager& operator=(PipelineManager&&) noexcept = default;
		PipelineManager& operator=(const PipelineManager&) = delete;

		PipelineManager(const Device& device, const SwapChain& swapChain, const RenderPass& renderPass);

		const Pipeline* pipelineForType(std::type_index type) const;

		void cleanUp(const Device& device);
	private:
		std::vector<Pipeline> _pipelines;
	};

}

#define PIPELINE_TYPE \
static bool pipeline_initialized();\
static bool _pipeline_initialized;

#define REGISTER_PIPELINE(Type, Func) \
bool Type::pipeline_initialized(){\
	static bool initialized = vkl::PipelineMetaFactory::instance().registerPipeline<Type>(Func);\
	return initialized;\
}\
bool Type::_pipeline_initialized = Type::pipeline_initialized();