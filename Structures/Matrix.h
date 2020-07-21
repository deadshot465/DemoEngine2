#pragma once
#include <DirectXMath.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Matrix3x3
{
	float _00, _01, _02, _10, _11, _12, _20, _21, _22;

	Matrix3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
		: _00(m00), _01(m01), _02(m02),
		_10(m10), _11(m11), _12(m12),
		_20(m20), _21(m21), _22(m22)
	{

	}

	Matrix3x3(const DirectX::XMFLOAT3X3& matrix)
		: _00(matrix._11), _01(matrix._12), _02(matrix._13),
		_10(matrix._21), _11(matrix._22), _12(matrix._23),
		_20(matrix._31), _21(matrix._32), _22(matrix._33)
	{

	}

	Matrix3x3(const glm::mat3& matrix)
		: _00(matrix[0][0]), _01(matrix[0][1]), _02(matrix[0][2]),
		_10(matrix[1][0]), _11(matrix[1][1]), _12(matrix[1][2]),
		_20(matrix[2][0]), _21(matrix[2][1]), _22(matrix[2][2])
	{

	}

	operator glm::mat3() const
	{
		auto matrix = glm::mat3();
		matrix[0][0] = _00;
		matrix[0][1] = _01;
		matrix[0][2] = _02;
		matrix[1][0] = _10;
		matrix[1][1] = _11;
		matrix[1][2] = _12;
		matrix[2][0] = _20;
		matrix[2][1] = _21;
		matrix[2][2] = _22;
		return matrix;
	}

	operator DirectX::XMFLOAT3X3() const
	{
		return DirectX::XMFLOAT3X3(_00, _01, _02, _10, _11, _12, _20, _21, _22);
	}
};

struct Matrix4x4
{
	float _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33;

	Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
		: _00(m00), _01(m01), _02(m02), _03(m03),
		_10(m10), _11(m11), _12(m12), _13(m13),
		_20(m20), _21(m21), _22(m22), _23(m23),
		_30(m30), _31(m31), _32(m32), _33(m33)
	{

	}

	Matrix4x4(const DirectX::XMFLOAT4X4& matrix)
		: _00(matrix._11), _01(matrix._12), _02(matrix._13), _03(matrix._14),
		_10(matrix._21), _11(matrix._22), _12(matrix._23), _13(matrix._24),
		_20(matrix._31), _21(matrix._32), _22(matrix._33), _23(matrix._34),
		_30(matrix._41), _31(matrix._42), _32(matrix._43), _33(matrix._44)
	{

	}

	Matrix4x4(const glm::mat4& matrix)
		: _00(matrix[0][0]), _01(matrix[0][1]), _02(matrix[0][2]), _03(matrix[0][3]),
		_10(matrix[1][0]), _11(matrix[1][1]), _12(matrix[1][2]), _13(matrix[1][3]),
		_20(matrix[2][0]), _21(matrix[2][1]), _22(matrix[2][2]), _23(matrix[2][3]),
		_30(matrix[3][0]), _31(matrix[3][1]), _32(matrix[3][2]), _33(matrix[3][3])
	{

	}

	operator glm::mat3() const
	{
		auto matrix = glm::mat3();
		matrix[0][0] = _00;
		matrix[0][1] = _01;
		matrix[0][2] = _02;
		matrix[0][3] = _03;
		matrix[1][0] = _10;
		matrix[1][1] = _11;
		matrix[1][2] = _12;
		matrix[1][3] = _13;
		matrix[2][0] = _20;
		matrix[2][1] = _21;
		matrix[2][2] = _22;
		matrix[2][3] = _23;
		matrix[3][0] = _30;
		matrix[3][1] = _31;
		matrix[3][2] = _32;
		matrix[3][3] = _33;
		return matrix;
	}

	operator DirectX::XMFLOAT4X4() const
	{
		return DirectX::XMFLOAT4X4(_00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33);
	}
};