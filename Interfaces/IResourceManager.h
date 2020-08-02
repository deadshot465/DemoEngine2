#pragma once
#include <algorithm>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include "IDisposable.h"
#include "../UtilsCommon.h"

class IResourceManager
{
public:
	template <Disposable T>
	T* AddResource(std::unique_ptr<T>& resource);
	template <Disposable T>
	T* AddResource(std::unique_ptr<T>& resource, std::string_view resourceName);
	template <Disposable T>
	T* GetResource(std::string_view resourceName);
	
	void RemoveResource(std::string_view resourceName)
	{
		auto item = std::find_if(m_resources.begin(), m_resources.end(), [&](const std::unique_ptr<IDisposable>& resource) {
			return resource->Name == resourceName;
			});

		item->get()->Dispose();
		m_resources.erase(item);
	}

private:
	std::vector<std::unique_ptr<IDisposable>> m_resources;

public:
	IResourceManager()
	{
		m_resources.reserve(100);
	}

	virtual ~IResourceManager()
	{
		for (auto& resource : m_resources)
			resource->Dispose();
		m_resources.clear();
	}
};

template<Disposable T>
inline T* IResourceManager::AddResource(std::unique_ptr<T>& resource)
{
	return AddResource(resource, GetRandomString(7));
}

template<Disposable T>
inline T* IResourceManager::AddResource(std::unique_ptr<T>& resource, std::string_view resourceName)
{
	auto& _resource = m_resources.emplace_back(std::move(resource));
	_resource->Name = resourceName;
	return dynamic_cast<T*>(_resource.get());
}

template<Disposable T>
inline T* IResourceManager::GetResource(std::string_view resourceName)
{
	if (m_resources.empty()) return nullptr;
	auto item = std::find_if(m_resources.begin(), m_resources.end(), [&](const std::unique_ptr<IDisposable>& resource) {
		return resource->Name == resourceName;
		});

	return dynamic_cast<T*>(item->get());
}
