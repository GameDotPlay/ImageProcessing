#include "../public/Effects.h"
#include <vector>
#include <cmath>
#include <algorithm>
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

	int radius = std::max((int)(10 * blurAmount), 1);
	int kernelWidth = (2 * radius) + 1;
	float sigma = (float)radius / 2.0f;
	std::vector<std::vector<float> > kernel(kernelWidth, std::vector<float>(kernelWidth));

	// Calculate Gaussian values for the kernel matrix.
	float sum = 0.0f;
	for (int32_t x = -radius; x <= radius; x++)
	{
		for (int32_t y = -radius; y <= radius; y++)
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

	// Apply filter to each pixel in the image.
	for (size_t pixelIndex = 0; pixelIndex < tgaImage.GetPixelData().size(); pixelIndex++)
	{
		uint8_t red = 0;
		uint8_t green = 0;
		uint8_t blue = 0;

		for (int32_t kernelX = -radius; kernelX <= radius; kernelX++)
		{
			for (int32_t kernelY = -radius; kernelY <= radius; kernelY++)
			{
				float kernelValue = kernel[kernelX + radius][kernelY + radius];
				int32_t kernelPixelIndex = (pixelIndex + kernelX) + (tgaImage.GetHeader()->Width * kernelY);

				// If the kernel tries to process a pixel outside of the image, just continue.
				if (kernelPixelIndex < 0 || kernelPixelIndex > tgaImage.GetPixelData().size() - 1)
				{
					continue;
				}

				red += (tgaImage.GetPixelData()[kernelPixelIndex].red) * kernelValue;
				green += (tgaImage.GetPixelData()[kernelPixelIndex].green) * kernelValue;
				blue += (tgaImage.GetPixelData()[kernelPixelIndex].blue) * kernelValue;
			}
		}

		newPixels[pixelIndex].red = red;
		newPixels[pixelIndex].green = green;
		newPixels[pixelIndex].blue = blue;
		newPixels[pixelIndex].alpha = tgaImage.GetPixelData()[pixelIndex].alpha;
	}

	// Replace the old pixel data with the new pixel data.
	tgaImage.SetPixelData(newPixels);
}
