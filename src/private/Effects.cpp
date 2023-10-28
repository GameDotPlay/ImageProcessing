#include <Effects.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <corecrt_math_defines.h>

std::shared_ptr<Vec4[]> const Effects::GaussianBlur(const std::shared_ptr<Vec4[]>& pixels, const size_t width, const size_t height, float blurAmount)
{
	blurAmount = std::clamp(blurAmount, 0.0f, 1.0f);

	// Create pixel data to store new pixel values;
	size_t length = width * height;
	std::shared_ptr<Vec4[]> newPixels = std::make_shared<Vec4[]>(length);

	// Scale the radius of the blurring effect by blurAmount, but we always want a radius of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value of the radius to get a near-unrecognizable image at blurAmount = 1.
	int32_t radius = std::max((int)round(10 * blurAmount), 1);

	// Scale the sigma value by the blurAmount, but we always want a sigma of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value for sigma to get a near-unrecognizable image at blurAmount = 1
	float sigma = std::max(10.0f * blurAmount, 1.0f);

	std::vector<std::vector<float>> kernel = Effects::GetGaussianMatrix(radius, sigma);

	// Apply filter to each pixel in the image.
	for (size_t pixelIndex = 0; pixelIndex < length; pixelIndex++)
	{
		Vec4f newPixel = {};

		for (int32_t kernelY = -radius; kernelY <= radius; kernelY++)
		{
			for (int32_t kernelX = -radius; kernelX <= radius; kernelX++)
			{
				float kernelValue = kernel[kernelY + radius][kernelX + radius];
				int32_t kernelSamplePixelIndex = (pixelIndex + kernelX) + (width * kernelY);

				// If the kernel tries to process a pixel outside of the image, just continue.
				if (kernelSamplePixelIndex < 0 || kernelSamplePixelIndex > length - 1)
				{
					continue;
				}

				newPixel.x += (float)pixels[kernelSamplePixelIndex].x * kernelValue;
				newPixel.y += (float)pixels[kernelSamplePixelIndex].y * kernelValue;
				newPixel.z += (float)pixels[kernelSamplePixelIndex].z * kernelValue;
				newPixel.w += (float)pixels[kernelSamplePixelIndex].w * kernelValue;
			}
		}

		newPixels[pixelIndex].x = round(newPixel.x);
		newPixels[pixelIndex].y = round(newPixel.y);
		newPixels[pixelIndex].z = round(newPixel.z);
		newPixels[pixelIndex].w = round(newPixel.w);
	}

	return newPixels;
}

std::shared_ptr<Vec4[]> const Effects::GaussianBlurSeparate(const std::shared_ptr<Vec4[]>& pixels, const size_t width, const size_t height, float blurAmount)
{
	blurAmount = std::clamp(blurAmount, 0.0f, 1.0f);

	// Create pixel data to store new pixel values;
	size_t length = width * height;
	std::shared_ptr<Vec4[]> newPixels = std::make_shared<Vec4[]>(length);

	// Scale the radius of the blurring effect by blurAmount, but we always want a radius of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value of the radius to get a near-unrecognizable image at blurAmount = 1.
	int32_t radius = std::max((int)round(10 * blurAmount), 1);

	// Scale the sigma value by the blurAmount, but we always want a sigma of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value for sigma to get a near-unrecognizable image at blurAmount = 1
	float sigma = std::max(10.0f * blurAmount, 1.0f);

	std::vector<float> kernel = Effects::Get1DMatrix(radius, sigma);

	// Apply a 1D kernel in the horizontal orientation to all pixels.
	for (size_t pixelIndex = 0; pixelIndex < length; pixelIndex++)
	{
		Vec4f pixel = {};

		for (int32_t kernelColumn = -radius; kernelColumn <= radius; kernelColumn++)
		{
			float kernelValue = kernel[kernelColumn + radius];
			int32_t kernelSamplePixelIndex = pixelIndex + kernelColumn;

			// If the kernel tries to process a pixel outside of the image, give it the value of the closest valid pixel.
			if (kernelSamplePixelIndex < 0)
			{
				kernelSamplePixelIndex = 0;
			}
			else if (kernelSamplePixelIndex >= length)
			{
				kernelSamplePixelIndex = length - 1;
			}

			pixel.x += (float)pixels[kernelSamplePixelIndex].x * kernelValue;
			pixel.y += (float)pixels[kernelSamplePixelIndex].y * kernelValue;
			pixel.z += (float)pixels[kernelSamplePixelIndex].z * kernelValue;
			pixel.w += (float)pixels[kernelSamplePixelIndex].w * kernelValue;
		}

		newPixels[pixelIndex].x = (uint8_t)ceil(pixel.x);
		newPixels[pixelIndex].y = (uint8_t)ceil(pixel.y);
		newPixels[pixelIndex].z = (uint8_t)ceil(pixel.z);
		newPixels[pixelIndex].w = (uint8_t)ceil(pixel.w);
	}

	// Apply a 1D kernel in the vertical direction to all pixels.
	for (size_t pixelIndex = 0; pixelIndex < length; pixelIndex++)
	{
		Vec4f pixel = {};
		for (int32_t kernelRow = -radius; kernelRow <= radius; kernelRow++)
		{
			float kernelValue = kernel[kernelRow + radius];
			int32_t kernelSamplePixelIndex = pixelIndex + (width * kernelRow);

			// If the kernel tries to process a pixel outside of the image, give it the value of the closest valid pixel.
			if (kernelSamplePixelIndex < 0)
			{
				kernelSamplePixelIndex = 0;
			}
			else if (kernelSamplePixelIndex >= length)
			{
				kernelSamplePixelIndex = length - 1;
			}

			pixel.x += (float)newPixels[kernelSamplePixelIndex].x * kernelValue;
			pixel.y += (float)newPixels[kernelSamplePixelIndex].y * kernelValue;
			pixel.z += (float)newPixels[kernelSamplePixelIndex].z * kernelValue;
			pixel.w += (float)newPixels[kernelSamplePixelIndex].w * kernelValue;
		}

		newPixels[pixelIndex].x = (uint8_t)ceil(pixel.x);
		newPixels[pixelIndex].y = (uint8_t)ceil(pixel.y);
		newPixels[pixelIndex].z = (uint8_t)ceil(pixel.z);
		newPixels[pixelIndex].w = (uint8_t)ceil(pixel.w);
	}

	return newPixels;
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

std::vector<float> Effects::Get1DMatrix(const int32_t radius, const float sigma)
{
	// Kernel width is a function of the radius passed in.
	// But we always want a width of at least 3, and width should be odd so there is always a center pixel.
	size_t kernelWidth = std::max(((2 * radius) + 1), 3);

	// Calculate Gaussian values for the kernel matrix.
	std::vector<float> kernel(kernelWidth);
	float sum = 0.0f;
	for (int32_t i = -radius; i <= radius; i++)
	{
		float exponentNumerator = (float)(i * i);
		float exponentDenominator = 2.0f * (sigma * sigma);

		float eExpression = exp(-exponentNumerator / exponentDenominator);
		float kernelValue = eExpression / sqrt(2.0f * (float)M_PI * sigma);

		kernel[i + radius] = kernelValue;
		sum += kernelValue;
	}

	// Normalize kernel values so they sum to 1.0.
	for (auto& value : kernel)
	{
		value /= sum;
	}

	return kernel;
}
