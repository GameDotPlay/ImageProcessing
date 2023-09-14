#include "../public/TgaImage.h"
#include "../public/DeveloperTag.h"
#include <fstream>

TgaImage::TgaImage(const std::string& filename)
{
	std::ifstream inputFile(filename, std::ios::in | std::ios::binary);

	if (inputFile.good())
	{
		std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
		buffer.shrink_to_fit();

		size_t offset = 0;
		
		this->PopulateHeader(buffer, offset);

		this->alphaChannelDepth = this->header->ImageDescriptor & ImageDescriptorMask::AlphaDepth;
		this->RightToLeftPixelOrdering = this->header->ImageDescriptor & ImageDescriptorMask::RightToLeftOrdering;
		this->TopToBottomPixelOrdering = this->header->ImageDescriptor & ImageDescriptorMask::TopToBottomOrdering;

		this->PopulatePixelBuffer(buffer, offset);
		
		this->PopulateFooter(buffer);
		
		// Only populate the developer and extension areas if the footer was valid.
		if (this->footer != nullptr)
		{
			this->PopulateDeveloperField(buffer);
			this->PopulateExtensions(buffer);
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

Header* TgaImage::GetHeader() const
{
	return this->header;
}

std::vector<Pixel>& TgaImage::GetPixelData()
{
	return this->pixelBuffer;
}

Footer* TgaImage::GetFooter() const
{
	return this->footer;
}

DeveloperDirectory* TgaImage::GetDeveloperDirectory() const
{
	return this->developerDirectory;
}

Extensions* TgaImage::GetExtensions() const
{
	return this->extensions;
}

bool TgaImage::IsRightToLeftPixelOrder() const
{
	return this->RightToLeftPixelOrdering;
}

bool TgaImage::IsTopToBottomPixelOrder() const
{
	return this->TopToBottomPixelOrdering;
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
		this->WriteFooterToFile(outFile);
	}

	outFile.close();
}

void TgaImage::PopulateHeader(const std::vector<uint8_t>& buffer, size_t& offset)
{
	this->header = new Header();

	memcpy(&this->header->IdLength, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&this->header->ColorMapType, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&this->header->ImageType, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);

	// Color Map Specification Fields. 5 bytes.
	memcpy(&this->header->FirstEntryIndex, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->header->ColorMapLength, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->header->ColorMapEntrySize, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);

	// Image Specification Fields. 10 bytes.
	memcpy(&this->header->XOrigin, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->header->YOrigin, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->header->Width, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->header->Height, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->header->PixelDepth, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&this->header->ImageDescriptor, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);
}

void TgaImage::PopulateFooter(const std::vector<uint8_t>& buffer)
{
	this->footer = new Footer();

	// Footer area always starts 26 bytes from the end. If it exists.
	size_t offset = buffer.size() - 26;
	
	memcpy(&this->footer->ExtensionAreaOffset, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->footer->DeveloperDirectoryOffset, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->footer->Signature, &buffer[offset], sizeof(this->footer->Signature));
	offset += sizeof(this->footer->Signature);
	memcpy(&this->footer->ReservedCharacter, &buffer[offset], sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&this->footer->ZeroTerminator, &buffer[offset], sizeof(uint8_t));

	// Signature string of "TRUEVISION-XFILE" should always be in bytes 8-23 of the footer area, if the footer is valid.
	std::string validSignature = "TRUEVISION-XFILE";
	if (strncmp(this->footer->Signature, validSignature.c_str(), validSignature.length()) != 0)
	{
		delete footer;
		this->footer = nullptr;
	}
}

void TgaImage::PopulateDeveloperField(const std::vector<uint8_t>& buffer)
{
	size_t offset = this->footer->DeveloperDirectoryOffset;
	if (offset == 0)
	{
		// No developer section.
		this->developerDirectory = nullptr;
		return;
	}

	this->developerDirectory = new DeveloperDirectory();
	
	memcpy(&this->developerDirectory->NumTagsInDirectory, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);

	this->developerDirectory->Tags.resize(this->developerDirectory->NumTagsInDirectory);
	this->developerDirectory->Tags.shrink_to_fit();

	for (auto& tag : this->developerDirectory->Tags)
	{
		memcpy(&tag.Tag, &buffer[offset], sizeof(uint16_t));
		offset += sizeof(uint16_t);
		memcpy(&tag.Offset, &buffer[offset], sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&tag.FieldSize, &buffer[offset], sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}
}

void TgaImage::PopulateExtensions(const std::vector<uint8_t>& buffer)
{
	size_t offset = this->footer->ExtensionAreaOffset;
	if (offset == 0)
	{
		// No extension fields.
		this->extensions = nullptr;
		return;
	}

	this->extensions = new Extensions();
	
	memcpy(&this->extensions->ExtensionSize, &buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(&this->extensions->AuthorName, &buffer[offset], sizeof(this->extensions->AuthorName));
	offset += sizeof(this->extensions->AuthorName);
	memcpy(&this->extensions->AuthorComment, &buffer[offset], sizeof(this->extensions->AuthorComment));
	offset += sizeof(this->extensions->AuthorComment);
	memcpy(&this->extensions->DateTimeStamp, &buffer[offset], sizeof(this->extensions->DateTimeStamp));
	offset += sizeof(this->extensions->DateTimeStamp);
	memcpy(&this->extensions->JobId, &buffer[offset], sizeof(this->extensions->JobId));
	offset += sizeof(this->extensions->JobId);
	memcpy(&this->extensions->JobTime, &buffer[offset], sizeof(this->extensions->JobTime));
	offset += sizeof(this->extensions->JobTime);
	memcpy(&this->extensions->SoftwareId, &buffer[offset], sizeof(this->extensions->SoftwareId));
	offset += sizeof(this->extensions->SoftwareId);
	memcpy(&this->extensions->SoftwareVersion, &buffer[offset], sizeof(this->extensions->SoftwareVersion));
	offset += sizeof(this->extensions->SoftwareVersion);
	memcpy(&this->extensions->KeyColor, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->extensions->PixelAspectRatio, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->extensions->GammaValue, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->extensions->ColorCorrectionOffset, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->extensions->PostageStampOffset, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->extensions->ScanLineOffset, &buffer[offset], sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&this->extensions->AttributesType, &buffer[offset], sizeof(uint8_t));
}

void TgaImage::PopulatePixelBuffer(const std::vector<uint8_t>& buffer, size_t& offset)
{
	this->pixelBuffer.resize(this->header->Width * this->header->Height);

	for (auto& pixel : this->pixelBuffer)
	{
		memcpy(&pixel.blue, &buffer[offset], sizeof(uint8_t));
		offset += sizeof(uint8_t);
		memcpy(&pixel.green, &buffer[offset], sizeof(uint8_t));
		offset += sizeof(uint8_t);
		memcpy(&pixel.red, &buffer[offset], sizeof(uint8_t));
		offset += sizeof(uint8_t);

		if (this->alphaChannelDepth != 0)
		{
			memcpy(&pixel.alpha, &buffer[offset], sizeof(uint8_t));
			offset += sizeof(uint8_t);
		}
	}
}

void TgaImage::WriteHeaderToFile(std::ofstream& outFile) const
{
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
	for (const auto& pixel : this->pixelBuffer)
	{
		outFile.write((char*)&pixel.blue, sizeof(uint8_t));
		outFile.write((char*)&pixel.green, sizeof(uint8_t));
		outFile.write((char*)&pixel.red, sizeof(uint8_t));

		if (this->alphaChannelDepth != 0)
		{
			outFile.write((char*)&pixel.alpha, sizeof(uint8_t));
		}
	}
}

void TgaImage::WriteDeveloperDirectoryToFile(std::ofstream& outFile) const
{
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
	outFile.write((char*)&this->footer->ExtensionAreaOffset, sizeof(uint32_t));
	outFile.write((char*)&this->footer->DeveloperDirectoryOffset, sizeof(uint32_t));
	outFile.write((char*)&this->footer->Signature, sizeof(this->footer->Signature));
	outFile.write((char*)&this->footer->ReservedCharacter, sizeof(uint8_t));
	outFile.write((char*)&this->footer->ZeroTerminator, sizeof(uint8_t));
}
