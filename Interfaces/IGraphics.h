#pragma once
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include "IResourceManager.h"
#include "IDisposable.h"
#include "../Structures/Vertex.h"
#include "../UtilsCommon.h"

class IGraphics
{
public:
	IGraphics(void* handle, int width, int height, IResourceManager* resourceManager, bool flipY = false)
		: m_handle(handle), m_width(width), m_height(height), m_resourceManager(resourceManager)
	{
		GeneratePrimitiveData(flipY);
	}

	static void AddTask(std::future<void>& future)
	{
		m_futures.emplace_back(std::move(future));
	}

	static void WaitForAllTasks()
	{
		for (auto& future : m_futures)
			future.wait();

		m_futures.clear();
	}

	virtual ~IGraphics() = default;
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;
	virtual void BeginDraw() = 0;
	virtual void EndDraw() = 0;

	virtual std::shared_ptr<IDisposable> CreateVertexBuffer(const std::vector<Vertex>& vertices) = 0;
	virtual std::shared_ptr<IDisposable> CreateIndexBuffer(const std::vector<uint32_t>& indices) = 0;
	virtual std::tuple<IDisposable*, unsigned int> LoadTexture(std::string_view fileName) = 0;
	virtual std::tuple<IDisposable*, unsigned int> LoadModel(std::string_view modelName, const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color) = 0;
	virtual std::tuple<IDisposable*, unsigned int> CreateMesh(const PrimitiveType& primitiveType, const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color) = 0;

protected:
	static void GenerateBoardData(bool flipY)
	{

	}
	
	static void GenerateCubeData(bool flipY)
	{
		std::vector<Vertex> vertices{};

		if (flipY)
		{
			const std::vector<Vertex> cube_vertices =
			{
				// Front Face
				{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },

				// Top Face
				{ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },

				// Back Face
				{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },

				// Bottom Face
				{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },

				// Left Face
				{ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },

				// Right Face
				{ {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
			};

			vertices = cube_vertices;
		}
		else
		{
			const std::vector<Vertex> cube_vertices =
			{
				// Front Face
				{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },

				// Top Face
				{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },

				// Back Face
				{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },

				// Bottom Face
				{ { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },

				// Left Face
				{ { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },

				// Right Face
				{ {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
			};

			vertices = cube_vertices;
		}

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,

			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,

			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		ShapeData shape_data = {
			.Vertices = vertices,
			.Indices = indices
		};

		std::lock_guard<std::mutex> lock{ m_mutex };
		m_shapeData.emplace(std::make_pair(PrimitiveType::Cube, shape_data));
	}
	
	static void GenerateRectData(bool flipY)
	{
		auto vertices = std::vector<Vertex>(4);
		auto indices = std::vector<uint32_t>(3 * 2);

		vertices[0].Position = Vector3(-0.5f, 0.0f, 0.5f);
		vertices[1].Position = Vector3(0.5f, 0.0f, 0.5f);
		vertices[2].Position = Vector3(0.5f, 0.0f, -0.5f);
		vertices[3].Position = Vector3(-0.5f, 0.0f, -0.5f);
		vertices[0].Normal = vertices[1].Normal = vertices[2].Normal
			= vertices[3].Normal = Vector3(0.0f, 1.0f, 0.0f);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 2;
		indices[4] = 3;
		indices[5] = 0;

		ShapeData shape_data = {
			.Vertices = vertices,
			.Indices = indices
		};

		std::lock_guard<std::mutex> lock(m_mutex);
		m_shapeData.emplace(std::make_pair(PrimitiveType::Rect, shape_data));
	}

	static void GenerateSphereData(bool flipY)
	{

	}

	static void GenerateCylinderData(bool flipY)
	{

	}

	static void GeneratePrimitiveData(bool flipY)
	{
		m_futures.emplace_back(std::async(std::launch::async, &IGraphics::GenerateBoardData, flipY));
		m_futures.emplace_back(std::async(std::launch::async, &IGraphics::GenerateCubeData, flipY));
		m_futures.emplace_back(std::async(std::launch::async, &IGraphics::GenerateRectData, flipY));
		m_futures.emplace_back(std::async(std::launch::async, &IGraphics::GenerateSphereData, flipY));
		m_futures.emplace_back(std::async(std::launch::async, &IGraphics::GenerateCylinderData, flipY));
	}

	inline static std::unordered_map<PrimitiveType, ShapeData> m_shapeData;
	inline static std::list<std::future<void>> m_futures;
	inline static std::mutex m_mutex;

	void* m_handle = nullptr;
	int m_width = 0;
	int m_height = 0;
	IResourceManager* m_resourceManager = nullptr;
};