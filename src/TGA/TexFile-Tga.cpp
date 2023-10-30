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

	this->PopulateHeader(inStream);

	switch (this->header->ImageType)
	{
	case EImageType::NoImageData:
		break;

	case EImageType::UncompressedColorMapped:
		this->ParseColorMapped(inStream);
		break;

	case EImageType::UncompressedTrueColor:
		this->ParseTrueColor(inStream);
		break;

	case EImageType::UncompressedBlackAndWhite:
		this->ParseBlackWhite(inStream);
		break;

	case EImageType::RunLengthEncodedColorMapped:
		// Will not implement.
		break;

	case EImageType::RunLengthEncodedTrueColor:
		this->ParseRLETrueColor(inStream);
		break;

	case EImageType::RunLengthEncodedBlackAndWhite:
		this->ParseRLEBlackWhite(inStream);
		break;

	default:
		this->header->ImageType = EImageType::NoImageData;
		break;
	}

	// Check for a TGA 2.0 footer.
	this->PopulateFooter(inStream);
	this->PopulateDeveloperField(inStream);
	this->PopulateExtensions(inStream);

	inStream.close();
}

TgaImage::~TgaImage() { }

void TgaImage::SetPixelData(std::unique_ptr<Vec4[]> newPixels)
{
	this->pixelBuffer = std::move(newPixels);
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

void TgaImage::SaveToFile(const std::string& filename, const EImageType fileFormat)
{
	std::ofstream outFile(filename, std::ios::out | std::ios::binary);

	if (!outFile.good())
	{
		return;
	}

	this->header->ImageType = fileFormat;

	if (this->header->ImageType == EImageType::UncompressedColorMapped)
	{
		this->UpdateColorMapping();
	}

	this->WriteHeaderToFile(outFile);
	this->WritePixelDataToFile(outFile);
	this->WriteDeveloperDirectoryToFile(outFile);
	this->WriteExtensionsToFile(outFile);
	this->WriteFooterToFile(outFile);
	
	outFile.close();
}

void TgaImage::ParseColorMapped(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	// Color mapped images only allowed 256 colors, and one-byte pixel indices into the color map.
	if (this->header->ColorMapLength <= 256 && this->header->PixelDepth == 8)
	{
		this->PopulateColorMap(inStream);
		this->PopulateColorMappedPixels(inStream);
		this->PopulatePixelBuffer(this->colorMap);
	}
}

void TgaImage::PopulateColorMap(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	this->colorMap = std::make_shared<Vec4[]>(this->header->ColorMapLength);

	// Go to start of color map entries.
	inStream.seekg((size_t)Header::SIZE + this->header->ColorMapFirstEntryIndex, std::ios::beg);

	for (size_t i = 0; i < this->header->ColorMapLength; i++)
	{
		inStream.read((char*)&this->colorMap[i].z, sizeof(uint8_t));
		inStream.read((char*)&this->colorMap[i].y, sizeof(uint8_t));
		inStream.read((char*)&this->colorMap[i].x, sizeof(uint8_t));

		if (this->header->ColorMapEntrySize == 32) // 32 bits
		{
			inStream.read((char*)&this->colorMap[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::ParseTrueColor(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	this->PopulatePixelBuffer(inStream);
}

void TgaImage::ParseBlackWhite(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);
	this->pixelBuffer = std::make_shared<Vec4[]>(pixelsLength);

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		inStream.read((char*)&this->pixelBuffer[i].x, sizeof(uint8_t));

		if (this->header->PixelDepth == 16 && this->GetAlphaChannelDepth() == 8)
		{
			inStream.read((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::ParseRLETrueColor(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);
	this->pixelBuffer = std::make_shared<Vec4[]>(pixelsLength);

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	size_t i = 0;
	while (i < pixelsLength)
	{
		uint8_t packet = 0;
		inStream.read((char*)&packet, sizeof(uint8_t));

		uint8_t pixelCount = (packet & EPacketMask::PixelCount) + 1;

		if (packet & EPacketMask::RunLengthPacket)
		{
			Vec4 pixelValue = {};
			inStream.read((char*)&pixelValue.z, sizeof(uint8_t));
			inStream.read((char*)&pixelValue.y, sizeof(uint8_t));
			inStream.read((char*)&pixelValue.x, sizeof(uint8_t));

			if (this->GetAlphaChannelDepth() == 8) // 32 bit pixels
			{
				inStream.read((char*)&pixelValue.w, sizeof(uint8_t));
			}

			// Run length packet.
			for (size_t j = 0; j < pixelCount; j++)
			{
				this->pixelBuffer[i] = pixelValue;
				i++;
			}
		}
		else
		{
			// Raw packet.
			for (size_t j = 0; j < pixelCount; j++)
			{
				inStream.read((char*)&this->pixelBuffer[i].z, sizeof(uint8_t));
				inStream.read((char*)&this->pixelBuffer[i].y, sizeof(uint8_t));
				inStream.read((char*)&this->pixelBuffer[i].x, sizeof(uint8_t));

				if (this->GetAlphaChannelDepth() == 8) // 32 bit pixels
				{
					inStream.read((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
				}

				i++;
			}
		}
	}
}

void TgaImage::ParseRLEBlackWhite(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);
	this->pixelBuffer = std::make_shared<Vec4[]>(pixelsLength);

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	size_t i = 0;
	while (i < pixelsLength)
	{
		uint8_t packet = 0;
		inStream.read((char*)&packet, sizeof(uint8_t));

		uint8_t pixelCount = (packet & EPacketMask::PixelCount) + 1;

		if (packet & EPacketMask::RunLengthPacket)
		{
			Vec4 pixelValue = {};
			inStream.read((char*)&pixelValue.x, sizeof(uint8_t));

			if (this->GetAlphaChannelDepth() == 8) // 32 bit pixels
			{
				inStream.read((char*)&pixelValue.w, sizeof(uint8_t));
			}

			// Run length packet.
			for (size_t j = 0; j < pixelCount; j++)
			{
				this->pixelBuffer[i] = pixelValue;
				i++;
			}
		}
		else
		{
			// Raw packet.
			for (size_t j = 0; j < pixelCount; j++)
			{
				inStream.read((char*)&this->pixelBuffer[i].x, sizeof(uint8_t));

				if (this->GetAlphaChannelDepth() == 8) // 32 bit pixels
				{
					inStream.read((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
				}

				i++;
			}
		}
	}
}

void TgaImage::PopulateHeader(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	this->header = std::make_unique<Header>();

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

void TgaImage::PopulatePixelBuffer(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);
	this->pixelBuffer = std::make_shared<Vec4[]>(pixelsLength);

	// Go to the pixel data position.
	inStream.seekg(Header::SIZE, std::ios::beg);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		inStream.read((char*)&this->pixelBuffer[i].z, sizeof(uint8_t));
		inStream.read((char*)&this->pixelBuffer[i].y, sizeof(uint8_t));
		inStream.read((char*)&this->pixelBuffer[i].x, sizeof(uint8_t));

		if (this->GetAlphaChannelDepth() == 8) // 32 bit pixels
		{
			inStream.read((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::PopulateColorMappedPixels(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	size_t pixelsLength = (size_t)this->header->Width * this->header->Height;
	size_t colorMapLengthBytes = (size_t)this->header->ColorMapLength * (this->header->ColorMapEntrySize / 8);

	// Go to the pixel data position.
	inStream.seekg((size_t)Header::SIZE + this->header->ColorMapFirstEntryIndex + colorMapLengthBytes, std::ios::beg);

	this->colorMappedPixels = std::make_shared<uint8_t[]>(pixelsLength);
	for (size_t i = 0; i < pixelsLength; i++)
	{
		inStream.read((char*)&this->colorMappedPixels[i], sizeof(uint8_t));
	}
}

void TgaImage::PopulatePixelBuffer(const std::shared_ptr<Vec4[]>& colorMap)
{
	size_t pixelsLength = (size_t)this->header->Width * this->header->Height;
	this->pixelBuffer = std::make_shared<Vec4[]>(pixelsLength);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		this->pixelBuffer[i] = this->colorMap[this->colorMappedPixels[i]];
	}
}

void TgaImage::PopulateFooter(std::ifstream& inStream)
{
	if (!inStream.good())
	{
		return;
	}

	// Go to 18 bytes from the end.
	inStream.seekg(-Footer::SIG_SIZE, std::ios_base::end);

	// Signature string of "TRUEVISION-XFILE" should always be in bytes 8-23 of the footer area, if the footer is valid.
	std::string validSignature = "TRUEVISION-XFILE";
	char sig[Footer::SIG_SIZE];
	inStream.read(sig, Footer::SIG_SIZE);

	if (validSignature.compare(0, validSignature.length(), sig) == 0)
	{
		this->footer = std::make_unique<Footer>();

		// Go to 26 bytes from the end.
		inStream.seekg(-Footer::SIZE, std::ios_base::end);

		inStream.read((char*)&this->footer->ExtensionAreaOffset, sizeof(uint32_t));
		inStream.read((char*)&this->footer->DeveloperDirectoryOffset, sizeof(uint32_t));
		inStream.read((char*)&this->footer->Signature, sizeof(this->footer->Signature));
		inStream.read((char*)&this->footer->ReservedCharacter, sizeof(uint8_t));
		inStream.read((char*)&this->footer->ZeroTerminator, sizeof(uint8_t));
	}
}

void TgaImage::PopulateDeveloperField(std::ifstream& inStream)
{
	if (!inStream.good() || this->footer == nullptr || this->footer->DeveloperDirectoryOffset == 0)
	{
		return;
	}

	this->developerDirectory = std::make_unique<DeveloperDirectory>();

	// Go to the developer directory position.
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
	if (!inStream.good() || this->footer == nullptr || this->footer->ExtensionAreaOffset == 0)
	{
		return;
	}

	this->extensions = std::make_unique<Extensions>();

	/// Go to the extensions position.
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

void TgaImage::UpdateColorMapping()
{
	this->header->ColorMapEntrySize = this->header->PixelDepth;
	this->header->ColorMapFirstEntryIndex = 0;
	this->header->ColorMapType = 1;

	size_t pixelsLength = (size_t)this->header->Width * this->header->Height;
	std::unordered_map<Vec4, size_t> newMap;

	size_t j = 0;
	for (size_t i = 0; i < pixelsLength; i++)
	{
		if (newMap.count(this->pixelBuffer[i]) == 0)
		{
			newMap[this->pixelBuffer[i]] = j;
			j++;
		}
	}

	this->colorMap.reset();
	this->colorMap = std::make_shared<Vec4[]>(newMap.size());

	for (const auto& entry : newMap)
	{
		this->colorMap[entry.second] = entry.first;
	}

	this->header->ColorMapLength = newMap.size();

	for (size_t i = 0; i < pixelsLength; i++)
	{
		this->colorMappedPixels[i] = newMap[this->pixelBuffer[i]];
	}
}

void TgaImage::WriteHeaderToFile(std::ofstream& outFile) const
{
	if (!outFile.good())
	{
		return;
	}

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
	case EImageType::NoImageData:
		break;

	case EImageType::UncompressedColorMapped:
		this->WriteColorMappedPixelDataToFile(outFile);
		break;

	case EImageType::UncompressedTrueColor:
		this->WriteTrueColorPixelDataToFile(outFile);
		break;

	case EImageType::UncompressedBlackAndWhite:
		this->WriteBlackWhitePixelDataToFile(outFile);
		break;

	case EImageType::RunLengthEncodedColorMapped:
		// Will not implement.
		break;

	case EImageType::RunLengthEncodedTrueColor:
		this->WriteEncodedTrueColorPixelDataToFile(outFile);
		break;

	case EImageType::RunLengthEncodedBlackAndWhite:
		this->WriteEncodedBlackWhitePixelDataToFile(outFile);
		break;

	default:
		break;
	}
}

void TgaImage::WriteColorMappedPixelDataToFile(std::ofstream& outFile) const
{
	outFile.seekp((size_t)Header::SIZE + this->header->ColorMapFirstEntryIndex, std::ios::beg);
	size_t pixelsLength = (size_t)this->header->Width * this->header->Height;

	for (size_t i = 0; i < this->header->ColorMapLength; i++)
	{
		outFile.write((char*)&this->colorMap[i].z, sizeof(uint8_t));
		outFile.write((char*)&this->colorMap[i].y, sizeof(uint8_t));
		outFile.write((char*)&this->colorMap[i].x, sizeof(uint8_t));

		if (this->header->ColorMapEntrySize == 32) // 32 bits
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

		if (this->GetAlphaChannelDepth() == 8)
		{
			outFile.write((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::WriteBlackWhitePixelDataToFile(std::ofstream& outFile) const
{
	outFile.seekp(Header::SIZE, std::ios::beg);
	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);

	for (size_t i = 0; i < pixelsLength; i++)
	{
		outFile.write((char*)&this->pixelBuffer[i].x, sizeof(uint8_t));

		if (this->header->PixelDepth == 16 && this->GetAlphaChannelDepth() == 8)
		{
			outFile.write((char*)&this->pixelBuffer[i].w, sizeof(uint8_t));
		}
	}
}

void TgaImage::WriteEncodedTrueColorPixelDataToFile(std::ofstream& outFile) const
{
	outFile.seekp(Header::SIZE, std::ios::beg);
	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);

	size_t i = 0;
	while (i < pixelsLength)
	{
		std::vector<uint8_t> packet = {};
		if (this->pixelBuffer[i] == this->pixelBuffer[i + 1])
		{
			packet = this->EncodeTrueColorRunLengthPacket(i);
		}
		else
		{
			packet = this->EncodeTrueColorRawPacket(i);
		}

		outFile.write((char*)packet.data(), packet.size());
	}
}

void TgaImage::WriteEncodedBlackWhitePixelDataToFile(std::ofstream& outFile) const
{
	outFile.seekp(Header::SIZE, std::ios::beg);
	size_t pixelsLength = (size_t)(this->header->Width * this->header->Height);

	size_t i = 0;
	while (i < pixelsLength)
	{
		std::vector<uint8_t> packet = {};
		if (this->pixelBuffer[i] == this->pixelBuffer[i + 1])
		{
			packet = this->EncodeBlackWhiteRunLengthPacket(i);
		}
		else
		{
			packet = this->EncodeBlackWhiteRawPacket(i);
		}

		outFile.write((char*)packet.data(), packet.size());
	}
}

std::vector<uint8_t> TgaImage::EncodeTrueColorRunLengthPacket(size_t& i) const
{
	std::vector<uint8_t> packet = {};
	uint8_t runLengthBit = EPacketMask::RunLengthPacket;
	uint8_t count = 0;
	uint8_t j = 1;

	while (count < 127 && (this->pixelBuffer[i] == this->pixelBuffer[i + j]))
	{
		count++;
		j++;

		if ((i + j) % (this->header->Width - 1) == 0)
		{
			// Packets should not cross scan lines. TGA 2.0 spec.
			break;
		}
	}

	uint8_t repititionCountField = runLengthBit | count;
	packet.push_back(repititionCountField);

	packet.push_back(this->pixelBuffer[i].z);
	packet.push_back(this->pixelBuffer[i].y);
	packet.push_back(this->pixelBuffer[i].x);

	if (this->GetAlphaChannelDepth() == 8)
	{
		packet.push_back(this->pixelBuffer[i].w);
	}

	i += count + 1;

	return packet;
}

std::vector<uint8_t> TgaImage::EncodeTrueColorRawPacket(size_t& i) const
{
	std::vector<uint8_t> packet = {};
	uint8_t runLengthBit = EPacketMask::RawPacket;
	uint8_t count = 0;
	size_t currentI = i;

	while (count < 127 && (this->pixelBuffer[currentI] != this->pixelBuffer[currentI + 1]))
	{
		count++;
		currentI++;

		if ((currentI + 1) % (this->header->Width - 1) == 0)
		{
			// Packets should not cross scan lines. TGA 2.0 spec.
			break;
		}
	}

	uint8_t repititionCountField = runLengthBit | count;
	packet.push_back(repititionCountField);

	for (size_t k = 0; k <= count; k++)
	{
		packet.push_back(this->pixelBuffer[i].z);
		packet.push_back(this->pixelBuffer[i].y);
		packet.push_back(this->pixelBuffer[i].x);

		if (this->GetAlphaChannelDepth() == 8)
		{
			packet.push_back(this->pixelBuffer[i].w);
		}

		i++;
	}

	return packet;
}

std::vector<uint8_t> TgaImage::EncodeBlackWhiteRunLengthPacket(size_t& i) const
{
	std::vector<uint8_t> packet = {};
	uint8_t runLengthBit = EPacketMask::RunLengthPacket;
	uint8_t count = 0;
	uint8_t j = 1;

	while (count < 127 && (this->pixelBuffer[i] == this->pixelBuffer[i + j]))
	{
		count++;
		j++;

		if ((i + j) % (this->header->Width - 1) == 0)
		{
			// Packets should not cross scan lines. TGA 2.0 spec.
			break;
		}
	}

	uint8_t repititionCountField = runLengthBit | count;
	packet.push_back(repititionCountField);

	packet.push_back(this->pixelBuffer[i].x);

	if (this->GetAlphaChannelDepth() == 8)
	{
		packet.push_back(this->pixelBuffer[i].w);
	}

	i += count + 1;

	return packet;
}

std::vector<uint8_t> TgaImage::EncodeBlackWhiteRawPacket(size_t& i) const
{
	std::vector<uint8_t> packet = {};
	uint8_t runLengthBit = EPacketMask::RawPacket;
	uint8_t count = 0;
	size_t currentI = i;

	while (count < 127 && (this->pixelBuffer[currentI] != this->pixelBuffer[currentI + 1]))
	{
		count++;
		currentI++;

		if ((currentI + 1) % (this->header->Width - 1) == 0)
		{
			// Packets should not cross scan lines. TGA 2.0 spec.
			break;
		}
	}

	uint8_t repititionCountField = runLengthBit | count;
	packet.push_back(repititionCountField);

	for (size_t k = 0; k <= count; k++)
	{
		packet.push_back(this->pixelBuffer[i].x);

		if (this->GetAlphaChannelDepth() == 8)
		{
			packet.push_back(this->pixelBuffer[i].w);
		}

		i++;
	}

	return packet;
}

void TgaImage::WriteDeveloperDirectoryToFile(std::ofstream& outFile) const
{
	if (!outFile.good() || this->footer == nullptr || this->developerDirectory == nullptr)
	{
		return;
	}

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
	if (!outFile.good() || this->footer == nullptr || this->extensions == nullptr)
	{
		return;
	}

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
	if (!outFile.good() || this->footer == nullptr)
	{
		return;
	}

	outFile.seekp(-Footer::SIZE, std::ios_base::end);

	outFile.write((char*)&this->footer->ExtensionAreaOffset, sizeof(uint32_t));
	outFile.write((char*)&this->footer->DeveloperDirectoryOffset, sizeof(uint32_t));
	outFile.write((char*)&this->footer->Signature, sizeof(this->footer->Signature));
	outFile.write((char*)&this->footer->ReservedCharacter, sizeof(uint8_t));
	outFile.write((char*)&this->footer->ZeroTerminator, sizeof(uint8_t));
}

const std::shared_ptr<Vec4[]> const TgaImage::GetPixelBuffer() const
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

EImageType TgaImage::GetImageType() const
{
	return this->header->ImageType;
}