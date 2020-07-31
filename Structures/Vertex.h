#pragma once
#ifdef _WIN32
#include <DirectXMath.h>
#endif
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <utility>

struct Vector2
{
	float x, y;

	operator glm::vec2() const
	{
		return glm::vec2(x, y);
	}

#ifdef _WIN32
	operator DirectX::XMFLOAT2() const
	{
		return DirectX::XMFLOAT2(x, y);
	}

	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
	}
#endif
};

struct Vector3
{
	float x, y, z;

	operator glm::vec3() const
	{
		return glm::vec3(x, y, z);
	}

#ifdef _WIN32
	operator DirectX::XMFLOAT3() const
	{
		return DirectX::XMFLOAT3(x, y, z);
	}

	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(x, y, z, 0.0f);
	}
#endif
};

struct Vector4
{
	float x, y, z, w;

	operator glm::vec4() const
	{
		return glm::vec4(x, y, z, w);
	}

#ifdef _WIN32
	operator DirectX::XMFLOAT4() const
	{
		return DirectX::XMFLOAT4(x, y, z, w);
	}

	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(x, y, z, w);
	}
#endif
};

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;

	Vertex() = default;

	Vertex(const Vertex& vertex)
		: Position(vertex.Position), Normal(vertex.Normal), TexCoord(vertex.TexCoord)
	{
	}

	Vertex(Vertex&& vertex) noexcept
		: Position(std::move(vertex.Position)), Normal(std::move(vertex.Normal)), TexCoord(std::move(vertex.TexCoord))
	{	
	}

	Vertex(const Vector3& position, const Vector3& normal, const Vector2& texCoord)
		: Position(position), Normal(normal), TexCoord(texCoord)
	{

	}

	Vertex(Vector3&& position, Vector3&& normal, Vector2&& texCoord) noexcept
		: Position(std::move(position)), Normal(std::move(normal)), TexCoord(std::move(texCoord))
	{

	}

	Vertex& operator=(const Vertex& vertex)
	{
		if (&vertex == this)
			return *this;

		Position = vertex.Position;
		Normal = vertex.Normal;
		TexCoord = vertex.TexCoord;

		return *this;
	}

	Vertex& operator=(Vertex&& vertex) noexcept
	{
		if (&vertex == this)
			return *this;

		std::swap(Position, vertex.Position);
		std::swap(Normal, vertex.Normal);
		std::swap(TexCoord, vertex.TexCoord);

		return *this;
	}
};