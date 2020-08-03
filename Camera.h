#pragma once
#include <DirectXMath.h>
#include <glm/gtc/matrix_transform.hpp>
#include <type_traits>
#include "Structures/Vertex.h"

class Camera
{
public:
	enum class Mode
	{
		Watch, Directional, Chase, TPS, FPS, Max
	};

	union ProjectionObject {
		glm::mat4 ProjectionGLM;
		DirectX::XMMATRIX ProjectionDX;
	};

	Camera(float width, float height);
	~Camera() = default;
	void Update(const Vector3& playerPos, float playerAngle);

	template <typename T = glm::mat4>
	T SetOrthographicMatrix(float width, float height, float near, float far);
	template <typename T = glm::mat4>
	T SetPerspectiveMatrix(float fieldOfView, float aspect, float near, float far);
	template <typename T = glm::mat4>
	T GetViewMatrix() const noexcept;
	template <typename T = glm::mat4>
	T GetProjectionMatrix() const noexcept;

	Vector3 Position = Vector3::Zero();
	Vector3 Target = Vector3::Zero();
	ProjectionObject Projection;
	float ScreenWidth = 0.0f;
	float ScreenHeight = 0.0f;

private:
	void Watch(const Vector3& playerPos);
	void Directional(const Vector3& playerPos);
	void Chase(const Vector3& playerPos);
	void TPS(const Vector3& playerPos, float playerAngle);
	void FPS(const Vector3& playerPos, float playerAngle);

	Mode m_mode = {};
};

template<typename T>
inline T Camera::SetOrthographicMatrix(float width, float height, float near, float far)
{
	if constexpr (std::is_same_v<T, glm::mat4>)
	{
		Projection.ProjectionGLM = glm::ortho(0.0f, width, height, 0.0f, near, far);
		return Projection.ProjectionGLM;
	}
	else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4X4> || std::is_same_v<T, DirectX::XMMATRIX>)
	{
		Projection.ProjectionDX = DirectX::XMMatrixOrthographicLH(width, height, near, far);

		if constexpr (std::is_same_v<T, DirectX::XMMATRIX>)
		{
			return Projection.ProjectionDX;
		}
		else
		{
			auto m = DirectX::XMFLOAT4X4();
			DirectX::XMStoreFloat4x4(&m, Projection.ProjectionDX);
			return m;
		}
	}
}

template<typename T>
inline T Camera::SetPerspectiveMatrix(float fieldOfView, float aspect, float near, float far)
{
	if constexpr (std::is_same_v<T, glm::mat4>)
	{
		Projection.ProjectionGLM = glm::perspective(fieldOfView, aspect, near, far);
		return Projection.ProjectionGLM;
	}
	else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4X4> || std::is_same_v<T, DirectX::XMMATRIX>)
	{
		Projection.ProjectionDX = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, aspect, near, far);
		
		if constexpr (std::is_same_v<T, DirectX::XMMATRIX>)
		{
			return Projection.ProjectionDX;
		}
		else
		{
			auto m = DirectX::XMFLOAT4X4();
			DirectX::XMStoreFloat4x4(&m, Projection.ProjectionDX);
			return m;
		}
	}
}

template<typename T>
inline T Camera::GetViewMatrix() const noexcept
{
	if constexpr (std::is_same_v<T, glm::mat4>)
	{
		return glm::lookAt(static_cast<glm::vec3>(Position), static_cast<glm::vec3>(Target), glm::vec3(0.0f, -1.0f, 0.0f));
	}
	else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4X4> || std::is_same_v<T, DirectX::XMMATRIX>)
	{
		auto eye = DirectX::XMVectorSet(Position.x, Position.y, Position.z, 1.0f);
		auto look_at = DirectX::XMVectorSet(Target.x, Target.y, Target.z, 1.0f);
		auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		auto view = DirectX::XMMatrixLookAtLH(eye, look_at, up);

		if constexpr (std::is_same_v<T, DirectX::XMMATRIX>)
		{
			return view;
		}
		else
		{
			auto m = DirectX::XMFLOAT4X4();
			DirectX::XMStoreFloat4x4(&m, view);
			return m;
		}
	}
}

template<typename T>
inline T Camera::GetProjectionMatrix() const noexcept
{
	if constexpr (std::is_same_v<T, glm::mat4>)
		return Projection.ProjectionGLM;
	else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4X4> || std::is_same_v<T, DirectX::XMMATRIX>)
	{
		return Projection.ProjectionDX;
	}
}
