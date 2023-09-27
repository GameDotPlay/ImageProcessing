#pragma once

#include <vector>

struct Pixel;

/** This class contains any effects that can be applied to an image. */
class Effects
{
public:

	/**
	* Applies a Gaussian Blur effect to the given image.
	* @param pixels The pixel data to modify.
	* @param width The width of the pixel data.
	* @param height The height of the pixel data.
	* @param blurAmount Value of 0-1 inclusive. Higher value gives stronger blur effect.
	*/
	static Pixel* const GaussianBlur(const Pixel* const pixels, const size_t width, const size_t height, float blurAmount);

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
