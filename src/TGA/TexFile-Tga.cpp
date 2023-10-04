module;

#include <fstream>

module TexFile:Tga;

using namespace Tga;

TgaImage::TgaImage(const std::string& filename)
{
	std::ifstream inStream(filename, std::ios::in | std::ios::binary);

	if (!inStream.good())
	{
		return;
	}

	this->header = std::make_unique<Header>();
	this->PopulateHeader(inStream, this->header);

	switch (this->header->ImageType)
	{
	case Header::EImageType::NoImageData:
		inStream.close();
		return;
	case Header::EImageType::UncompressedColorMapped:
		this->ParseColorMapped(inStream);
		break;
	case Header::EImageType::UncompressedTrueColor:
		this->ParseTrueColor(inStream);
		break;
	case Header::EImageType::UncompressedBlackAndWhite:
		//this->ParseBlackWhite(inputFile);
		break;
	case Header::EImageType::RunLengthEncodedColorMapped:
		//this->ParseRLEColorMapped(inputFile);
		break;
	case Header::EImageType::RunLengthEncodedTrueColor:
		//this->ParseRLETrueColor(inputFile);
		break;
	case Header::EImageType::RunLengthEncodedBlackAndWhite:
		//this->ParseRLEBlackWhite(inputFile);
		break;
	default:
		this->header->ImageType = Header::EImageType::NoImageData;
		inStream.close();
		return;
	}

	// If eof is false after pixel data then check for a TGA 2.0 footer.
	if (!inStream.eof())
	{
		this->footer = std::make_unique<Footer>();
		this->PopulateFooter(inStream, this->footer);

		// Only populate the developer and extension areas if the footer was valid.
		if (this->footer != nullptr)
		{
			this->developerDirectory = std::make_unique<DeveloperDirectory>();
			this->PopulateDeveloperField(inStream, this->developerDirectory);

			this->extensions = std::make_unique<Extensions>();
			this->PopulateExtensions(inStream, extensions);
		}
	}

	inStream.close();
}

TgaImage::~TgaImage()
{

}

void TgaImage::SetPixelData(const std::shared_ptr<Vec4[]>& newPixels)
{
	this->pixelBuffer = newPixels;
}

bool TgaImage::IsRightToLeftPixelOrder() const
{
	return this->header->ImageDescriptor & EImageDescriptorMask::RightToLeftOrdering;
}

bool TgaImage::IsTopToBottomPixelOrder() const
{
	return this->header->ImageDescriptor & EImageDescriptorMask::TopToBottomOrdering;
}

uint8_t TgaImage::GetAlphaChannelDepth() const
{
	return this->header->ImageDescriptor & EImageDescriptorMask::AlphaDepth;
}

void TgaImage::SaveToFile(const std::string& filename) const
{
	std::ofstream outFile(filename, std::ios::out | std::ios::binary);

	if (outFile.good())
	{
		this->WriteHeaderToFile(outFile);
		this->WritePixelDataToFile(outFile);

		if (this->footer != nullptr)
		{
			this->WriteFooterToFile(outFile);

			if (this->developerDirectory != nullptr)
			{
				this->WriteDeveloperDirectoryToFile(outFile);
			}

			if (this->extensions != nullptr)
			{
				this->WriteExtensionsToFile(outFile);
			}
		}
	}

	outFile.close();
}

void TgaImage::ParseColorMapped(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	this->colorMap = std::make_shared<Vec4[]>(this->header->ColorMapLength);
	this->PopulateColorMap(inStream, this->colorMap);

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Width);
	this->colorMappedPixels = std::make_shared<uint8_t[]>(pixelsLength);
	this->PopulatePixelData(inStream, this->colorMappedPixels);

	this->pixelBuffer = std::make_shared<Vec4[]>(pixelsLength);
	this->PopulatePixelData(this->colorMap, this->colorMappedPixels, this->pixelBuffer);
}

void TgaImage::PopulateColorMap(std::ifstream& inStream, const std::shared_ptr<Vec4[]>& colorMap)
{
	if (!inStream.good())
	{
		return;
	}

	// Go to start of color map entries.
	inStream.seekg(Header::SIZE + this->header->ColorMapFirstEntryIndex, std::ios::beg);

	for (size_t i = 0; i < this->header->ColorMapLength; i++)
	{
		inStream.read((char*)&colorMap[i].z, sizeof(uint8_t));
		inStream.read((char*)&colorMap[i].y, sizeof(uint8_t));
		inStream.read((char*)&colorMap[i].x, sizeof(uint8_t));

		if (this->header->ColorMapEntrySize == 32) // 32 bits
		{
			inStream.read((char*)&this->colorMap[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::ParseTrueColor(std::ifstream& inStream)
{
	size_t length = (size_t)(this->header->Width * this->header->Height);

	this->pixelBuffer = std::make_shared<Vec4[]>(length);
	this->PopulatePixelData(inStream, this->pixelBuffer);
}

void TgaImage::ParseBlackWhite(std::ifstream& inStream)
{
	// TODO
}

void TgaImage::ParseRLEColorMapped(std::ifstream& inStream)
{
	// TODO
}

void TgaImage::ParseRLETrueColor(std::ifstream& inStream)
{
	// TODO
}

void TgaImage::ParseRLEBlackWhite(std::ifstream& inStream)
{
	// TODO
}

void TgaImage::PopulateHeader(std::ifstream& inStream, std::unique_ptr<Header>& header)
{
	if (!inStream.good())
	{
		return;
	}

	// Go to the beginning of the file.
	inStream.seekg(0, std::ios::beg);

	inStream.read((char*)&header->IdLength, sizeof(uint8_t));
	inStream.read((char*)&header->ColorMapType, sizeof(uint8_t));
	inStream.read((char*)&header->ImageType, sizeof(uint8_t));

	// Color Map Specification Fields. 5 bytes.
	inStream.read((char*)&header->ColorMapFirstEntryIndex, sizeof(uint16_t));
	inStream.read((char*)&header->ColorMapLength, sizeof(uint16_t));
	inStream.read((char*)&header->ColorMapEntrySize, sizeof(uint8_t));

	// Image Specification Fields. 10 bytes.
	inStream.read((char*)&header->XOrigin, sizeof(uint16_t));
	inStream.read((char*)&header->YOrigin, sizeof(uint16_t));
	inStream.read((char*)&header->Width, sizeof(uint16_t));
	inStream.read((char*)&header->Height, sizeof(uint16_t));
	inStream.read((char*)&header->PixelDepth, sizeof(uint8_t));
	inStream.read((char*)&header->ImageDescriptor, sizeof(uint8_t));
}

void TgaImage::PopulatePixelData(std::ifstream& inStream, const std::shared_ptr<Vec4[]>& pixelBuffer)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		inStream.read((char*)&pixelBuffer[i].z, sizeof(uint8_t));
		inStream.read((char*)&pixelBuffer[i].y, sizeof(uint8_t));
		inStream.read((char*)&pixelBuffer[i].x, sizeof(uint8_t));

		if (this->GetAlphaChannelDepth() != 0) // 32 bit pixels
		{
			inStream.read((char*)&pixelBuffer[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::PopulatePixelData(std::ifstream& inStream, const std::shared_ptr<uint8_t[]>& colorMappedPixels)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);	
	size_t colorMapLengthBytes = (size_t)this->header->ColorMapLength * (this->header->ColorMapEntrySize / 8);

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE + this->header->ColorMapFirstEntryIndex + colorMapLengthBytes, std::ios::beg);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		inStream.read((char*)&colorMappedPixels[i], sizeof(uint8_t));
	}
}

void TgaImage::PopulatePixelData(const std::shared_ptr<Vec4[]>& colorMap, const std::shared_ptr<uint8_t[]>& colorMappedPixels, const std::shared_ptr<Vec4[]>& pixelBuffer)
{
	size_t pixelsLength = (size_t)this->header->Width * this->header->Height;

	for (size_t i = 0; i < pixelsLength; i++)
	{
		pixelBuffer[i] = colorMap[colorMappedPixels[i]];
	}
}

void TgaImage::PopulateFooter(std::ifstream& inStream, std::unique_ptr<Footer>& footer)
{
	if (!inStream.good())
	{
		return;
	}

	// Go to 26 bytes from the end.
	inStream.seekg(-Footer::SIZE, std::ios_base::end);

	inStream.read((char*)&footer->ExtensionAreaOffset, sizeof(uint32_t));
	inStream.read((char*)&footer->DeveloperDirectoryOffset, sizeof(uint32_t));
	inStream.read((char*)&footer->Signature, sizeof(this->footer->Signature));
	inStream.read((char*)&footer->ReservedCharacter, sizeof(uint8_t));
	inStream.read((char*)&footer->ZeroTerminator, sizeof(uint8_t));

	// Signature string of "TRUEVISION-XFILE" should always be in bytes 8-23 of the footer area, if the footer is valid.
	std::string validSignature = "TRUEVISION-XFILE";
	if (strncmp(footer->Signature, validSignature.c_str(), validSignature.length()) != 0)
	{
		footer = nullptr;
	}
}

void TgaImage::PopulateDeveloperField(std::ifstream& inStream, std::unique_ptr<DeveloperDirectory>& developerDirectory)
{
	if (!inStream.good())
	{
		return;
	}

	// Go to the developer offset position.
	inStream.seekg(this->footer->DeveloperDirectoryOffset, std::ios::beg);

	inStream.read((char*)&developerDirectory->NumTagsInDirectory, sizeof(uint16_t));

	developerDirectory->Tags.resize(developerDirectory->NumTagsInDirectory);
	developerDirectory->Tags.shrink_to_fit();

	for (auto& tag : developerDirectory->Tags)
	{
		inStream.read((char*)&tag.Tag, sizeof(uint16_t));
		inStream.read((char*)&tag.Offset, sizeof(uint32_t));
		inStream.read((char*)&tag.FieldSize, sizeof(uint32_t));
	}
}

void TgaImage::PopulateExtensions(std::ifstream& inStream, std::unique_ptr<Extensions>& extensions)
{
	if (!inStream.good())
	{
		return;
	}

	// Go to the extensions offset position.
	inStream.seekg(this->footer->ExtensionAreaOffset, std::ios::beg);

	inStream.read((char*)&extensions->ExtensionSize, sizeof(uint16_t));
	inStream.read((char*)&extensions->AuthorName, sizeof(this->extensions->AuthorName));
	inStream.read((char*)&extensions->AuthorComment, sizeof(this->extensions->AuthorComment));
	inStream.read((char*)&extensions->DateTimeStamp, sizeof(this->extensions->DateTimeStamp));
	inStream.read((char*)&extensions->JobId, sizeof(this->extensions->JobId));
	inStream.read((char*)&extensions->JobTime, sizeof(this->extensions->JobTime));
	inStream.read((char*)&extensions->SoftwareId, sizeof(this->extensions->SoftwareId));
	inStream.read((char*)&extensions->SoftwareVersion, sizeof(this->extensions->SoftwareVersion));
	inStream.read((char*)&extensions->KeyColor, sizeof(uint32_t));
	inStream.read((char*)&extensions->PixelAspectRatio, sizeof(uint32_t));
	inStream.read((char*)&extensions->GammaValue, sizeof(uint32_t));
	inStream.read((char*)&extensions->ColorCorrectionOffset, sizeof(uint32_t));
	inStream.read((char*)&extensions->PostageStampOffset, sizeof(uint32_t));
	inStream.read((char*)&extensions->ScanLineOffset, sizeof(uint32_t));
	inStream.read((char*)&extensions->AttributesType, sizeof(uint8_t));
}

void TgaImage::WriteHeaderToFile(std::ofstream& outFile) const
{
	outFile.seekp(0, std::ios::beg);

	outFile.write((char*)&this->header->IdLength, sizeof(uint8_t));
	outFile.write((char*)&this->header->ColorMapType, sizeof(uint8_t));
	outFile.write((char*)&this->header->ImageType, sizeof(uint8_t));
	outFile.write((char*)&this->header->ColorMapFirstEntryIndex, sizeof(uint16_t));
	outFile.write((char*)&this->header->ColorMapLength, sizeof(uint16_t));
	outFile.write((char*)&this->header->ColorMapEntrySize, sizeof(uint8_t));
	outFile.write((char*)&this->header->XOrigin, sizeof(uint16_t));
	outFile.write((char*)&this->header->YOrigin, sizeof(uint16_t));
	outFile.write((char*)&this->header->Width, sizeof(uint16_t));
	outFile.write((char*)&this->header->Height, sizeof(uint16_t));
	outFile.write((char*)&this->header->PixelDepth, sizeof(uint8_t));
	outFile.write((char*)&this->header->ImageDescriptor, sizeof(uint8_t));
}

void TgaImage::WritePixelDataToFile(std::ofstream& outFile) const
{
	switch (this->header->ImageType)
	{
	case Header::EImageType::NoImageData:
		return;
	case Header::EImageType::UncompressedColorMapped:
		this->WriteColorMappedPixelDataToFile(outFile);
		break;
	case Header::EImageType::UncompressedTrueColor:
		this->WriteTrueColorPixelDataToFile(outFile);
		break;
	case Header::EImageType::UncompressedBlackAndWhite:
		
		break;
	case Header::EImageType::RunLengthEncodedColorMapped:
		
		break;
	case Header::EImageType::RunLengthEncodedTrueColor:
		
		break;
	case Header::EImageType::RunLengthEncodedBlackAndWhite:
		
		break;
	default:
		
		return;
	}
}

void TgaImage::WriteColorMappedPixelDataToFile(std::ofstream& outFile) const
{
	outFile.seekp(Header::SIZE, std::ios::beg);
	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);

	for (size_t i = 0; i < this->header->ColorMapLength; i++)
	{
		outFile.write((char*)&this->colorMap[i].z, sizeof(uint8_t));
		outFile.write((char*)&this->colorMap[i].y, sizeof(uint8_t));
		outFile.write((char*)&this->colorMap[i].x, sizeof(uint8_t));

		if (this->GetAlphaChannelDepth() != 0)
		{
			outFile.write((char*)&this->colorMap[i].w, sizeof(uint8_t));
		}
	}

	for (size_t i = 0; i < pixelsLength; i++)
	{
		outFile.write((char*)&this->colorMappedPixels[i], sizeof(uint8_t));
	}
}

void TgaImage::WriteTrueColorPixelDataToFile(std::ofstream& outFile) const
{
	outFile.seekp(Header::SIZE, std::ios::beg);
	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		outFile.write((char*)&this->pixelBuffer[i].z, sizeof(uint8_t));
		outFile.write((char*)&this->pixelBuffer[i].y, sizeof(uint8_t));
		outFile.write((char*)&this->pixelBuffer[i].x, sizeof(uint8_t));

		if (this->GetAlphaChannelDepth() != 0)
		{
			outFile.write((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::WriteDeveloperDirectoryToFile(std::ofstream& outFile) const
{
	outFile.seekp(this->footer->DeveloperDirectoryOffset, std::ios::beg);

	outFile.write((char*)&this->developerDirectory->NumTagsInDirectory, sizeof(uint16_t));

	for (const auto& tag : this->developerDirectory->Tags)
	{
		outFile.write((char*)&tag.Tag, sizeof(uint16_t));
		outFile.write((char*)&tag.Offset, sizeof(uint32_t));
		outFile.write((char*)&tag.FieldSize, sizeof(uint32_t));
	}
}

void TgaImage::WriteExtensionsToFile(std::ofstream& outFile) const
{
	outFile.seekp(this->footer->ExtensionAreaOffset, std::ios::beg);

	outFile.write((char*)&this->extensions->ExtensionSize, sizeof(uint16_t));
	outFile.write((char*)&this->extensions->AuthorName, sizeof(this->extensions->AuthorName));
	outFile.write((char*)&this->extensions->AuthorComment, sizeof(this->extensions->AuthorComment));
	outFile.write((char*)&this->extensions->DateTimeStamp, sizeof(this->extensions->DateTimeStamp));
	outFile.write((char*)&this->extensions->JobId, sizeof(this->extensions->JobId));
	outFile.write((char*)&this->extensions->JobTime, sizeof(this->extensions->JobTime));
	outFile.write((char*)&this->extensions->SoftwareId, sizeof(this->extensions->SoftwareId));
	outFile.write((char*)&this->extensions->SoftwareVersion, sizeof(this->extensions->SoftwareVersion));
	outFile.write((char*)&this->extensions->KeyColor, sizeof(uint32_t));
	outFile.write((char*)&this->extensions->PixelAspectRatio, sizeof(uint32_t));
	outFile.write((char*)&this->extensions->GammaValue, sizeof(uint32_t));
	outFile.write((char*)&this->extensions->ColorCorrectionOffset, sizeof(uint32_t));
	outFile.write((char*)&this->extensions->PostageStampOffset, sizeof(uint32_t));
	outFile.write((char*)&this->extensions->ScanLineOffset, sizeof(uint32_t));
	outFile.write((char*)&this->extensions->AttributesType, sizeof(uint8_t));
}

void TgaImage::WriteFooterToFile(std::ofstream& outFile) const
{
	outFile.seekp(-Footer::SIZE, std::ios_base::end);

	outFile.write((char*)&this->footer->ExtensionAreaOffset, sizeof(uint32_t));
	outFile.write((char*)&this->footer->DeveloperDirectoryOffset, sizeof(uint32_t));
	outFile.write((char*)&this->footer->Signature, sizeof(this->footer->Signature));
	outFile.write((char*)&this->footer->ReservedCharacter, sizeof(uint8_t));
	outFile.write((char*)&this->footer->ZeroTerminator, sizeof(uint8_t));
}

const std::shared_ptr<Vec4[]> const TgaImage::GetPixelData() const
{
	return this->pixelBuffer;
}

uint16_t TgaImage::GetWidth() const
{
	return this->header->Width;
}

uint16_t TgaImage::GetHeight() const
{
	return this->header->Height;
}

Header::EImageType TgaImage::GetImageType() const
{
	return this->header->ImageType;
}