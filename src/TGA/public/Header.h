#include <cstdint>

namespace Tga
{
	/** Represents the header field of a TGA image. */
	struct Header
	{
		static const uint8_t SIZE = 18;

		/** Enumeration of TGA image types. */
		static enum EImageType : uint8_t
		{
			NoImageData = 0,
			UncompressedColorMapped = 1,
			UncompressedTrueColor = 2,
			UncompressedBlackAndWhite = 3,
			RunLengthEncodedColorMapped = 9,
			RunLengthEncodedTrueColor = 10,
			RunLengthEncodedBlackAndWhite = 11
		};

		uint8_t IdLength = 0;
		uint8_t ColorMapType = 0;
		EImageType ImageType = EImageType::NoImageData;

		uint16_t ColorMapFirstEntryIndex = 0;
		uint16_t ColorMapLength = 0;
		uint8_t ColorMapEntrySize = 0;

		uint16_t XOrigin = 0;
		uint16_t YOrigin = 0;
		uint16_t Width = 0;
		uint16_t Height = 0;
		uint8_t PixelDepth = 0;
		uint8_t ImageDescriptor = 0;
	};
}
