module;

#include <fstream>
#include <memory>

module TexFile:Tga;

using namespace Tga;

TgaImage::TgaImage(const std::string& filename)
{
	std::ifstream inputFile(filename, std::ios::in | std::ios::binary);

	if (!inputFile.good())
	{
		return;
	}

	this->header = std::make_unique<Header>();
	this->PopulateHeader(inputFile, this->header);

	switch (this->header->ImageType)
	{
	case Header::EImageType::NoImageData:
		return;
	case Header::EImageType::UncompressedColorMapped:
		// Parse uncompressed color map image.
		//this->ParseColorMapped(inputFile);
		break;
	case Header::EImageType::UncompressedTrueColor:
		// Parse uncompressed true color image.
		this->ParseTrueColor(inputFile);
		break;
	case Header::EImageType::UncompressedBlackAndWhite:
		// Parse uncompressed black and white image.
		//this->ParseBlackWhite(inputFile);
		break;
	case Header::EImageType::RunLengthEncodedColorMapped:
		// Parse run length encoded color mapped image.
		//this->ParseRLEColorMapped(inputFile);
		break;
	case Header::EImageType::RunLengthEncodedTrueColor:
		// Parse run length encoded true color image.
		//this->ParseRLETrueColor(inputFile);
		break;
	case Header::EImageType::RunLengthEncodedBlackAndWhite:
		//this->ParseRLEBlackWhite(inputFile);
		// Parse run length encoded black and white image.
		break;
	default:
		return;
	}

	inputFile.close();
}

TgaImage::~TgaImage()
{

}

void TgaImage::SetPixelData(const std::shared_ptr<Vec4[]>& newPixels)
{
	this->pixelData = newPixels;
}

bool TgaImage::IsRightToLeftPixelOrder() const
{
	return this->header->ImageDescriptor & ImageDescriptorMask::RightToLeftOrdering;
}

bool TgaImage::IsTopToBottomPixelOrder() const
{
	return this->header->ImageDescriptor & ImageDescriptorMask::TopToBottomOrdering;
}

uint8_t TgaImage::GetAlphaChannelDepth() const
{
	return this->header->ImageDescriptor & ImageDescriptorMask::AlphaDepth;
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

}

void TgaImage::ParseTrueColor(std::ifstream& inStream)
{
	size_t length = this->header->Width * this->header->Height;

	this->pixelData = std::make_shared<Vec4[]>(length);
	this->PopulatePixelData(inStream, this->pixelData);

	// If eof is false after pixel data then there is probably a TGA 2.0 footer.
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
}

void TgaImage::ParseBlackWhite(std::ifstream& inStream)
{

}

void TgaImage::ParseRLEColorMapped(std::ifstream& inStream)
{

}

void TgaImage::ParseRLETrueColor(std::ifstream& inStream)
{

}

void TgaImage::ParseRLEBlackWhite(std::ifstream& inStream)
{

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

void TgaImage::PopulatePixelData(std::ifstream& inStream, std::shared_ptr<Vec4[]>& pixelData)
{
	if (!inStream.good())
	{
		return;
	}

	size_t size = this->header->Width * this->header->Height;

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	for (size_t i = 0; i < size; i++)
	{
		inStream.read((char*)&pixelData[i].z, sizeof(uint8_t));
		inStream.read((char*)&pixelData[i].y, sizeof(uint8_t));
		inStream.read((char*)&pixelData[i].x, sizeof(uint8_t));

		if (this->GetAlphaChannelDepth() != 0)
		{
			inStream.read((char*)&pixelData[i].w, sizeof(uint8_t));
		}
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
	outFile.seekp(Header::SIZE, std::ios::beg);
	size_t size = this->header->Width * this->header->Height;

	for (size_t i = 0; i < size; i++)
	{
		outFile.write((char*)&this->pixelData[i].z, sizeof(uint8_t));
		outFile.write((char*)&this->pixelData[i].y, sizeof(uint8_t));
		outFile.write((char*)&this->pixelData[i].x, sizeof(uint8_t));

		if (this->GetAlphaChannelDepth() != 0)
		{
			outFile.write((char*)&this->pixelData[i].w, sizeof(uint8_t));
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

std::shared_ptr<Vec4[]> TgaImage::GetPixelData() const
{
	return this->pixelData;
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