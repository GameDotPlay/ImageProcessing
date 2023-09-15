#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "Header.h"
#include "Extensions.h"
#include "Footer.h"
#include "DeveloperDirectory.h"
#include "../../public/Pixel.h"

/** TgaImage class is responsible for managing a TGA file resource. */
class TgaImage
{
public:

	/**
	 * Tries to construct a TgaImage from the filename given.
	 * @param filename The path to a TGA file to load.
	 */
	TgaImage(const std::string& filename);

	/**
	 * The destructor. Releases any resources.
	 */
	~TgaImage();

	/**
	 * Get the TGA header.
	 */
	Header* GetHeader() const;

	/**
	 * Get the pixel data from the TGA image.
	 */
	const std::vector<Pixel>& GetPixelData() const;

	/**
	 * Get the TGA footer. 
	 */
	Footer* GetFooter() const;

	/**
	 * Get the developer directory from the TGA image. 
	 */
	DeveloperDirectory* GetDeveloperDirectory() const;

	/**
	 * Get the extension area info from the TGA image.
	 */
	Extensions* GetExtensions() const;

	/**
	 * Set the pixel data of the TGA image.
	 * @param newPixels The new pixel data. Must be same size as original pixel data.
	 */
	void SetPixelData(std::vector<Pixel>& newPixels);

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

	/** The header of the TGA image. */
	Header* header = nullptr;

	/** The developer field of the TGA image. */
	DeveloperDirectory* developerDirectory = nullptr;

	/** The extensions field of the TGA image. */
	Extensions* extensions = nullptr;

	/** The footer field of the TGA image. */
	Footer* footer = nullptr;

	/** The right-to-left pixel ordering. Populated from the header. */
	bool rightToLeftPixelOrdering = false;

	/** The top-to-bottom pixel ordering. Populated from the header. */
	bool topToBottomPixelOrdering = false;

	/** The alpha channel depth in bits. Populated from the header. */
	uint8_t alphaChannelDepth = 0;

	/** The internal pixel data stored as a vector of Pixels. */
	std::vector<Pixel> pixelData = {};
	
	/** Default constructor not allowed. */
	TgaImage() = delete;

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
	 * Populate the internal pixel data from the input stream.
	 * @param inStream The input stream.
	 */
	void PopulatePixelData(std::ifstream& inStream);

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

	/** enumeration of TGA image descriptor masks. */
	enum ImageDescriptorMask : uint8_t
	{
		AlphaDepth = 0xF,
		RightToLeftOrdering = 0x10,
		TopToBottomOrdering = 0x20
	};

	/** Enumeration of TGA image types. */
	enum ImageType : uint8_t
	{
		ColorMapped = 1,
		TrueColor = 2,
		BlackAndWhite = 3,
		RunLengthEncodedColorMapped = 9,
		RunLengthEncodedTrueColor = 10,
		RunLengthEncodedBlackAndWhite = 11
	};
};
