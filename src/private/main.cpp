import TexFile;

#include <iostream>
#include <string>
#include <chrono>
//#include <memory>
//#include <Vector.h>
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

	if (tgaImage.GetImageType() == 0)
	{
		std::cout << "An error occurred while attempting to parse image " << inputPath << std::endl;
		std::cout << "Verify correct image path or try a different image." << std::endl;
		return -1;
	}

	if ((int)tgaImage.GetImageType() != 2)
	{
		std::cout << "This image is of ImageType=" << (int)tgaImage.GetImageType() << std::endl;
		std::cout << "This application currently only supports uncompressed TRUE-COLOR tga images. (ImageType=2)." << std::endl;
		return -1;
	}

	auto start = std::chrono::high_resolution_clock::now();
	auto blurredPixels = Effects::GaussianBlur(tgaImage.GetPixelData(), tgaImage.GetWidth(), tgaImage.GetHeight(), blurValue);
	auto stop = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	tgaImage.SetPixelData(blurredPixels);
	tgaImage.SaveToFile(outputPath);

	std::cout << "New image saved to " << outputPath << std::endl;
	std::cout << "Gaussian Blur runtime: " << duration.count() << "ms";
}
