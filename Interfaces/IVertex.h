#pragma once

template <typename T, typename U>
struct IVertex
{
	IVertex(const T& position, const T& normal, const U& texCoord)
		: Position(position), Normal(normal), TexCoord(texCoord)
	{

	}

	T Position;
	T Normal;
	U TexCoord;
};