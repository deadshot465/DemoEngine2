#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

enum class BlendMode
{
	None, Alpha, Add, Subtract, Replace, Multiply, Lighten, Darken, Screen, End
};

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