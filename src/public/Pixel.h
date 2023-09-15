#pragma once

#include <cstdint>

/** Represents a pixel in a TGA image.
 *  If the image has no alpha channel then the alpha field is ignored.
 */
struct Pixel
{
	uint8_t alpha = 0;
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
};
