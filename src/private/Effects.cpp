#include <Effects.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <corecrt_math_defines.h>

std::unique_ptr<glm::u8vec4[]> const Effects::GaussianBlur(const std::shared_ptr<glm::u8vec4[]>& pixels, const size_t width, const size_t height, float blurAmount)
{
	blurAmount = std::clamp(blurAmount, 0.0f, 1.0f);
	size_t length = width * height;
	std::unique_ptr<glm::u8vec4[]> newPixels = std::make_unique<glm::u8vec4[]>(length);

	// Scale the radius of the blurring effect by blurAmount, but we always want a radius of at least 1.
	// A value of 20 is chosen here as a reasonable maximum value of the radius to get a near-unrecognizable image at blurAmount = 1.
	int32_t radius = std::max((int)round(20 * blurAmount), 1);

	// Scale the sigma value by the blurAmount, but we always want a sigma of at least 1.
	// A value of 10 is chosen here as a reasonable maximum value for sigma to get a near-unrecognizable image at blurAmount = 1
	float sigma = std::max(10.0f * blurAmount, 1.0f);

	std::vector<float> kernel = Effects::Get1DMatrix(radius, sigma);

	// Apply a 1D kernel in the horizontal orientation to all pixels.
	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			glm::u8vec4 pixel = {};
			size_t pixelIndex = j + (i * height);

			for (int32_t kernelColumn = -radius; kernelColumn <= radius; kernelColumn++)
			{
				int32_t kernelSamplePixelIndex = pixelIndex + kernelColumn;

				if (kernelSamplePixelIndex < 0)
				{
					kernelSamplePixelIndex = 0;
				}
				else if (kernelSamplePixelIndex >= length)
				{
					kernelSamplePixelIndex = length - 1;
				}

				float kernelValue = kernel[kernelColumn + radius];

				pixel.w += pixels[kernelSamplePixelIndex].w * kernelValue;
				pixel.x += pixels[kernelSamplePixelIndex].x * kernelValue;
				pixel.y += pixels[kernelSamplePixelIndex].y * kernelValue;
				pixel.z += pixels[kernelSamplePixelIndex].z * kernelValue;
			}

			newPixels[pixelIndex].w = (uint8_t)std::clamp((float)round(pixel.w), 0.0f, 255.0f);
			newPixels[pixelIndex].x = (uint8_t)std::clamp((float)round(pixel.x), 0.0f, 255.0f);
			newPixels[pixelIndex].y = (uint8_t)std::clamp((float)round(pixel.y), 0.0f, 255.0f);
			newPixels[pixelIndex].z = (uint8_t)std::clamp((float)round(pixel.z), 0.0f, 255.0f);
		}
	}

	// Apply a 1D kernel in the vertical direction to all pixels.
	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			glm::u8vec4 pixel = {};
			size_t pixelIndex = j + (i * height);

			for (int32_t kernelRow = -radius; kernelRow <= radius; kernelRow++)
			{
				int32_t kernelSamplePixelIndex = pixelIndex + (width * kernelRow);

				if (kernelSamplePixelIndex < 0)
				{
					kernelSamplePixelIndex = 0;
				}
				else if (kernelSamplePixelIndex >= length)
				{
					kernelSamplePixelIndex = length - 1;
				}

				float kernelValue = kernel[kernelRow + radius];

				pixel.w += newPixels[kernelSamplePixelIndex].w * kernelValue;
				pixel.x += newPixels[kernelSamplePixelIndex].x * kernelValue;
				pixel.y += newPixels[kernelSamplePixelIndex].y * kernelValue;
				pixel.z += newPixels[kernelSamplePixelIndex].z * kernelValue;
			}

			newPixels[pixelIndex].w = (uint8_t)std::clamp((float)round(pixel.w), 0.0f, 255.0f);
			newPixels[pixelIndex].x = (uint8_t)std::clamp((float)round(pixel.x), 0.0f, 255.0f);
			newPixels[pixelIndex].y = (uint8_t)std::clamp((float)round(pixel.y), 0.0f, 255.0f);
			newPixels[pixelIndex].z = (uint8_t)std::clamp((float)round(pixel.z), 0.0f, 255.0f);
		}
	}

	return newPixels;
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
