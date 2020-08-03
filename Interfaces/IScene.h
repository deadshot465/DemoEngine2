#pragma once
#include <concepts>
#include <type_traits>

class IScene
{
public:
	virtual void Initialize() {}
	virtual void LoadContent() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float deltaTime) = 0;
};

template <typename T>
concept Scene = std::is_base_of_v<IScene, T> && std::is_convertible_v<const volatile T*, const volatile IScene*>;