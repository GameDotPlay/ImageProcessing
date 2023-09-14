#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include "../TGA/public/TgaImage.h"
#include "../public/Effects.h"

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cout << "Incorrect parameters. Correct usage is:" << std::endl;
		std::cout << ".>ImageManipulation.exe <path to original image> <path to newly saved image> <blur amount 0.0 - 1.0>" << std::endl;
		return -1;
	}

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];
	float blurValue = std::stof(argv[3]);

	TgaImage tgaImage(inputPath);
	tgaImage.GetHeader()->PrintToConsole();

	Effects::GaussianBlur(tgaImage, blurValue);

	tgaImage.SaveToFile(outputPath);
}
