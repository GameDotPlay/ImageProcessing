#pragma once

#include "../TGA/public/TgaImage.h"

/** This class contains any effects that can be applied to an image. */
class Effects
{
public:

	/**
	* Applies a Gaussian Blur effect to the given image.
	* @param tgaImage The image to modify.
	* @param blurAmount Value of 0-1 inclusive. Higher value gives stronger blur effect.
	*/
	static void GaussianBlur(TgaImage& tgaImage, const float blurAmount);

private:

	/**
	 * Constructor not allowed for static class.
	 */
	Effects() = delete;

	/**
	 * Destructor not allowed for static class.
	 */
	~Effects() = delete;

	/**
    * Creates a normalized Gaussian matrix of values.
    * @param radius The radius of the kernel. Higher value gives stronger blurring effect.
    * @param sigma The standard deviation to use for the kernel. Higher value gives stronger blurring effect.
    * @return The normalized Gaussian matrix.
    */
	static std::vector<std::vector<float>> GetGaussianMatrix(const int32_t radius, const float sigma);
};
