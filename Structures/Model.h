#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <d3d12.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "../GLVK/VK/PipelineVK.h"
#include "../GLVK/VK/UtilsVK.h"
#include "../Interfaces/IDisposable.h"
#include "../Interfaces/IGraphics.h"
#include "../Interfaces/IResourceManager.h"
#include "../Structures/Matrix.h"
#include "../Structures/Vertex.h"

template <Disposable Texture, Disposable Buffer>
struct Mesh
	: public IDisposable
{
public:
	Mesh() = default;

	Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Texture*>& textures, std::vector<uint32_t>& textureIndices, std::shared_ptr<Buffer>& vertexBuffer = nullptr, std::shared_ptr<Buffer>& indexBuffer = nullptr)
		: Vertices(vertices),
		Indices(indices),
		Textures(textures),
		TextureIndices(textureIndices),
		VertexBuffer(vertexBuffer),
		IndexBuffer(indexBuffer)
	{

	}

	Mesh(const Mesh& mesh)
		: Vertices(mesh.Vertices), Indices(mesh.Indices), Textures(mesh.Textures), TextureIndices(mesh.TextureIndices)
	{

	}

	explicit Mesh(Mesh&& mesh) noexcept
		: Vertices(std::move(mesh.Vertices)), Indices(std::move(mesh.Indices)), Textures(std::move(mesh.Textures)), TextureIndices(std::move(mesh.TextureIndices)), VertexBuffer(std::move(mesh.VertexBuffer)), IndexBuffer(std::move(mesh.IndexBuffer))
	{
	}

	Mesh& operator=(const Mesh& mesh)
	{
		if (this == &mesh) return *this;

		Vertices = mesh.Vertices;
		Indices = mesh.Indices;
		Textures = mesh.Textures;
		TextureIndices = mesh.TextureIndices;

		return *this;
	}

	Mesh& operator=(Mesh&& mesh) noexcept
	{
		if (this == &mesh) return *this;

		std::swap(Vertices, mesh.Vertices);
		std::swap(Indices, mesh.Indices);
		std::swap(Textures, mesh.Textures);
		std::swap(TextureIndices, mesh.TextureIndices);
		std::swap(VertexBuffer, mesh.VertexBuffer);
		std::swap(IndexBuffer, mesh.IndexBuffer);

		return *this;
	}

	virtual void Dispose() override
	{
		for (auto& texture : Textures)
			texture->Dispose();

		VertexBuffer->Dispose();
		IndexBuffer->Dispose();
	}

	template <typename T = glm::mat4>
	T GetWorldMatrix() const noexcept
	{
		if constexpr (std::is_same_v<T, glm::mat4>)
		{
			auto world = glm::mat4(1.0f);

			auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(ScaleX, ScaleY, ScaleZ));

			auto rotate_z = glm::rotate(glm::mat4(1.0f), RotationZ, glm::vec3(0.0f, 0.0f, 1.0f));
			auto rotate_y = glm::rotate(glm::mat4(1.0f), RotationY, glm::vec3(0.0f, -1.0f, 0.0f));
			auto rotate_x = glm::rotate(glm::mat4(1.0f), RotationX, glm::vec3(1.0f, 0.0f, 0.0f));

			auto translate = glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(Position));
			auto rotate = rotate_z * rotate_y * rotate_x;
			world = scale * rotate * translate * world;

			return world;
		}
		else if constexpr (std::is_same_v<T, DirectX::XMMATRIX> || std::is_same_v<T, DirectX::XMFLOAT4X4>)
		{
			auto world = DirectX::XMMatrixIdentity();
			auto scale = DirectX::XMMatrixScaling(ScaleX, ScaleY, ScaleZ);
			auto rotate_z = DirectX::XMMatrixRotationZ(RotationZ);
			auto rotate_y = DirectX::XMMatrixRotationY(RotationY);
			auto rotate_x = DirectX::XMMatrixRotationX(RotationX);
			auto translate = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
			auto rotate = rotate_z * rotate_y * rotate_x;
			
			world = scale * rotate * translate * world;

			if constexpr (std::is_same_v<T, DirectX::XMFLOAT4X4>)
			{
				auto mat = DirectX::XMFLOAT4X4();
				DirectX::XMStoreFloat4x4(&mat, world);
				return mat;
			}
			else
			{
				return world;
			}
		}
	}

	template <typename T>
	auto Render(float deltaTime, const T& commandBuffer, uint32_t dynamicOffset, const GLVK::VK::Pipeline* pipeline, const vk::DescriptorSet& descriptorSet, GLVK::VK::PushConstant& pushConstant) -> typename std::enable_if_t<std::is_same_v<T, vk::CommandBuffer>, void>
	{
		uint32_t dynamic_offset = ModelIndex * dynamicOffset;

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline(BlendMode::None, ShaderType::BasicShaderForMesh));

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(ShaderType::BasicShaderForMesh), 0, { descriptorSet }, { dynamic_offset, dynamic_offset });

		pushConstant.ObjectColor = Color;
		commandBuffer.pushConstants<GLVK::VK::PushConstant>(pipeline->GetPipelineLayout(ShaderType::BasicShaderForMesh), vk::ShaderStageFlagBits::eFragment, 0, { pushConstant });
		commandBuffer.bindVertexBuffers(0, VertexBuffer->GetBuffer(), { 0 });
		commandBuffer.bindIndexBuffer(IndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
		commandBuffer.drawIndexed(static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
	}

	template <typename T>
	auto Render(float deltaTime, T* commandBuffer) -> typename std::enable_if_t<std::is_same_v<T, ID3D12CommandList>, void>
	{

	}

	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<Texture*> Textures;
	std::vector<unsigned int> TextureIndices;
	std::shared_ptr<Buffer> VertexBuffer;
	std::shared_ptr<Buffer> IndexBuffer;
	
	Vector3 Position = Vector3();
	float ScaleX = 0.0f;
	float ScaleY = 0.0f;
	float ScaleZ = 0.0f;
	float RotationX = 0.0f;
	float RotationY = 0.0f;
	float RotationZ = 0.0f;
	Vector4 Color = Vector4();
	uint32_t ModelIndex = 0;
};

template <Disposable Texture, Disposable Buffer>
struct Model
	: public IDisposable
{
public:
	Model() = default;

	Model(const Model& model)
		: Meshes(model.Meshes), Position(model.Position), ScaleX(model.ScaleX),
		ScaleY(model.ScaleY), ScaleZ(model.ScaleZ), RotationX(model.RotationX),
		RotationY(model.RotationY), RotationZ(model.RotationZ), Color(model.Color),
		ModelIndex(model.ModelIndex)
	{
		m_handle = model.m_handle;
		m_isDisposed = model.m_isDisposed;
		this->Name = model.Name;
	}
	
	explicit Model(std::vector<Mesh<Texture, Buffer>>& meshes)
		: Meshes(std::move(meshes))
	{

	}

	Model(Model&& model) noexcept
		: Meshes(std::move(model.Meshes))
	{

	}

	Model& operator=(const Model& model)
	{
		if (this == &model) return *this;

		Meshes = model.Meshes;
		Position = model.Position;
		ScaleX = model.ScaleX;
		ScaleY = model.ScaleY;
		ScaleZ = model.ScaleZ;
		RotationX = model.RotationX;
		RotationY = model.RotationY;
		RotationZ = model.RotationZ;
		Color = model.Color;
		ModelIndex = model.ModelIndex;
		m_handle = model.m_handle;
		m_isDisposed = model.m_isDisposed;
		Name = model.Name;

		return *this;
	}

	Model& operator=(Model&& model) noexcept
	{
		if (this == &model) return *this;

		std::swap(Meshes, model.Meshes);

		return *this;
	}

	virtual void Dispose() override
	{
		for (auto& mesh : Meshes)
			mesh.Dispose();
	}

	void Load(std::string_view fileName, IGraphics* graphics, const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color, bool flipUV = true)
	{
		Position = position;
		ScaleX = scale.x;
		ScaleY = scale.y;
		ScaleZ = scale.z;
		RotationX = glm::radians(rotation.x);
		RotationY = glm::radians(rotation.y);
		RotationZ = glm::radians(rotation.z);
		Color = color;

		auto importer = Assimp::Importer();
		auto scene = importer.ReadFile(fileName.data(), flipUV ? DEFAULT_FLAGS | aiProcess_FlipUVs : DEFAULT_FLAGS);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			throw std::runtime_error(importer.GetErrorString());
		}

		ProcessNode(scene->mRootNode, scene, this, graphics, fileName);
	}

	template <typename T = glm::mat4>
	T GetWorldMatrix() const noexcept
	{
		if constexpr (std::is_same_v<T, glm::mat4>)
		{
			auto world = glm::mat4(1.0f);

			auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(ScaleX, ScaleY, ScaleZ));

			auto rotate_z = glm::rotate(glm::mat4(1.0f), RotationZ, glm::vec3(0.0f, 0.0f, 1.0f));
			auto rotate_y = glm::rotate(glm::mat4(1.0f), RotationY, glm::vec3(0.0f, -1.0f, 0.0f));
			auto rotate_x = glm::rotate(glm::mat4(1.0f), RotationX, glm::vec3(1.0f, 0.0f, 0.0f));

			auto translate = glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(Position));
			auto rotate = rotate_z * rotate_y * rotate_x;
			world = scale * rotate * translate * world;

			return world;
		}
		else if constexpr (std::is_same_v<T, DirectX::XMMATRIX> || std::is_same_v<T, DirectX::XMFLOAT4X4>)
		{
			auto world = DirectX::XMMatrixIdentity();
			auto scale = DirectX::XMMatrixScaling(ScaleX, ScaleY, ScaleZ);
			auto rotate_z = DirectX::XMMatrixRotationZ(RotationZ);
			auto rotate_y = DirectX::XMMatrixRotationY(RotationY);
			auto rotate_x = DirectX::XMMatrixRotationX(RotationX);
			auto translate = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
			auto rotate = rotate_z * rotate_y * rotate_x;

			world = scale * rotate * translate * world;

			if constexpr (std::is_same_v<T, DirectX::XMFLOAT4X4>)
			{
				auto mat = DirectX::XMFLOAT4X4();
				DirectX::XMStoreFloat4x4(&mat, world);
				return mat;
			}
			else
			{
				return world;
			}
		}
	}

	template <typename T>
	auto Render(float deltaTime, const T& commandBuffer, uint32_t dynamicOffset, const GLVK::VK::Pipeline* pipeline, const vk::DescriptorSet& descriptorSet, GLVK::VK::PushConstant& pushConstant) -> typename std::enable_if_t<std::is_same_v<T, vk::CommandBuffer>, void>
	{
		uint32_t dynamic_offset = ModelIndex * dynamicOffset;

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline(BlendMode::None, ShaderType::BasicShader));

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(ShaderType::BasicShader), 0, { descriptorSet }, { dynamic_offset, dynamic_offset });

		for (const auto& mesh : Meshes)
		{
			pushConstant.ObjectColor = Color;
			commandBuffer.pushConstants<GLVK::VK::PushConstant>(pipeline->GetPipelineLayout(ShaderType::BasicShader), vk::ShaderStageFlagBits::eFragment, 0, { pushConstant });
			commandBuffer.bindVertexBuffers(0, mesh.VertexBuffer->GetBuffer(), { 0 });
			commandBuffer.bindIndexBuffer(mesh.IndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
			commandBuffer.drawIndexed(static_cast<uint32_t>(mesh.Indices.size()), 1, 0, 0, 0);
		}
	}

	template <typename T>
	auto Render(float deltaTime, T* commandBuffer) -> typename std::enable_if_t<std::is_same_v<T, ID3D12CommandList>, void>
	{

	}

	std::vector<Mesh<Texture, Buffer>> Meshes;
	
	Vector3 Position = Vector3();
	float ScaleX = 0.0f;
	float ScaleY = 0.0f;
	float ScaleZ = 0.0f;
	float RotationX = 0.0f;
	float RotationY = 0.0f;
	float RotationZ = 0.0f;
	Vector4 Color = Vector4();
	uint32_t ModelIndex = 0;

private:
	inline static constexpr unsigned int DEFAULT_FLAGS = aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;

	static Mesh<Texture, Buffer> ProcessMesh(aiMesh* mesh, const aiScene* scene, IGraphics* graphics, std::string_view fileName)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Texture*> textures;
		std::vector<unsigned int> texture_indices;
		Mesh<Texture, Buffer> _mesh{};

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex{};
			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;

			if (mesh->mTextureCoords[0])
			{
				vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
				vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.TexCoord.x = 0.0f;
				vertex.TexCoord.y = 0.0f;
			}

			vertices.emplace_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			auto face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indices.emplace_back(face.mIndices[j]);
			}
		}

		if (mesh->mMaterialIndex >= 0)
		{
			auto material = scene->mMaterials[mesh->mMaterialIndex];
			auto directory = std::string(fileName.substr(0, fileName.find_last_of('/'))) + '/';

			for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE); ++i)
			{
				auto str = aiString();
				material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, i, &str);
				auto file_name = directory + str.C_Str();
				auto texture = graphics->LoadTexture(file_name);
				textures.emplace_back(reinterpret_cast<Texture*>(std::get<0>(texture)));
				texture_indices.emplace_back(std::get<1>(texture));
			}

			for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType::aiTextureType_SPECULAR); ++i)
			{
				auto str = aiString();
				material->GetTexture(aiTextureType::aiTextureType_SPECULAR, i, &str);
				auto file_name = directory + str.C_Str();
				auto texture = graphics->LoadTexture(file_name);
				textures.emplace_back(reinterpret_cast<Texture*>(std::get<0>(texture)));
				texture_indices.emplace_back(std::get<1>(texture));
			}
		}

		_mesh.Vertices = vertices;
		_mesh.Indices = indices;
		_mesh.Textures = textures;
		_mesh.TextureIndices = texture_indices;
		return _mesh;
	}

	static void ProcessNode(aiNode* node, const aiScene* scene, Model<Texture, Buffer>* model, IGraphics* graphics, std::string_view fileName)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			auto mesh = scene->mMeshes[node->mMeshes[i]];
			Mesh<Texture, Buffer>& _mesh = model->Meshes.emplace_back(ProcessMesh(mesh, scene, graphics, fileName));
			_mesh.Color = model->Color;
			_mesh.Position = model->Position;
			_mesh.ScaleX = model->ScaleX;
			_mesh.ScaleY = model->ScaleY;
			_mesh.ScaleZ = model->ScaleZ;
			_mesh.RotationX = model->RotationX;
			_mesh.RotationY = model->RotationY;
			_mesh.RotationZ = model->RotationZ;
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			ProcessNode(node->mChildren[i], scene, model, graphics, fileName);
		}
	}
};