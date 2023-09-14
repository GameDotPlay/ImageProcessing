#pragma once

#include <cstdint>

struct Footer
{
	uint32_t ExtensionAreaOffset = 0;
	uint32_t DeveloperDirectoryOffset = 0;
	char Signature[16] = {};
	char ReservedCharacter = '.';
	char ZeroTerminator = '\0';
};
