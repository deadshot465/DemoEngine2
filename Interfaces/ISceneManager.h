#pragma once
#include <memory>
#include <utility>
#include <vector>
#include "IScene.h"

class ISceneManager
{
public:
	ISceneManager() = default;
	
	~ISceneManager()
	{
		for (auto& scene : m_scenes)
			scene.reset();
	}

	virtual void Initialize()
	{
		for (auto& scene : m_scenes)
			scene->Initialize();
	}

	virtual void LoadContent()
	{
		for (auto& scene : m_scenes)
			scene->LoadContent();
	}

	virtual void Update(float deltaTime)
	{
		for (auto& scene : m_scenes)
			scene->Update(deltaTime);
	}

	virtual void Render(float deltaTime)
	{
		for (auto& scene : m_scenes)
			scene->Render(deltaTime);
	}

	template <Scene T>
	T* RegisterScene(std::unique_ptr<T>& scene)
	{
		m_scenes.emplace_back(std::move(scene));
	}

protected:
	std::vector<std::unique_ptr<IScene>> m_scenes;
};
