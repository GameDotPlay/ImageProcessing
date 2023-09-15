#pragma once

#include <cstdint>

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
