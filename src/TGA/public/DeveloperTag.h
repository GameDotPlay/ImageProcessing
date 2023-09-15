#pragma once

#include <cstdint>

/** Represents a developer tag field of a TGA image. */
struct DeveloperTag
{
	uint16_t Tag = 0;
	uint32_t Offset = 0;
	uint32_t FieldSize = 0;
};
