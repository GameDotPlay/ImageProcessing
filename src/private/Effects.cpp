#include "../public/Effects.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <corecrt_math_defines.h>

void Effects::GaussianBlur(TgaImage& tgaImage, float blurAmount)
{
	blurAmount = std::clamp(blurAmount, 0.0f, 1.0f);

	// Create pixel data to store new pixel values;
	std::vector<Pixel> newPixels(tgaImage.GetPixelData().size());

	float sigma = 5.0f * blurAmount;
	int32_t kernelWidth = std::max((int)round(2 * M_PI * sigma * blurAmount), 3);
	if (kernelWidth % 2 == 0)
	{
		kernelWidth--;
	}

	int32_t radius = kernelWidth / 2;

	std::vector<std::vector<float>> kernel = Effects::GetGaussianMatrix(radius, sigma, kernelWidth);

	// Apply filter to each pixel in the image.
	for (size_t pixelIndex = 0; pixelIndex < tgaImage.GetPixelData().size(); pixelIndex++)
	{
		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;

		for (int32_t kernelY = -radius; kernelY <= radius; kernelY++)
		{
			for (int32_t kernelX = -radius; kernelX <= radius; kernelX++)
			{
				float kernelValue = kernel[kernelY + radius][kernelX + radius];
				int32_t kernelPixelIndex = (pixelIndex + kernelX) + (tgaImage.GetHeader()->Width * kernelY);

				// If the kernel tries to process a pixel outside of the image, just continue.
				if (kernelPixelIndex < 0 || kernelPixelIndex > tgaImage.GetPixelData().size() - 1)
				{
					continue;
				}

				red += (float)tgaImage.GetPixelData()[kernelPixelIndex].red * kernelValue;
				green += (float)tgaImage.GetPixelData()[kernelPixelIndex].green * kernelValue;
				blue += (float)tgaImage.GetPixelData()[kernelPixelIndex].blue * kernelValue;
			}
		}

		newPixels[pixelIndex].red = round(red);
		newPixels[pixelIndex].green = round(green);
		newPixels[pixelIndex].blue = round(blue);
		newPixels[pixelIndex].alpha = tgaImage.GetPixelData()[pixelIndex].alpha;
	}

	// Replace the old pixel data with the new pixel data.
	tgaImage.SetPixelData(newPixels);
}

std::vector<std::vector<float>> Effects::GetGaussianMatrix(const int32_t radius, const float sigma, const size_t kernelWidth)
{
	// Calculate Gaussian values for the kernel matrix.
	std::vector<std::vector<float>> kernel(kernelWidth, std::vector<float>(kernelWidth));
	float sum = 0.0f;
	for (int32_t y = -radius; y <= radius; y++)
	{
		for (int32_t x = -radius; x <= radius; x++)
		{
			float exponentNumerator = -(float)((x * x) + (y * y));
			float exponentDenominator = 2.0f * (sigma * sigma);

			float eExpression = exp(exponentNumerator / exponentDenominator);
			float kernelValue = eExpression / (2.0f * (float)M_PI * (sigma * sigma));

			kernel[x + radius][y + radius] = kernelValue;
			sum += kernelValue;
		}
	}

	// Normalize kernel values to between 0 and 1.
	for (size_t y = 0; y < kernelWidth; y++)
	{
		for (size_t x = 0; x < kernelWidth; x++)
		{
			kernel[x][y] /= sum;
		}
	}

	return kernel;
}
