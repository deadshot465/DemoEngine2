#include "Camera.h"
#include <cmath>

Camera::Camera(float width, float height)
	: Position(0.0f, 10.0f, -15.0f), Target(0.0f), ScreenWidth(width), ScreenHeight(height),
	m_mode(Mode::Watch)
{
	auto field_of_view = glm::radians(30.0f);
	auto aspect = width / height;
	SetPerspectiveMatrix<glm::mat4>(field_of_view, aspect, 0.1f, 100.0f);
}

void Camera::Update(const Vector3& playerPos, float playerAngle)
{
	switch (m_mode)
	{
	case Camera::Mode::Watch:
		Watch(playerPos);
		break;
	case Camera::Mode::Directional:
		Directional(playerPos);
		break;
	case Camera::Mode::Chase:
		Chase(playerPos);
		break;
	case Camera::Mode::TPS:
		TPS(playerPos, playerAngle);
		break;
	case Camera::Mode::FPS:
		FPS(playerPos, playerAngle);
		break;
	default:
		break;
	}
}

void Camera::Watch(const Vector3& playerPos)
{
	Position = Vector3(0.0f, 10.0f, -15.0f);
	Target = playerPos;
}

void Camera::Directional(const Vector3& playerPos)
{
	Position.x = playerPos.x + 8.0f;
	Position.y = playerPos.y + 5.0f;
	Position.z = playerPos.z - 8.0f;
	Target = playerPos;
}

void Camera::Chase(const Vector3& playerPos)
{
	constexpr auto min_distance = 5.0f;
	constexpr auto max_distance = 15.0f;

	auto dx = playerPos.x - Position.x;
	auto dz = playerPos.z - Position.z;
	auto distance = std::sqrtf(dx * dx + dz * dz);

	if (distance < min_distance)
	{
		dx /= distance;
		dz /= distance;
		Position.x = playerPos.x - min_distance * dx;
		Position.z = playerPos.z - min_distance * dz;
	}
	if (distance > max_distance)
	{
		dx /= distance;
		dz /= distance;
		Position.x = playerPos.x - max_distance * dx;
		Position.z = playerPos.z - max_distance * dz;
	}

	Target = playerPos;
}

void Camera::TPS(const Vector3& playerPos, float playerAngle)
{
	constexpr auto distance = 12.0f;
	auto dx = std::sinf(playerAngle);
	auto dz = std::cosf(playerAngle);

	Position.x = playerPos.x - distance * dx;
	Position.z = playerPos.z - distance * dz;
	Target = playerPos;
}

void Camera::FPS(const Vector3& playerPos, float playerAngle)
{
	constexpr auto height = 0.75f;
	auto dx = std::sinf(playerAngle);
	auto dz = std::cosf(playerAngle);

	Position.x = playerPos.x;
	Position.y = playerPos.y + height;
	Position.z = playerPos.z;
	
	Target.x = Position.x + dx;
	Target.y = Position.y;
	Target.z = Position.z + dz;
}
