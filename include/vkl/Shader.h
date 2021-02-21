#pragma once
#include <memory>
#include <Common.h>

#include <string>
#include <filesystem>

namespace vkl
{

	class ShaderData
	{
	public:
		ShaderData() = default;
		virtual ~ShaderData() = default;

		virtual const char* const data() const = 0;
		virtual size_t dataSize() const = 0;

		virtual VkShaderStageFlagBits stage() const = 0;

		virtual std::string fileName() const = 0;
	};
	/******************************************************************************************/

	class GLSLShader : public ShaderData
	{
	public:
		GLSLShader() = delete;
		GLSLShader(const char* shader, VkShaderStageFlagBits stage);
		GLSLShader(const std::filesystem::path& path, VkShaderStageFlagBits stage);
		virtual ~GLSLShader();

		const char* const data() const override;
		size_t dataSize() const override;

		std::string fileName() const override;

		VkShaderStageFlagBits stage() const override;
	private:

		void init(const char* data, size_t size);

		const char* _data{ nullptr };
		size_t      _size{ 0 };
		std::string _fileName;
		VkShaderStageFlagBits _stage;
		void* _result{ nullptr };
		std::vector<char> _ownedBuffer;
	};
	/******************************************************************************************/

	class SPVShader : public ShaderData
	{
	public:
		SPVShader() = delete;
		SPVShader(const char* shader, VkShaderStageFlagBits stage);
		SPVShader(const std::filesystem::path& path, VkShaderStageFlagBits stage);
		virtual ~SPVShader() = default;

		const char* const data() const override;
		size_t dataSize() const override;

		std::string fileName() const override;
		VkShaderStageFlagBits stage() const override;

	private:
		const char* _data;
		size_t      _size;
		std::string _fileName;
		VkShaderStageFlagBits _stage;
		std::vector<char> _ownedBuffer;
	};
	/******************************************************************************************/


	class ShaderModule
	{
	public:
		ShaderModule() = delete;
		ShaderModule(const Device& device, std::shared_ptr<ShaderData> shader, VkShaderStageFlagBits shaderStage);

		VkShaderModule handle() const;

	private:
		std::shared_ptr<ShaderData> m_shaderData;
		VkShaderModule _shaderModule{ VK_NULL_HANDLE };
	};
}