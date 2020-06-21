#pragma once
#include <windows.h>

class ImageLoader
{
public:
	unsigned char* textureData;
	int iWidth, iHeight;
	ImageLoader(const char*);
	~ImageLoader();

private:
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
};

