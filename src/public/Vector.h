#pragma once

#include <cstdint>
#include <functional>

struct Vec2
{
	uint8_t x;
	uint8_t y;

	bool operator==(const Vec2& other) const
	{
		return (x == other.x
			&& y == other.y);
	}
};

struct Vec3
{
	uint8_t x;
	uint8_t y;
	uint8_t z;

	bool operator==(const Vec3& other) const
	{
		return (x == other.x
			&& y == other.y
			&& z == other.z);
	}
};

struct Vec4
{
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t w;

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
	float x;
	float y;
};

struct Vec3f
{
	float x;
	float y;
	float z;
};

struct Vec4f
{
	float x;
	float y;
	float z;
	float w;
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

