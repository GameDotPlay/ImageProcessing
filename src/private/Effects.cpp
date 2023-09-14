#include "../public/Effects.h"
#include <vector>
#include <cmath>
#include <corecrt_math_defines.h>

void Effects::GaussianBlur(TgaImage& tgaImage, float blurAmount)
{
	if (blurAmount == 0.0f)
	{
		return;
	}

	// Clamp blur amount to between 0 and 1.
	blurAmount = blurAmount < 0.0f ? 0.0f : blurAmount;
	blurAmount = blurAmount > 1.0f ? 1.0f : blurAmount;

	// Copy the image pixel data to store new pixel values.
	std::vector<Pixel> newPixels(tgaImage.GetPixelData().size());

	int radius = 10;
	int kernelWidth = (2 * radius) + 1;
	float sigma = (float)(radius / 2) * blurAmount;
	std::vector<std::vector<float> > kernel(kernelWidth, std::vector<float>(kernelWidth));

	// Calculate Gaussian values for the kernel matrix.
	float sum = 0.0f;
	for (int x = -radius; x <= radius; x++)
	{
		for (int y = -radius; y <= radius; y++)
		{
			float exponentNumerator = -((x * x) + (y * y));
			float exponentDenominator = 2 * (sigma * sigma);

			float eExpression = exp(exponentNumerator / exponentDenominator);
			float kernelValue = (eExpression / (2 * M_PI * (sigma * sigma)));

			kernel[x + radius][y + radius] = kernelValue;
			sum += kernelValue;
		}
	}

	// Normalize kernel values to between 0 and 1.
	for (size_t x = 0; x < kernelWidth; x++)
	{
		for (size_t y = 0; y < kernelWidth; y++)
		{
			kernel[x][y] /= sum;
		}
	}

	for (size_t pixelIndex = radius; pixelIndex < tgaImage.GetPixelData().size(); pixelIndex++)
	{
		uint8_t red = 0;
		uint8_t green = 0;
		uint8_t blue = 0;

		for (int kernelX = -radius; kernelX <= radius; kernelX++)
		{
			for (int kernelY = -radius; kernelY <= radius; kernelY++)
			{
				float kernelValue = kernel[kernelX + radius][kernelY + radius];

				red += (tgaImage.GetPixelData()[pixelIndex - kernelX, pixelIndex - kernelY].red) * kernelValue;
				green += (tgaImage.GetPixelData()[pixelIndex - kernelX, pixelIndex - kernelY].green) * kernelValue;
				blue += (tgaImage.GetPixelData()[pixelIndex - kernelX, pixelIndex - kernelY].blue) * kernelValue;
			}
		}

		newPixels[pixelIndex].red = red;
		newPixels[pixelIndex].green = green;
		newPixels[pixelIndex].blue = blue;
	}

	// Apply filter to each pixel in the image.
	for (size_t imageX = radius; imageX < tgaImage.GetHeader()->Width; imageX++)
	{
		for (size_t imageY = radius; imageY <= tgaImage.GetHeader()->Height; imageY++)
		{
			uint8_t red = 0;
			uint8_t green = 0;
			uint8_t blue = 0;

			for (int kernelX = -radius; kernelX <= radius; kernelX++)
			{
				for(int kernelY = -radius; kernelY <= radius; kernelY++)
				{
					float kernelValue = kernel[kernelX + radius][kernelY + radius];

					red += (tgaImage.GetPixelData()[imageX - kernelX, imageY - kernelY].red) * kernelValue;
					green += (tgaImage.GetPixelData()[imageX - kernelX, imageY - kernelY].green) * kernelValue;
					blue += (tgaImage.GetPixelData()[imageX - kernelX, imageY - kernelY].blue) * kernelValue;
				}
			}

			newPixels[imageX + imageY].red = red;
			newPixels[imageX + imageY].green = green;
			newPixels[imageX + imageY].blue = blue;
		}
	}

	// Replace the old pixel data with the new pixel data.
	std::vector<Pixel>* pToPixels = &newPixels;
	tgaImage.GetPixelData() = *pToPixels;
}
