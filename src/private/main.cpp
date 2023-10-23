import TexFile;

#include <iostream>
#include <string>
#include <chrono>
#include <Effects.h>

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cout << "Incorrect parameters. Correct usage is:" << std::endl;
		std::cout << ".>ImageProcessing.exe <Input Image Path> <Output Image Path> <Blur Strength 0-1>" << std::endl;
		return -1;
	}

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];
	float blurValue = std::stof(argv[3]);

	Tga::TgaImage tgaImage(inputPath);

	// Does not support run length encoded color mapped images.
	if (tgaImage.GetImageType() == Tga::RunLengthEncodedColorMapped || tgaImage.GetImageType() == Tga::NoImageData)
	{
		std::cout << "An error occurred while parsing image or image format not supported" << inputPath << std::endl;
		std::cout << "Verify correct image path or try a different image." << std::endl;
		return -1;
	}
	else
	{
		auto start = std::chrono::high_resolution_clock::now();
		auto blurredPixels = Effects::GaussianBlur(tgaImage.GetPixelBuffer(), tgaImage.GetWidth(), tgaImage.GetHeight(), blurValue);
		auto stop = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

		tgaImage.SetPixelData(blurredPixels);
		tgaImage.SaveToFile(outputPath, tgaImage.GetImageType());

		std::cout << "New image saved to " << outputPath << std::endl;
		std::cout << "Gaussian Blur runtime: " << duration.count() << "ms";
	}
}
