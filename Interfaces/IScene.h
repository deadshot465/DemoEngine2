#pragma once
#include <concepts>
#include <string>
#include <type_traits>
#include "IGraphics.h"

class IScene
{
public:
	IScene(IGraphics* graphics)
		: m_graphics(graphics)
	{

	}

	virtual ~IScene() = default;

	virtual void Initialize() {}
	virtual void LoadContent() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float deltaTime) = 0;

	std::string SceneName = "";

protected:
	IGraphics* m_graphics = nullptr;
};

template <typename T>
concept Scene = std::is_base_of_v<IScene, T> && std::is_convertible_v<const volatile T*, const volatile IScene*>;