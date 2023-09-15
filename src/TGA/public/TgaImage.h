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
	std::vector<Pixel>& GetPixelData();
	void SetPixelData(std::vector<Pixel>& newPixels);
	Footer* GetFooter() const;
	DeveloperDirectory* GetDeveloperDirectory() const;
	Extensions* GetExtensions() const;
	bool IsRightToLeftPixelOrder() const;
	bool IsTopToBottomPixelOrder() const;
	uint8_t GetAlphaChannelDepth() const;

	void SaveToFile(const std::string& filename) const; 

private:

	Header* header = nullptr;
	DeveloperDirectory* developerDirectory = nullptr;
	Extensions* extensions = nullptr;
	Footer* footer = nullptr;
	bool RightToLeftPixelOrdering = false;
	bool TopToBottomPixelOrdering = false;
	uint8_t alphaChannelDepth = 0;
	std::vector<Pixel> pixelData = {};
	
	void PopulateHeader(const std::vector<uint8_t>& buffer);
	void PopulateFooter(const std::vector<uint8_t>& buffer);
	void PopulateDeveloperField(const std::vector<uint8_t>& buffer);
	void PopulateExtensions(const std::vector<uint8_t>& buffer);
	void PopulatePixelData(const std::vector<uint8_t>& buffer);

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
