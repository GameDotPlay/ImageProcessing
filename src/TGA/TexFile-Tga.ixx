export module TexFile:Tga;

import <Vector.h>;
import <string>;
import <memory>;
import <vector>;
import <unordered_map>;

namespace Tga
{
	/** Enumeration of TGA image types. */
	export enum EImageType : uint8_t
	{
		NoImageData = 0,
		UncompressedColorMapped = 1,
		UncompressedTrueColor = 2,
		UncompressedBlackAndWhite = 3,
		RunLengthEncodedColorMapped = 9,
		RunLengthEncodedTrueColor = 10,
		RunLengthEncodedBlackAndWhite = 11
	};

	/** Fields of a TGA header. */
	struct Header
	{
		static const uint8_t SIZE = 18;

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

	/** Fields of a TGA developer tag field. */
	struct DeveloperTag
	{
		uint16_t Tag = 0;
		uint32_t Offset = 0;
		uint32_t FieldSize = 0;
	};

	/** Fields of a TGA developer directory. */
	struct DeveloperDirectory
	{
		uint16_t NumTagsInDirectory = 0;
		std::vector<DeveloperTag> Tags = {};
	};

	/** Fields of TGA extension. */
	struct Extensions
	{
		uint16_t ExtensionSize = 0;
		char AuthorName[41] = {};
		char AuthorComment[324] = {};
		char DateTimeStamp[12] = {};
		char JobId[41] = {};
		char JobTime[6] = {};
		char SoftwareId[41] = {};
		char SoftwareVersion[3] = {};
		uint32_t KeyColor = 0;
		uint32_t PixelAspectRatio = 0;
		uint32_t GammaValue = 0;
		uint32_t ColorCorrectionOffset = 0;
		uint32_t PostageStampOffset = 0;
		uint32_t ScanLineOffset = 0;
		uint8_t AttributesType = 0;
	};

	/** Fields of a TGA footer. */
	struct Footer
	{
		static const uint8_t SIZE = 26;
		static const uint8_t SIG_SIZE = 18;
		uint32_t ExtensionAreaOffset = 0;
		uint32_t DeveloperDirectoryOffset = 0;
		char Signature[16] = {};
		char ReservedCharacter = '.';
		char ZeroTerminator = '\0';
	};

	/** TgaImage class is responsible for managing a TGA file resource. */
	export class TgaImage
	{
	public:
		/**
		 * Tries to construct a TgaImage from the filename given.
		 * @param filename The path to a TGA file to load.
		 */
		TgaImage(const std::string& filename);

		/**
		 * Get the width of the image.
		 */
		uint16_t GetWidth() const;

		/**
		 * Get the height of the image.
		 */
		uint16_t GetHeight() const;

		/**
		 * Get the image type.
		 */
		EImageType GetImageType() const;

		/**
		 * The destructor. Releases any resources.
		 */
		~TgaImage();

		/**
		 * Get the raw pixel buffer from the TGA image.
		 */
		const std::shared_ptr<Vec4[]> const GetPixelBuffer() const;

		/**
		 * Set the pixel data of the TGA image.
		 * @param newPixels The new pixel data. Must be same size as original pixel data.
		 */
		void SetPixelData(const std::shared_ptr<Vec4[]>& newPixels);

		/**
		 * Indicates the right-to-left pixel ordering of the TGA image.
		 */
		bool IsRightToLeftPixelOrder() const;

		/**
		 * Indicates the top-to-bottom pixel ordering of the TGA image.
		 */
		bool IsTopToBottomPixelOrder() const;

		/**
		 * Get the alpha channel depth of the TGA image.
		 */
		uint8_t GetAlphaChannelDepth() const;

		/**
		 * Save the TGA image as a new file at the path given.
		 * @param filename The path to save the image to.
		 */
		void SaveToFile(const std::string& filename, const EImageType fileFormat);

	private:

		/** Enumeration of TGA image descriptor masks. */
		enum EImageDescriptorMask : uint8_t
		{
			AlphaDepth = 0xF,
			RightToLeftOrdering = 0x10,
			TopToBottomOrdering = 0x20
		};

		/** Enumeration of TGA run-length packet repetition count masks. */
		enum EPacketMask : uint8_t
		{
			RawPacket = 0,
			RunLengthPacket = 0x80,
			PixelCount = 0x7F
		};

		/** The header of the TGA image. */
		std::unique_ptr<Header> header = nullptr;

		/** The developer field of the TGA image. */
		std::unique_ptr<DeveloperDirectory> developerDirectory = nullptr;

		/** The extensions field of the TGA image. */
		std::unique_ptr<Extensions> extensions = nullptr;

		/** The footer field of the TGA image. */
		std::unique_ptr<Footer> footer = nullptr;

		/** Uncompressed pixel data stored as an array of Vec4. */
		std::shared_ptr<Vec4[]> pixelBuffer = nullptr;

		/** Pixels stored as an index into the colorMap. Only used if ImageType==1 (ColorMapped). */
		std::shared_ptr<uint8_t[]> colorMappedPixels = nullptr;

		/** A mapping of unique pixel values. Only used if ImageType==1 (ColorMapped). */
		std::shared_ptr<Vec4[]> colorMap = nullptr;

		/** Default constructor not allowed. */
		TgaImage() = delete;

		/**
		 * Parses an uncompressed color mapped TGA image into internal fields.
		 * @param inStream
		 */
		void ParseColorMapped(std::ifstream& inStream);

		/**
		 * Parses an uncompressed true color TGA image into internal fields.
		 * @param inStream
		 */
		void ParseTrueColor(std::ifstream& inStream);

		/**
		 * Parses an uncompressed black and white TGA image into internal fields.
		 * @param inStream
		 */
		void ParseBlackWhite(std::ifstream& inStream);

		/**
		 * Parses a run-length encoded true color TGA image into internal fields.
		 * @param inStream
		 */
		void ParseRLETrueColor(std::ifstream& inStream);

		/**
		 * Parses a run-length encoded black and white TGA image into internal fields.
		 * @param inStream
		 */
		void ParseRLEBlackWhite(std::ifstream& inStream);

		/**
		 * Populate the internal color map from file.
		 * @param inStream
		 */
		void PopulateColorMap(std::ifstream& inStream);

		/**
		 * Update the internal color mapping from the pixelBuffer.
		 */
		void UpdateColorMapping();

		/**
		 * Populate the internal header field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateHeader(std::ifstream& inStream);

		/**
		 * Populate the internal footer field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateFooter(std::ifstream& inStream);

		/**
		 * Populate the internal developer field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateDeveloperField(std::ifstream& inStream);

		/**
		 * Populate the internal extensions field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateExtensions(std::ifstream& inStream);

		/**
		 * Populate the color mapped pixel data from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateColorMappedPixels(std::ifstream& inStream);

		/**
		 * Populate the uncompressed pixel data from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulatePixelBuffer(std::ifstream& inStream);

		/**
		 * Takes a color mapping and indices into the color map and populates the raw pixel buffer.
		 * @param colorMap The color mapping.
		 */
		void PopulatePixelBuffer(const std::shared_ptr<Vec4[]>& colorMap);

		/**
		 * Write the TGA header field to the output stream.
		 * @param outFile The output stream.
		 */
		void WriteHeaderToFile(std::ofstream& outFile) const;

		/**
		 * Write the TGA pixel data to the output stream.
		 * @param outFile The output stream.
		 */
		void WritePixelDataToFile(std::ofstream& outFile) const;

		/**
		 * Writes the color map and the indices into the color map to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteColorMappedPixelDataToFile(std::ofstream& outFile) const;

		/**
		 * Writes the raw pixel buffer to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteTrueColorPixelDataToFile(std::ofstream& outFile) const;

		/**
		 * Writes the pixel data to the output stream.
		 * @param outfile The output stream to write to.
		 */
		void WriteBlackWhitePixelDataToFile(std::ofstream& outfile) const;

		/**
		 * Encodes a true color run length packet.
		 * @param i Index into the pixelBuffer data.
		 * @return An encoded run length packet.
		 */
		std::vector<uint8_t> EncodeTrueColorRunLengthPacket(size_t& i) const;

		/**
		 * Encodes a true color raw packet.
		 * @param i Index into the pixelBuffer data.
		 * @return An encoded raw packet.
		 */
		std::vector<uint8_t> EncodeTrueColorRawPacket(size_t& i) const;

		/**
		 * Encodes a black and white run length packet.
		 * @param i Index into the pixelBuffer data.
		 * @return An encoded run length packet.
		 */
		std::vector<uint8_t> EncodeBlackWhiteRunLengthPacket(size_t& i) const;

		/**
		 * Encodes a black and white raw packet.
		 * @param i Index into the pixelBuffer data.
		 * @return An encoded raw packet.
		 */
		std::vector<uint8_t> EncodeBlackWhiteRawPacket(size_t& i) const;

		/**
		 * Write encoded true color packets to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteEncodedTrueColorPixelDataToFile(std::ofstream& outFile) const;

		/**
		 * Write encoded black and white packets to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteEncodedBlackWhitePixelDataToFile(std::ofstream& outFile) const;

		/**
		 * Write the TGA developer field to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteDeveloperDirectoryToFile(std::ofstream& outFile) const;

		/**
		 * Write the TGA extensions field to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteExtensionsToFile(std::ofstream& outFile) const;

		/**
		 * Write the TGA footer info to the output stream.
		 * @param outFile The output stream to write to.
		 */
		void WriteFooterToFile(std::ofstream& outFile) const;
	};
}