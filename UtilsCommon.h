#pragma once
#include <algorithm>
#include <fstream>
#include <functional>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>
#include "Structures/Vertex.h"
#include "Structures/Matrix.h"

enum class BlendMode
{
	None, Alpha, Add, Subtract, Replace, Multiply, Lighten, Darken, Screen, End
};

enum class ShaderType
{
	BasicShader, BasicShaderForMesh
};

enum class PrimitiveType
{
	Board, Cube, Rect, Sphere, Cylinder, Capsule
};

struct MVP
{
	alignas(16) glm::mat4 Model;
	alignas(16) glm::mat4 View;
	alignas(16) glm::mat4 Projection;
};

struct DynamicBufferModels
{
	std::vector<uint32_t> ModelIndices;
	std::vector<glm::mat4> Models;
	glm::mat4* Buffer;
};

struct ShapeData
{
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
};

inline void ThrowIfFailed(std::string_view errorMsg)
{
	throw std::runtime_error(errorMsg.data());
}

template <typename T = char>
inline std::vector<T> ReadFromFile(std::string_view filePath, bool isBinary = true)
{
	auto data = std::vector<T>();
	auto fs = std::ifstream();
	auto flags = std::ios_base::ate | std::ios_base::in;

	if (isBinary) flags |= std::ios_base::binary;

	fs.open(filePath.data(), flags);

	if (fs.good())
	{
		auto pos = fs.tellg();
		data.resize(pos);
		fs.seekg(0, std::ios_base::beg);
		fs.read(reinterpret_cast<char*>(data.data()), pos);
	}

	fs.close();
	return data;
}

inline std::string WstringToString(std::wstring_view string) noexcept
{
	auto ss = std::stringstream();
	for (auto i = 0; i < string.size(); ++i)
	{
		ss << static_cast<char>(string[i]);
	}
	return ss.str();
}

#ifdef __APPLE__
template <typename T, size_t N>
inline size_t CountOf(const T (&arr)[N])
{
	static_assert(N >= 0);
	return N;
}
#define _countof(arr) CountOf(arr)
#endif

template <typename T = std::mt19937, size_t N = T::state_size>
inline auto GetRandomSeededEngine() -> typename std::enable_if_t<N != 0, T>
{
	typename T::result_type data[N];
	std::random_device rd;
	std::generate(std::begin(data), std::end(data), std::ref(rd));
	std::seed_seq seeds(std::begin(data), std::end(data));
	T engine(seeds);
	return engine;
}

static auto DEFAULT_ENGINE = GetRandomSeededEngine();

template <typename T = float>
inline T GetRandomNumber(T lower, T upper)
{
	if constexpr (std::is_floating_point_v<T>)
	{
		std::uniform_real_distribution<T> rng(lower, upper);
		return rng(DEFAULT_ENGINE);
	}
	else if constexpr (std::is_integral_v<T>)
	{
		std::uniform_int_distribution<T> rng(lower, upper);
		return rng(DEFAULT_ENGINE);
	}
}

inline std::string GetRandomString(size_t length)
{
	static std::string random_characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	auto generated = std::string();
	std::sample(random_characters.cbegin(), random_characters.cend(), std::back_inserter(generated), length, DEFAULT_ENGINE);
	return generated;
}