#pragma once

#include <cstdint>
#include <vector>
#include "DeveloperTag.h"

namespace Tga
{
	/** Represents the developer field of a TGA image. */
	struct DeveloperDirectory
	{
		uint16_t NumTagsInDirectory = 0;
		std::vector<DeveloperTag> Tags = {};
	};
}
