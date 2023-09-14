#pragma once

#include "../TGA/public/TgaImage.h"

class Effects
{
public:

	static void GaussianBlur(TgaImage& tgaImage, const float blurAmount);

private:

	Effects() = delete;
	~Effects() = delete;


};
