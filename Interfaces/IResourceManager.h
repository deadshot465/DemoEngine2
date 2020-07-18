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

template <typename T>
concept Disposable = std::is_base_of_v<IDisposable, T> && std::is_convertible_v<const volatile T*, const volatile IDisposable*>;

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
	auto& resource = m_resources.emplace_back(std::move(resource));
	resource->get()->Name = resourceName;
	return resource->get();
}

template<Disposable T>
inline T* IResourceManager::GetResource(std::string_view resourceName)
{
	auto item = std::find_if(m_resources.begin(), m_resources.end(), [&](const std::unique_ptr<IDisposable>& resource) {
		return resource->Name == resourceName;
		});

	return dynamic_cast<T*>(item->get());
}
