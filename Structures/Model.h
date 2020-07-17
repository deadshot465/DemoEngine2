#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "../Interfaces/IDisposable.h"
#include "../Interfaces/IGraphics.h"
#include "../Interfaces/IVertex.h"

template <typename T, typename U, typename Texture>
struct Mesh
	: public IDisposable
{
public:
	Mesh() = default;

	Mesh(std::vector<IVertex<T, U>>& vertices, std::vector<uint32_t>& indices, std::vector<Texture*>& textures)
		: Vertices(vertices), Indices(indices), Textures(textures)
	{

	}

	Mesh(Mesh&& mesh) noexcept
		: Vertices(std::move(mesh.Vertices)), Indices(std::move(mesh.Indices)), Textures(std::move(mesh.Textures))
	{
	}

	Mesh& operator=(Mesh&& mesh) noexcept
	{
		if (this == &mesh) return *this;

		std::swap(Vertices, mesh.Vertices);
		std::swap(Indices, mesh.Indices);
		std::swap(Texture, mesh.Textures);

		return *this;
	}

	virtual void Dispose() override
	{
		for (auto& texture : Textures)
			texture->Dispose();
	}

	std::vector<IVertex<T, U>> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<Texture*> Textures;
};

template <typename T, typename U, typename Texture>
struct Model
	: public IDisposable
{
public:
	Model() = default;

	explicit Model(std::vector<Mesh<T, U, Texture>>& meshes)
		: Meshes(std::move(meshes))
	{

	}

	Model(Model&& model) noexcept
		: Meshes(std::move(model.Meshes))
	{

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

	Model<T, U, Texture> Load(std::string_view fileName, IGraphics* graphics, bool flipUV = true)
	{
		auto importer = Assimp::Importer();
		auto scene = importer.ReadFile(fileName.data(), flipUV ? DEFAULT_FLAGS | aiProcess_FlipUVs : DEFAULT_FLAGS);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			throw std::runtime_error(importer.GetErrorString());
		}

		Model<T, U, Texture> model{};
		ProcessNode(scene->mRootNode, scene, model, graphics, fileName);
		return model;
	}

	std::vector<Mesh<T, U, Texture>> Meshes;

private:
	inline static constexpr unsigned int DEFAULT_FLAGS = aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;

	static Mesh<T, U, Texture> ProcessMesh(aiMesh* mesh, const aiScene* scene, IGraphics* graphics, std::string_view fileName)
	{
		std::vector<IVertex<T, U>> vertices;
		std::vector<uint32_t> indices;
		std::vector<Texture*> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			IVertex<T, U> vertex{};
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
				auto file_name = directory.data();
				file_name += str.C_Str();
				textures.emplace_back(graphics->LoadTexture(file_name));
			}

			for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType::aiTextureType_SPECULAR); ++i)
			{
				auto str = aiString();
				material->GetTexture(aiTextureType::aiTextureType_SPECULAR, i, &str);
				auto file_name = directory.data();
				file_name += str.C_Str();
				textures.emplace_back(reinterpret_cast<Texture>(graphics->LoadTexture(file_name)));
			}
		}

		Mesh<T, U, Texture> _mesh{ vertices, indices, textures };
		return _mesh;
	}

	static void ProcessNode(aiNode* node, const aiScene* scene, Model<T, U, Texture>& model, IGraphics* graphics, std::string_view fileName)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			auto mesh = scene->mMeshes[node->mMeshes[i]];
			model.Meshes.emplace_back(ProcessMesh(mesh, scene, graphics, fileName));
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			ProcessNode(node->mChildren[i], scene, model, graphics, fileName);
		}
	}
};