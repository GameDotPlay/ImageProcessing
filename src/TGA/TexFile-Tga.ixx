export module TexFile:Tga;

import <string>;
import <memory>;
import <vector>;

import <Vector.h>;

namespace Tga
{
	/** Represents the header field of a TGA image. */
	struct Header
	{
		static const uint8_t SIZE = 18;

		/** Enumeration of TGA image types. */
		enum EImageType : uint8_t
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


	/** Represents a developer tag field of a TGA image. */
	struct DeveloperTag
	{
		uint16_t Tag = 0;
		uint32_t Offset = 0;
		uint32_t FieldSize = 0;
	};

	/** Represents the developer field of a TGA image. */
	struct DeveloperDirectory
	{
		uint16_t NumTagsInDirectory = 0;
		std::vector<DeveloperTag> Tags = {};
	};

	/** Represents the extensions field of a TGA image. */
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
		Header::EImageType GetImageType() const;

		/**
		 * The destructor. Releases any resources.
		 */
		~TgaImage();

		/**
		 * Get the pixel data from the TGA image.
		 */
		std::shared_ptr<Vec4[]> GetPixelData() const;

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
		void SaveToFile(const std::string& filename) const;

	private:

		/** Enumeration of TGA image descriptor masks. */
		enum EImageDescriptorMask : uint8_t
		{
			AlphaDepth = 0xF,
			RightToLeftOrdering = 0x10,
			TopToBottomOrdering = 0x20
		};

		/** The header of the TGA image. */
		std::unique_ptr<Header> header = nullptr;

		/** The developer field of the TGA image. */
		std::unique_ptr<DeveloperDirectory> developerDirectory = nullptr;

		/** The extensions field of the TGA image. */
		std::unique_ptr<Extensions> extensions = nullptr;

		/** The footer field of the TGA image. */
		std::unique_ptr<Footer> footer = nullptr;

		/** The internal pixel data stored as an array of Vec4. */
		std::shared_ptr<Vec4[]> pixelData = nullptr;

		std::shared_ptr<void> colorMap = nullptr;

		/** Default constructor not allowed. */
		TgaImage() = delete;

		void ParseColorMapped(std::ifstream& inStream);

		void ParseTrueColor(std::ifstream& inStream);

		void ParseBlackWhite(std::ifstream& inStream);

		void ParseRLEColorMapped(std::ifstream& inStream);

		void ParseRLETrueColor(std::ifstream& inStream);

		void ParseRLEBlackWhite(std::ifstream& inStream);

		/**
		 * Populate the internal header field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateHeader(std::ifstream& inStream, std::unique_ptr<Header>& header);

		/**
		 * Populate the internal footer field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateFooter(std::ifstream& inStream, std::unique_ptr<Footer>& footer);

		/**
		 * Populate the internal developer field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateDeveloperField(std::ifstream& inStream, std::unique_ptr<DeveloperDirectory>& developerDirectory);

		/**
		 * Populate the internal extensions field from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulateExtensions(std::ifstream& inStream, std::unique_ptr<Extensions>& extensions);

		/**
		 * Populate the internal pixel data from the input stream.
		 * @param inStream The input stream.
		 */
		void PopulatePixelData(std::ifstream& inStream, std::shared_ptr<Vec4[]>& pixelData);

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
		 * Write the TGA developer field to the output stream.
		 * @param outFile The output stream.
		 */
		void WriteDeveloperDirectoryToFile(std::ofstream& outFile) const;

		/**
		 * Write the TGA extensions field to the output stream.
		 * @param outFile The output stream.
		 */
		void WriteExtensionsToFile(std::ofstream& outFile) const;

		/**
		 * Write the TGA footer info to the output stream.
		 * @param outFile The output stream.
		 */
		void WriteFooterToFile(std::ofstream& outFile) const;
	};
}