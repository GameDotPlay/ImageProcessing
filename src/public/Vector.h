#pragma once

#include <cstdint>
#include <functional>

struct Vec2
{
	uint8_t x = 0;
	uint8_t y = 0;

	bool operator==(const Vec2& other) const
	{
		return (x == other.x
			&& y == other.y);
	}
};

struct Vec3
{
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;

	bool operator==(const Vec3& other) const
	{
		return (x == other.x
			&& y == other.y
			&& z == other.z);
	}
};

struct Vec4
{
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;
	uint8_t w = 0;

	bool operator==(const Vec4& other) const
	{
		return (x == other.x
			&& y == other.y
			&& z == other.z
			&& w == other.w);
	}
};

struct Vec2f
{
	float x = 0.0f;
	float y = 0.0f;
};

struct Vec3f
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct Vec4f
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
};

namespace std
{
	template<> struct hash<Vec4>
	{
		size_t operator()(const Vec4& k) const
		{
			return ((hash<uint8_t>()(k.x)
				^ (hash<uint8_t>()(k.y) << 1)) >> 1)
				^ (hash<uint8_t>()(k.z) << 1)
				^ (hash<uint8_t>()(k.w) << 1);
		}
	};
}

