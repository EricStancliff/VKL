#include <vkl/Common.h>

#include <vkl/Instance.h>
#include <vkl/Device.h>
#include <vkl/SwapChain.h>
#include <vkl/Window.h>
#include <vkl/Surface.h>
#include <vkl/RenderObject.h>
#include <vkl/BufferManager.h>
#include <vkl/Pipeline.h>
#include <vkl/RenderPass.h>
#include <vkl/CommandDispatcher.h>
#include <vkl/VertexBuffer.h>
#include <vkl/DrawCall.h>
#include <vkl/IndexBuffer.h>

#include <vxt/PNGLoader.h>
#include <vxt/LinearAlgebra.h>

constexpr const char* VertShader = R"Shader(

#version 450


layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragUV = inUV;
}
)Shader";
constexpr const char* FragShader = R"Shader(

#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragUV);
}
)Shader";

struct Vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
};

class ImagePlane : public vkl::RenderObject
{
	REFLECTED_TYPE(ImagePlane, vkl::RenderObject)
	static void populateReflection(vkl::RenderObjectDescription& reflection)
	{
		reflection.pipelineDescription().setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		reflection.pipelineDescription().declareVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(Vertex), offsetof(Vertex, pos));
		reflection.pipelineDescription().declareVertexAttribute(0, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(Vertex), offsetof(Vertex, uv));

		reflection.pipelineDescription().declareTexture(1);
	}

public:
	void init(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines) override
	{
		vkl::RenderObject::init(device, swapChain, bufferManager, pipelines);

		auto vbo = bufferManager.createVertexBuffer(device, swapChain);

		_verts.push_back({ glm::vec2(-0.5, -0.5), glm::vec2(0,0) });
		_verts.push_back({ glm::vec2(-0.5, 0.5), glm::vec2(0,1) });
		_verts.push_back({ glm::vec2(0.5, 0.5), glm::vec2(1,1) });
		_verts.push_back({ glm::vec2(0.5, -0.5), glm::vec2(1,0) });

		vbo->setData(_verts.data(), sizeof(Vertex), _verts.size());
		addVBO(device, swapChain, vbo, 0);

		auto drawCall = std::make_shared<vkl::DrawCall>();

		_indices.push_back(0);
		_indices.push_back(1);
		_indices.push_back(3);
		_indices.push_back(3);
		_indices.push_back(1);
		_indices.push_back(2);
		auto indexBuffer = bufferManager.createIndexBuffer(device, swapChain);
		indexBuffer->setData(_indices);
		drawCall->setCount(_indices.size());

		drawCall->setIndexBuffer(indexBuffer);

		addDrawCall(device, swapChain, drawCall);

		_imageData = vxt::loadJPGData((std::filesystem::path(vkl::vklDataDir()) / "textures" / "texture.jpg").make_preferred().string().c_str(), _width, _height, _components);

		if (_imageData)
		{
			auto texBuff = bufferManager.createTextureBuffer(device, swapChain, _imageData, (size_t)_width, (size_t)_height, (size_t)_components);
			addTexture(device, swapChain, texBuff, 1);
		}
		else
		{
			std::cerr << "Couldn't load example image!!!";
		}
	}
	~ImagePlane()
	{
		if(_imageData)
			vxt::freeJPGData(_imageData);
	}
private:
	std::vector<Vertex> _verts;
	std::vector<uint32_t> _indices;
	int _width{ 0 }, _height{ 0 }, _components{ 0 };
	void* _imageData{ nullptr };
};

IMPL_REFLECTION(ImagePlane)

int main(int argc, char* argv[])
{
	vkl::Instance instance("triangle_vkl", true);
	vkl::Window window(1080, 720, "triangle_vkl");
	vkl::Surface surface(instance, window);
	vkl::Device device(instance, surface);

	vkl::SwapChainOptions swapChainOptions{};
	swapChainOptions.swapChainExtent.width = window.getWindowSize().width;
	swapChainOptions.swapChainExtent.height = window.getWindowSize().height;
	vkl::SwapChain swapChain(device, surface, swapChainOptions);

	vkl::RenderPassOptions mainPassOptions;
	mainPassOptions.clearColor = { 0.f, 0.f, 0.f, 1.f };
	vkl::RenderPass mainPass(device, swapChain, mainPassOptions);

	swapChain.registerRenderPass(device, mainPass);

	vkl::BufferManager bufferManager(device, swapChain);
	vkl::PipelineManager pipelineManager(device, swapChain, mainPass);

	vkl::CommandDispatcher commandDispatcher(device, swapChain);

	std::vector<std::shared_ptr<vkl::RenderObject>> renderObjects;
	auto imagePlane = std::make_shared<ImagePlane>();
	imagePlane->init(device, swapChain, bufferManager, pipelineManager);
	renderObjects.push_back(imagePlane);

	while (!window.shouldClose())
	{
		swapChain.prepNextFrame(device, surface, commandDispatcher, mainPass, window.getWindowSize());
		bufferManager.update(device, swapChain);
		commandDispatcher.processUnsortedObjects(renderObjects, pipelineManager, mainPass, swapChain, swapChain.frameBuffer(swapChain.frame()), swapChain.swapChainExtent());
		swapChain.swap(device, surface, commandDispatcher, mainPass, window.getWindowSize());
		window.clearLastFrame();
		vkl::Window::pollEventsForAllWindows();
		window.updateToThisFrame();
	}

	device.waitIdle();
	renderObjects.clear();
	commandDispatcher.cleanUp(device);
	pipelineManager.cleanUp(device);
	bufferManager.cleanUp(device);
	mainPass.cleanUp(device);
	swapChain.cleanUp(device);
	device.cleanUp();
	surface.cleanUp(instance);
	window.cleanUp();

	instance.cleanUp();
	vkl::Window::cleanUpWindowSystem();

}