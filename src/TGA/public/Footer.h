#pragma once

#include <cstdint>

struct Footer
{
	static const uint8_t SIZE = 26;
	uint32_t ExtensionAreaOffset = 0;
	uint32_t DeveloperDirectoryOffset = 0;
	char Signature[16] = {};
	char ReservedCharacter = '.';
	char ZeroTerminator = '\0';
};
