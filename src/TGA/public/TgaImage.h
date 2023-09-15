#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "Header.h"
#include "Extensions.h"
#include "Footer.h"
#include "DeveloperDirectory.h"
#include "../../public/Pixel.h"

class TgaImage
{
public:

	TgaImage(const std::string& filename);
	~TgaImage();
	Header* GetHeader() const;
	const std::vector<Pixel>& GetPixelData() const;
	Footer* GetFooter() const;
	DeveloperDirectory* GetDeveloperDirectory() const;
	Extensions* GetExtensions() const;

	void SetPixelData(std::vector<Pixel>& newPixels);

	bool IsRightToLeftPixelOrder() const;
	bool IsTopToBottomPixelOrder() const;
	uint8_t GetAlphaChannelDepth() const;
	void SaveToFile(const std::string& filename) const; 

private:

	Header* header = nullptr;
	DeveloperDirectory* developerDirectory = nullptr;
	Extensions* extensions = nullptr;
	Footer* footer = nullptr;
	bool rightToLeftPixelOrdering = false;
	bool topToBottomPixelOrdering = false;
	uint8_t alphaChannelDepth = 0;
	std::vector<Pixel> pixelData = {};
	
	TgaImage() = delete;
	void PopulateHeader(std::ifstream& inStream);
	void PopulateFooter(std::ifstream& inStream);
	void PopulateDeveloperField(std::ifstream& inStream);
	void PopulateExtensions(std::ifstream& inStream);
	void PopulatePixelData(std::ifstream& inStream);

	void WriteHeaderToFile(std::ofstream& outFile) const;
	void WritePixelDataToFile(std::ofstream& outFile) const;
	void WriteDeveloperDirectoryToFile(std::ofstream& outFile) const;
	void WriteExtensionsToFile(std::ofstream& outFile) const;
	void WriteFooterToFile(std::ofstream& outFile) const;

	enum ImageDescriptorMask : uint8_t
	{
		AlphaDepth = 0xF,
		RightToLeftOrdering = 0x10,
		TopToBottomOrdering = 0x20
	};

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
