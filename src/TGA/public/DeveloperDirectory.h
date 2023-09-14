#pragma once

#include <cstdint>
#include <vector>
#include "DeveloperTag.h"

struct DeveloperDirectory
{
	uint16_t NumTagsInDirectory = 0;
	std::vector<DeveloperTag> Tags = {};
};
