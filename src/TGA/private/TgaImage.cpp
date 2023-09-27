#include <fstream>
#include <TgaImage.h>
#include <Header.h>
#include <Footer.h>
#include <Extensions.h>
#include <DeveloperDirectory.h>
#include <DeveloperTag.h>
#include <Pixel.h>

using namespace Tga;

TgaImage::TgaImage(const std::string& filename)
{
	std::ifstream inputFile(filename, std::ios::in | std::ios::binary);

	if (!inputFile.good())
	{
		return;
	}

	this->PopulateHeader(inputFile);

	this->alphaChannelDepth = this->header->ImageDescriptor & ImageDescriptorMask::AlphaDepth;
	this->rightToLeftPixelOrdering = this->header->ImageDescriptor & ImageDescriptorMask::RightToLeftOrdering;
	this->topToBottomPixelOrdering = this->header->ImageDescriptor & ImageDescriptorMask::TopToBottomOrdering;

	// Only support uncompressed true-color images currently.
	if (this->header->ImageType != ImageType::TrueColor)
	{
		return;
	}

	this->PopulatePixelData(inputFile);

	// If eof is false after pixel data then there is probably a TGA 2.0 footer.
	if (!inputFile.eof())
	{
		this->PopulateFooter(inputFile);

		// Only populate the developer and extension areas if the footer was valid.
		if (this->footer != nullptr)
		{
			this->PopulateDeveloperField(inputFile);
			this->PopulateExtensions(inputFile);
		}
	}

	inputFile.close();
}

TgaImage::~TgaImage()
{
	if (this->header != nullptr)
	{
		delete this->header;
	}

	if (this->footer != nullptr)
	{
		delete this->footer;
	}

	if (this->developerDirectory != nullptr)
	{
		delete this->developerDirectory;
	}

	if (this->extensions != nullptr)
	{
		delete this->extensions;
	}
}

Tga::Header* TgaImage::GetHeader() const
{
	return this->header;
}

const Pixel* const TgaImage::GetPixelData() const
{
	return this->pixelData;
}

void TgaImage::SetPixelData(Pixel* const newPixels)
{
	this->pixelData = newPixels;
}

Tga::Footer* TgaImage::GetFooter() const
{
	return this->footer;
}

Tga::DeveloperDirectory* TgaImage::GetDeveloperDirectory() const
{
	return this->developerDirectory;
}

Tga::Extensions* TgaImage::GetExtensions() const
{
	return this->extensions;
}

bool TgaImage::IsRightToLeftPixelOrder() const
{
	return this->rightToLeftPixelOrdering;
}

bool TgaImage::IsTopToBottomPixelOrder() const
{
	return this->topToBottomPixelOrdering;
}

uint8_t TgaImage::GetAlphaChannelDepth() const
{
	return this->alphaChannelDepth;
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

void TgaImage::PopulateHeader(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	this->header = new Header();

	// Go to the beginning of the file.
	inStream.seekg(0, std::ios::beg);

	inStream.read((char*)&this->header->IdLength, sizeof(uint8_t));
	inStream.read((char*)&this->header->ColorMapType, sizeof(uint8_t));
	inStream.read((char*)&this->header->ImageType, sizeof(uint8_t));

	// Color Map Specification Fields. 5 bytes.
	inStream.read((char*)&this->header->FirstEntryIndex, sizeof(uint16_t));
	inStream.read((char*)&this->header->ColorMapLength, sizeof(uint16_t));
	inStream.read((char*)&this->header->ColorMapEntrySize, sizeof(uint8_t));

	// Image Specification Fields. 10 bytes.
	inStream.read((char*)&this->header->XOrigin, sizeof(uint16_t));
	inStream.read((char*)&this->header->YOrigin, sizeof(uint16_t));
	inStream.read((char*)&this->header->Width, sizeof(uint16_t));
	inStream.read((char*)&this->header->Height, sizeof(uint16_t));
	inStream.read((char*)&this->header->PixelDepth, sizeof(uint8_t));
	inStream.read((char*)&this->header->ImageDescriptor, sizeof(uint8_t));
}

void TgaImage::PopulatePixelData(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	size_t size = this->header->Width * this->header->Height;
	this->pixelData = new Pixel[size];

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	for (size_t i = 0; i < size; i++)
	{
		inStream.read((char*)&this->pixelData[i].blue, sizeof(uint8_t));
		inStream.read((char*)&this->pixelData[i].green, sizeof(uint8_t));
		inStream.read((char*)&this->pixelData[i].red, sizeof(uint8_t));

		if (this->alphaChannelDepth != 0)
		{
			inStream.read((char*)&this->pixelData[i].alpha, sizeof(uint8_t));
		}
	}
}

void TgaImage::PopulateFooter(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	this->footer = new Footer();

	// Go to 26 bytes from the end.
	inStream.seekg(-Footer::SIZE, std::ios_base::end);

	inStream.read((char*)&this->footer->ExtensionAreaOffset, sizeof(uint32_t));
	inStream.read((char*)&this->footer->DeveloperDirectoryOffset, sizeof(uint32_t));
	inStream.read((char*)&this->footer->Signature, sizeof(this->footer->Signature));
	inStream.read((char*)&this->footer->ReservedCharacter, sizeof(uint8_t));
	inStream.read((char*)&this->footer->ZeroTerminator, sizeof(uint8_t));

	// Signature string of "TRUEVISION-XFILE" should always be in bytes 8-23 of the footer area, if the footer is valid.
	std::string validSignature = "TRUEVISION-XFILE";
	if (strncmp(this->footer->Signature, validSignature.c_str(), validSignature.length()) != 0)
	{
		delete footer;
		this->footer = nullptr;
	}
}

void TgaImage::PopulateDeveloperField(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	if (this->footer == nullptr || this->footer->DeveloperDirectoryOffset == 0)
	{
		return;
	}

	this->developerDirectory = new DeveloperDirectory();

	// Go to the developer offset position.
	inStream.seekg(this->footer->DeveloperDirectoryOffset, std::ios::beg);

	inStream.read((char*)&this->developerDirectory->NumTagsInDirectory, sizeof(uint16_t));

	this->developerDirectory->Tags.resize(this->developerDirectory->NumTagsInDirectory);
	this->developerDirectory->Tags.shrink_to_fit();

	for (auto& tag : this->developerDirectory->Tags)
	{
		inStream.read((char*)&tag.Tag, sizeof(uint16_t));
		inStream.read((char*)&tag.Offset, sizeof(uint32_t));
		inStream.read((char*)&tag.FieldSize, sizeof(uint32_t));
	}
}

void TgaImage::PopulateExtensions(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	if (this->footer == nullptr || this->footer->ExtensionAreaOffset == 0)
	{
		return;
	}

	this->extensions = new Extensions();

	// Go to the extensions offset position.
	inStream.seekg(this->footer->ExtensionAreaOffset, std::ios::beg);

	inStream.read((char*)&this->extensions->ExtensionSize, sizeof(uint16_t));
	inStream.read((char*)&this->extensions->AuthorName, sizeof(this->extensions->AuthorName));
	inStream.read((char*)&this->extensions->AuthorComment, sizeof(this->extensions->AuthorComment));
	inStream.read((char*)&this->extensions->DateTimeStamp, sizeof(this->extensions->DateTimeStamp));
	inStream.read((char*)&this->extensions->JobId, sizeof(this->extensions->JobId));
	inStream.read((char*)&this->extensions->JobTime, sizeof(this->extensions->JobTime));
	inStream.read((char*)&this->extensions->SoftwareId, sizeof(this->extensions->SoftwareId));
	inStream.read((char*)&this->extensions->SoftwareVersion, sizeof(this->extensions->SoftwareVersion));
	inStream.read((char*)&this->extensions->KeyColor, sizeof(uint32_t));
	inStream.read((char*)&this->extensions->PixelAspectRatio, sizeof(uint32_t));
	inStream.read((char*)&this->extensions->GammaValue, sizeof(uint32_t));
	inStream.read((char*)&this->extensions->ColorCorrectionOffset, sizeof(uint32_t));
	inStream.read((char*)&this->extensions->PostageStampOffset, sizeof(uint32_t));
	inStream.read((char*)&this->extensions->ScanLineOffset, sizeof(uint32_t));
	inStream.read((char*)&this->extensions->AttributesType, sizeof(uint8_t));
}

void TgaImage::WriteHeaderToFile(std::ofstream& outFile) const
{
	outFile.seekp(0, std::ios::beg);

	outFile.write((char*)&this->header->IdLength, sizeof(uint8_t));
	outFile.write((char*)&this->header->ColorMapType, sizeof(uint8_t));
	outFile.write((char*)&this->header->ImageType, sizeof(uint8_t));
	outFile.write((char*)&this->header->FirstEntryIndex, sizeof(uint16_t));
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
		outFile.write((char*)&this->pixelData[i].blue, sizeof(uint8_t));
		outFile.write((char*)&this->pixelData[i].green, sizeof(uint8_t));
		outFile.write((char*)&this->pixelData[i].red, sizeof(uint8_t));

		if (this->alphaChannelDepth != 0)
		{
			outFile.write((char*)&this->pixelData[i].alpha, sizeof(uint8_t));
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
