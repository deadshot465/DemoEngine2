#pragma once
#include <algorithm>
#include <memory>
#include <string_view>
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
		m_currentScene->Initialize();
	}

	virtual void LoadContent()
	{
		m_currentScene->LoadContent();
	}

	virtual void Update(float deltaTime)
	{
		m_currentScene->Update(deltaTime);
	}

	virtual void Render(float deltaTime)
	{
		m_currentScene->Render(deltaTime);
	}

	template <Scene T>
	T* RegisterScene(std::unique_ptr<T>& scene, std::string_view sceneName)
	{
		scene->SceneName = sceneName;
		auto& _scene = m_scenes.emplace_back(std::move(scene));
		return dynamic_cast<T*>(_scene.get());
	}

	void SetCurrentScene(std::string_view sceneName)
	{
		auto scene = std::find_if(m_scenes.cbegin(), m_scenes.cend(), [&](const std::unique_ptr<IScene>& _scene) {
			return _scene->SceneName == sceneName;
			});
		m_currentScene = scene->get();
	}

	template <Scene T>
	void SetCurrentScene(T* sceneName)
	{
		m_currentScene = sceneName;
	}

protected:
	std::vector<std::unique_ptr<IScene>> m_scenes;
	IScene* m_currentScene = nullptr;
};
