#pragma once

#include <cstdint>

namespace Tga
{
	/** Represents the footer field of a TGA image. */
	struct Footer
	{
		static const uint8_t SIZE = 26;
		uint32_t ExtensionAreaOffset = 0;
		uint32_t DeveloperDirectoryOffset = 0;
		char Signature[16] = {};
		char ReservedCharacter = '.';
		char ZeroTerminator = '\0';
	};
}
