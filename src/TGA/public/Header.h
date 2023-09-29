#include <cstdint>
#include <iostream>

namespace Tga
{
	/** Represents the header field of a TGA image. */
	struct Header
	{
		static const uint8_t SIZE = 18;

		uint8_t IdLength = 0;
		uint8_t ColorMapType = 0;
		uint8_t ImageType = 0;

		uint16_t ColorMapFirstEntryIndex = 0;
		uint16_t ColorMapLength = 0;
		uint8_t ColorMapEntrySize = 0;

		uint16_t XOrigin = 0;
		uint16_t YOrigin = 0;
		uint16_t Width = 0;
		uint16_t Height = 0;
		uint8_t PixelDepth = 0;
		uint8_t ImageDescriptor = 0;

		/**
		 * Prints the values of all internal fields to the console.
		 */
		void PrintToConsole()
		{
			std::cout << "Id Length: " << (unsigned int)this->IdLength << std::endl;
			std::cout << "Color Map Type: " << (unsigned int)this->ColorMapType << std::endl;
			std::cout << "Image Type: " << (unsigned int)this->ImageType << std::endl;
			std::cout << "First Entry Index: " << (unsigned int)this->ColorMapFirstEntryIndex << std::endl;
			std::cout << "Color Map Length: " << (unsigned int)this->ColorMapLength << std::endl;
			std::cout << "Color Map Entry Size: " << (unsigned int)this->ColorMapEntrySize << std::endl;
			std::cout << "X Origin: " << (unsigned int)this->XOrigin << std::endl;
			std::cout << "Y Origin: " << (unsigned int)this->YOrigin << std::endl;
			std::cout << "Width: " << (unsigned int)this->Width << std::endl;
			std::cout << "Height: " << (unsigned int)this->Height << std::endl;
			std::cout << "Pixel Depth: " << (unsigned int)this->PixelDepth << std::endl;
			std::cout << "Image Descriptor: " << (unsigned int)this->ImageDescriptor << std::endl;
		}
	};
}
