#pragma once
#include <string_view>
#include "IResourceManager.h"

class IGraphics
{
public:
	IGraphics(void* handle, int width, int height, IResourceManager* resourceManager)
		: m_handle(handle), m_width(width), m_height(height), m_resourceManager(resourceManager)
	{

	}

	virtual ~IGraphics() = default;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;

	virtual void* LoadTexture(std::string_view fileName) = 0;
	virtual void CreateCube() = 0;
	virtual void CreateSphere() = 0;
	virtual void CreateCylinder() = 0;
	virtual void CreateCapsule() = 0;

protected:
	void* m_handle = nullptr;
	int m_width = 0;
	int m_height = 0;
	IResourceManager* m_resourceManager = nullptr;
};