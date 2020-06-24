#include "ImageLoader.h"
#include <stdio.h>
#include <iostream>

using namespace std;

ImageLoader::ImageLoader(const char *filename) {
	FILE* file = NULL;
	file = fopen(filename, "rb");

	if (!file) {
		cout << "FILE NOT FOUND" << endl;
	}

	fread(&bfh, sizeof(BITMAPFILEHEADER), 1 , file);
	if (bfh.bfType != 0x4D42) {
		cout << "FILE NOT VALID" << endl;
	}
	
	fread(&bih, sizeof(BITMAPINFOHEADER), 1, file);
	if (bih.biSizeImage == 0) {
		bih.biSizeImage = bih.biHeight * bih.biWidth * 3;
	}

	textureData = new unsigned char[bih.biSizeImage];
	fseek(file, bfh.bfOffBits, SEEK_SET);
	fread(textureData, 1, bih.biSizeImage, file);

	unsigned char temp;
	for (int i = 0; i < bih.biSizeImage; i+=3) {
		temp = textureData[i];
		textureData[i] = textureData[i + 2];
		textureData[i + 2] = temp;
	}

	iWidth = bih.biWidth;
	iHeight = bih.biHeight;

	fclose(file);
}

ImageLoader::~ImageLoader() {
	delete[] textureData;
}
