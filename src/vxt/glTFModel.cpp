/******************

Parts of this code are heavily borrowed and modified from Sascha Willem's Vulkan glTF-PBR library (MIT License)
https://github.com/SaschaWillems/Vulkan-glTF-PBR

********************/


#include <vxt/Model.h>

#include <vkl/Common.h>

#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <iostream>

#include <vkl/DrawCall.h>
#include <vkl/TextureBuffer.h>
#include <vkl/BufferManager.h>
#include <vkl/VertexBuffer.h>
#include <vkl/IndexBuffer.h>

//helpers fro later versions
namespace tinygltf
{

	static inline int32_t GetTypeSizeInBytes(uint32_t ty) {
		if (ty == TINYGLTF_TYPE_SCALAR) {
			return 1;
		}
		else if (ty == TINYGLTF_TYPE_VEC2) {
			return 2;
		}
		else if (ty == TINYGLTF_TYPE_VEC3) {
			return 3;
		}
		else if (ty == TINYGLTF_TYPE_VEC4) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT2) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT3) {
			return 9;
		}
		else if (ty == TINYGLTF_TYPE_MAT4) {
			return 16;
		}
		else {
			// Unknown componenty type
			return -1;
		}
	}

}

namespace vxt
{

	VkSamplerAddressMode getVkWrapMode(int32_t wrapMode)
	{
		switch (wrapMode) {
		case 10497:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case 33071:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case 33648:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		}
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkFilter getVkFilterMode(int32_t filterMode)
	{
		switch (filterMode) {
		case 9728:
			return VK_FILTER_NEAREST;
		case 9729:
			return VK_FILTER_LINEAR;
		case 9984:
			return VK_FILTER_NEAREST;
		case 9985:
			return VK_FILTER_NEAREST;
		case 9986:
			return VK_FILTER_LINEAR;
		case 9987:
			return VK_FILTER_LINEAR;
		}
		return VK_FILTER_LINEAR;
	}

	class glTFModel : public Model
	{
		REFLECTED_TYPE(glTFModel, Model)
		static void populateReflection(reflect::reflection_data_base& reflection) {}
	public:
		glTFModel() = default;
		virtual ~glTFModel() = default;
		glTFModel(const glTFModel&) = delete;
		glTFModel(glTFModel&&) noexcept = default;
		glTFModel& operator=(glTFModel&&) noexcept = default;
		glTFModel& operator=(const glTFModel&) = delete;

		struct NodeTransform
		{
			glm::vec3 trans{ glm::zero<glm::vec3>() };
			glm::quat rot{  };
			glm::vec3 scale{ glm::one<glm::vec3>() };
			glm::mat4 mat{ glm::identity<glm::mat4>() };
		};

	protected:
		void init(const std::string& file, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager) override {

			tinygltf::Model gltfModel;
			tinygltf::TinyGLTF gltfContext;

			std::string error;
			std::string warning;

			bool binary = false;
			size_t extpos = file.rfind('.', file.length());
			if (extpos != std::string::npos) {
				binary = (file.substr(extpos + 1, file.length() - extpos) == "glb");
			}

			bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, file.c_str()) : gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, file.c_str());

			if (!warning.empty())
			{
				std::cout << "Could not load gltf file: " << warning << std::endl;
			}

			if (fileLoaded) {

			}
			else {
				std::cerr << "Could not load gltf file: " << error << std::endl;
				return;
			}

			_nodeParents.resize(gltfModel.nodes.size());
			_nodeTransforms.resize(gltfModel.nodes.size());

			loadTextureSamplers(gltfModel);
			loadTextures(gltfModel, device, swapChain, bufferManager);
			loadMaterials(gltfModel);
			
			for (auto& scene : gltfModel.scenes)
				for (auto& node : scene.nodes)
					loadNode(-1, gltfModel.nodes[node], node, gltfModel);

			_vertexBuffer = bufferManager.createVertexBuffer(device, swapChain);
			_vertexBuffer->setData(_verts.data(), _verts.size(), sizeof(Vertex));

			_indexBuffer = bufferManager.createIndexBuffer(device, swapChain);
			_indexBuffer->setData(_indices);
		
		}

		void loadTextures(tinygltf::Model& gltfModel, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager)
		{
			for (tinygltf::Texture& tex : gltfModel.textures) {
				tinygltf::Image gltfimage = gltfModel.images[tex.source];

				vkl::TextureOptions options{};
				
				if (tex.sampler != -1) {
					options = _texOptions[tex.sampler];
				}

				unsigned char* buffer = nullptr;
				VkDeviceSize bufferSize = 0;
				bool deleteBuffer = false;
				if (gltfimage.component == 3) {
					// Most devices don't support RGB only on Vulkan so convert if necessary
					// TODO: Check actual format support and transform only if required
					bufferSize = gltfimage.width * gltfimage.height * 4;
					buffer = new unsigned char[bufferSize];
					unsigned char* rgba = buffer;
					unsigned char* rgb = &gltfimage.image[0];
					for (int32_t i = 0; i < gltfimage.width * gltfimage.height; ++i) {
						for (int32_t j = 0; j < 3; ++j) {
							rgba[j] = rgb[j];
						}
						rgba += 4;
						rgb += 3;
					}
					deleteBuffer = true;
				}
				else {
					buffer = &gltfimage.image[0];
					bufferSize = gltfimage.image.size();
				}

				//creates static texture in GPU local memory, so no need to preserve the buffer above
				_textureBuffers.emplace_back(std::move(bufferManager.createTextureBuffer(device, swapChain, buffer, gltfimage.width, gltfimage.height, 4)));

				if (deleteBuffer)
				{
					delete[] buffer;
				}
			}
		}

		void loadTextureSamplers(tinygltf::Model& gltfModel)
		{
			for (tinygltf::Sampler smpl : gltfModel.samplers) {
				vkl::TextureOptions options{};
				options.minFilter = getVkFilterMode(smpl.minFilter);
				options.magFilter = getVkFilterMode(smpl.magFilter);
				options.addressModeU = getVkWrapMode(smpl.wrapS);
				options.addressModeV = getVkWrapMode(smpl.wrapT);
				options.addressModeW = options.addressModeV;
				_texOptions.push_back(options);
			}
		}


		void loadMaterials(tinygltf::Model& gltfModel)
		{
			for (tinygltf::Material& mat : gltfModel.materials) {
				Model::Material material;
				if (mat.values.find("baseColorTexture") != mat.values.end()) {
					material.baseColorTexture = _textureBuffers[mat.values["baseColorTexture"].TextureIndex()];
					material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
				}
				if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
					material.metallicRoughnessTexture = _textureBuffers[mat.values["metallicRoughnessTexture"].TextureIndex()];
					material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
				}
				if (mat.values.find("roughnessFactor") != mat.values.end()) {
					material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
				}
				if (mat.values.find("metallicFactor") != mat.values.end()) {
					material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
				}
				if (mat.values.find("baseColorFactor") != mat.values.end()) {
					material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
				}
				if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
					material.normalTexture = _textureBuffers[mat.additionalValues["normalTexture"].TextureIndex()];
					material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
				}
				if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
					material.emissiveTexture = _textureBuffers[mat.additionalValues["emissiveTexture"].TextureIndex()];
					material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
				}
				if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
					material.occlusionTexture = _textureBuffers[mat.additionalValues["occlusionTexture"].TextureIndex()];
					material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
				}
				if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
					tinygltf::Parameter param = mat.additionalValues["alphaMode"];
					if (param.string_value == "BLEND") {
						material.alphaMode = Material::ALPHAMODE_BLEND;
					}
					if (param.string_value == "MASK") {
						material.alphaCutoff = 0.5f;
						material.alphaMode = Material::ALPHAMODE_MASK;
					}
				}
				if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
					material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
				}
				if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
					material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
					material.emissiveFactor = glm::vec4(0.0f);
				}

				// Extensions
				// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
				if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
					auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
					if (ext->second.Has("specularGlossinessTexture")) {
						auto index = ext->second.Get("specularGlossinessTexture").Get("index");
						material.extension.specularGlossinessTexture = _textureBuffers[index.Get<int>()];
						auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
						material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
						material.pbrWorkflows.specularGlossiness = true;
					}
					if (ext->second.Has("diffuseTexture")) {
						auto index = ext->second.Get("diffuseTexture").Get("index");
						material.extension.diffuseTexture = _textureBuffers[index.Get<int>()];
					}
					if (ext->second.Has("diffuseFactor")) {
						auto factor = ext->second.Get("diffuseFactor");
						for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
							auto val = factor.Get(i);
							material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
						}
					}
					if (ext->second.Has("specularFactor")) {
						auto factor = ext->second.Get("specularFactor");
						for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
							auto val = factor.Get(i);
							material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
						}
					}
				}

				_materials.emplace_back(std::move(material));
			}
		}


		void loadNode(int parent, const tinygltf::Node& node, int nodeIndex, const tinygltf::Model& model)
		{

			_nodeParents[nodeIndex] = parent;

			// Generate local node matrix
			glm::vec3 translation = glm::vec3(0.0f);
			if (node.translation.size() == 3) {
				translation = glm::make_vec3(node.translation.data());
				_nodeTransforms[nodeIndex].trans = translation;
			}
			glm::mat4 rotation = glm::mat4(1.0f);
			if (node.rotation.size() == 4) {
				glm::quat q = glm::make_quat(node.rotation.data());
				_nodeTransforms[nodeIndex].rot = glm::mat4(q);
			}
			glm::vec3 scale = glm::vec3(1.0f);
			if (node.scale.size() == 3) {
				scale = glm::make_vec3(node.scale.data());
				_nodeTransforms[nodeIndex].scale = scale;
			}
			if (node.matrix.size() == 16) {
				_nodeTransforms[nodeIndex].mat = glm::make_mat4x4(node.matrix.data());
			};

			// Node with children
			if (node.children.size() > 0) {
				for (size_t i = 0; i < node.children.size(); i++) {
					loadNode(nodeIndex, model.nodes[node.children[i]], node.children[i], model);
				}
			}

			// Node contains mesh data
			if (node.mesh > -1) {
				const tinygltf::Mesh mesh = model.meshes[node.mesh];
				for (size_t j = 0; j < mesh.primitives.size(); j++) {
					const tinygltf::Primitive& primitive = mesh.primitives[j];
					uint32_t indexStart = static_cast<uint32_t>(_indices.size());
					uint32_t vertexStart = static_cast<uint32_t>(_verts.size());
					uint32_t indexCount = 0;
					uint32_t vertexCount = 0;
					glm::vec3 posMin{};
					glm::vec3 posMax{};
					bool hasSkin = false;
					bool hasIndices = primitive.indices > -1;
					// Vertices
					{
						const float* bufferPos = nullptr;
						const float* bufferNormals = nullptr;
						const float* bufferTexCoordSet0 = nullptr;
						const float* bufferTexCoordSet1 = nullptr;
						const uint16_t* bufferJoints = nullptr;
						const float* bufferWeights = nullptr;

						int posByteStride;
						int normByteStride;
						int uv0ByteStride;
						int uv1ByteStride;
						int jointByteStride;
						int weightByteStride;

						// Position attribute is required
						assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

						const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
						bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
						posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
						posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
						vertexCount = static_cast<uint32_t>(posAccessor.count);
						posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

						if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
							const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
							const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
							bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
							normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
						}

						if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
							const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
							const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
							bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
							uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
						}
						if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
							const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
							const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
							bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
							uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
						}

						// Skinning
						// Joints
						if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
							const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
							const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
							bufferJoints = reinterpret_cast<const uint16_t*>(&(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));
							jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / sizeof(bufferJoints[0])) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
						}

						if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
							const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
							const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
							bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
							weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
						}

						hasSkin = (bufferJoints && bufferWeights);

						for (size_t v = 0; v < posAccessor.count; v++) {
							Vertex vert{};
							vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
							vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
							vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
							vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);

							vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * jointByteStride])) : glm::vec4(0.0f);
							vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
							// Fix for all zero weights
							if (glm::length(vert.weight0) == 0.0f) {
								vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
							}
							_verts.push_back(vert);
						}
					}
					// Indices
					if (hasIndices)
					{
						const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
						const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

						indexCount = static_cast<uint32_t>(accessor.count);
						const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

						switch (accessor.componentType) {
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
							const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
							const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
							const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						default:
							std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
							return;
						}
					}

					Model::Primitive prim;
					auto drawCall = std::make_shared<vkl::DrawCall>();
					drawCall->setCount(indexCount);
					drawCall->setOffset(indexStart);
					drawCall->setIndexBuffer(_indexBuffer);
					prim.draw = drawCall;
					prim.material = primitive.material;
					_primitives.emplace_back(std::move(prim));
				}
			}
		}






		//API
		virtual std::span<const Vertex> getVerts() const override
		{
			return _verts;
		}
		virtual std::shared_ptr<const vkl::VertexBuffer> getVertexBuffer() const override
		{
			return _vertexBuffer;
		}

		virtual std::span<const uint32_t> getIndices() const  override
		{
			return _indices;
		}

		virtual std::shared_ptr<const vkl::IndexBuffer> getIndexBuffer() const  override
		{
			return _indexBuffer;
		}

		virtual std::span<const Primitive> getPrimitives() const  override
		{
			return _primitives;
		}

		virtual std::span<const Material> getMaterials() const override
		{
			return _materials;
		}


	private:


		//external
		std::shared_ptr<vkl::VertexBuffer> _vertexBuffer;
		std::shared_ptr<vkl::IndexBuffer> _indexBuffer;
		std::vector<std::shared_ptr<vkl::TextureBuffer>> _textureBuffers;
		std::vector<Model::Material> _materials;
		std::vector<Model::Primitive> _primitives;

		//internal
		std::vector<Model::Vertex> _verts;
		std::vector<uint32_t> _indices;

		std::vector<vkl::TextureOptions> _texOptions;

		//node stuffs
		std::vector<NodeTransform> _nodeTransforms;
		std::vector<int> _nodeParents;
	};
}
IMPL_REFLECTION(vxt::Model)
IMPL_REFLECTION(vxt::glTFModel)