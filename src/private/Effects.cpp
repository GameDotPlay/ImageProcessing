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
	newPixels.shrink_to_fit();

	// Scale the radius of the blurring effect by blurAmount, but we always want a radius of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value of the radius to get a near-unrecognizable image at blurAmount = 1.
	int32_t radius = std::max((int)round(10 * blurAmount), 1);

	// Scale the sigma value by the blurAmount, but we always want a sigma of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value for sigma to get a near-unrecognizable image at blurAmount = 1
	float sigma = std::max(10.0f * blurAmount, 1.0f);

	std::vector<std::vector<float>> kernel = Effects::GetGaussianMatrix(radius, sigma);

	// Apply filter to each pixel in the image.
	for (size_t pixelIndex = 0; pixelIndex < tgaImage.GetPixelData().size(); pixelIndex++)
	{
		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;
		float alpha = 0.0f;

		for (int32_t kernelY = -radius; kernelY <= radius; kernelY++)
		{
			for (int32_t kernelX = -radius; kernelX <= radius; kernelX++)
			{
				float kernelValue = kernel[kernelY + radius][kernelX + radius];
				int32_t kernelSamplePixelIndex = (pixelIndex + kernelX) + (tgaImage.GetHeader()->Width * kernelY);

				// If the kernel tries to process a pixel outside of the image, just continue.
				if (kernelSamplePixelIndex < 0 || kernelSamplePixelIndex > tgaImage.GetPixelData().size() - 1)
				{
					continue;
				}

				red += (float)tgaImage.GetPixelData()[kernelSamplePixelIndex].red * kernelValue;
				green += (float)tgaImage.GetPixelData()[kernelSamplePixelIndex].green * kernelValue;
				blue += (float)tgaImage.GetPixelData()[kernelSamplePixelIndex].blue * kernelValue;
				alpha += (float)tgaImage.GetPixelData()[kernelSamplePixelIndex].alpha * kernelValue;
			}
		}

		newPixels[pixelIndex].red = round(red);
		newPixels[pixelIndex].green = round(green);
		newPixels[pixelIndex].blue = round(blue);
		newPixels[pixelIndex].alpha = round(alpha);
	}

	// Replace the old pixel data with the new pixel data.
	tgaImage.SetPixelData(newPixels);
}

std::vector<std::vector<float>> Effects::GetGaussianMatrix(const int32_t radius, const float sigma)
{
	// Kernel width is a function of the radius passed in.
	// But we always want a width of at least 3, and width should be odd so there is always a center pixel.
	size_t kernelWidth = std::max(((2 * radius) + 1), 3);

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
