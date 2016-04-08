#include "TextureHelper.h"
#include <string>
#include <algorithm>
#include "OpenGLApp.h"
#include "memory.h"
extern "C" {
#include "jpeglib/jpeglib.h"
}
unsigned int loadJPG_custom(const char* filename)
{
	// 声明并初始化解压缩对象
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// 打开jpg图像文件，并指定为解压缩对象的源文件
	FILE *f = fopen(filename,"rb");
	if (f==NULL)
	{
		return 0;
	}

	jpeg_stdio_src(&cinfo, f);
	// 读取图像信息
	jpeg_read_header(&cinfo, TRUE);
	// 根据图像信息申请一个图像缓冲区
	unsigned char* data = new unsigned char[cinfo.image_width*cinfo.image_height*cinfo.num_components];

	// 开始解压缩
	jpeg_start_decompress(&cinfo);
	JSAMPROW row_pointer[1];
	while (cinfo.output_scanline < cinfo.output_height)
	{
		row_pointer[0] = reinterpret_cast<JSAMPROW>(&data[(cinfo.output_height-cinfo.output_scanline-1)*cinfo.image_width*cinfo.num_components]);
		jpeg_read_scanlines(&cinfo,row_pointer , 1);
	}
	jpeg_finish_decompress(&cinfo);
	// 释放资源
	jpeg_destroy_decompress(&cinfo);
	fclose(f);


	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cinfo.image_width, cinfo.image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	delete [] data;
	// Return the ID of the texture we just created
	return textureID;
}


// Define targa header. This is only used locally.
#pragma pack(1)
typedef struct
{
	GLbyte	identsize;              // Size of ID field that follows header (0)
	GLbyte	colorMapType;           // 0 = None, 1 = paletted
	GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
	unsigned short	colorMapStart;          // First colour map entry
	unsigned short	colorMapLength;         // Number of colors
	unsigned char 	colorMapBits;   // bits per palette entry
	unsigned short	xstart;                 // image x origin
	unsigned short	ystart;                 // image y origin
	unsigned short	width;                  // width in pixels
	unsigned short	height;                 // height in pixels
	GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
	GLbyte	descriptor;             // image descriptor
} TGAHEADER;
#pragma pack(8)

unsigned int loadTGA_custom(const char *szFileName)
{
	FILE *pFile;			// File pointer
	TGAHEADER tgaHeader;		// TGA file header
	unsigned long lImageSize;		// Size in bytes of image
	short sDepth;			// Pixel depth;
	unsigned char *pBits = NULL;          // Pointer to bits

	// Default/Failed values
	unsigned int iWidth = 0;
	unsigned int iHeight = 0;
	GLenum eFormat = GL_BGR_EXT;
	GLenum iComponents = GL_RGB8;

	// Attempt to open the fil
	pFile = fopen(szFileName, "rb");
	if(pFile == NULL)
		return 0;

	// Read in header (binary)
	fread(&tgaHeader, 18/* sizeof(TGAHEADER)*/, 1, pFile);

	// Do byte swap for big vs little endian
#ifdef __APPLE__
	LITTLE_ENDIAN_WORD(&tgaHeader.colorMapStart);
	LITTLE_ENDIAN_WORD(&tgaHeader.colorMapLength);
	LITTLE_ENDIAN_WORD(&tgaHeader.xstart);
	LITTLE_ENDIAN_WORD(&tgaHeader.ystart);
	LITTLE_ENDIAN_WORD(&tgaHeader.width);
	LITTLE_ENDIAN_WORD(&tgaHeader.height);
#endif


	// Get width, height, and depth of texture
	iWidth = tgaHeader.width;
	iHeight = tgaHeader.height;
	sDepth = tgaHeader.bits / 8;

	// Put some validity checks here. Very simply, I only understand
	// or care about 8, 24, or 32 bit targa's.
	if(tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
		return NULL;

	// Calculate size of image buffer
	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	// Allocate memory and check for success
	pBits = (unsigned char*)malloc(lImageSize * sizeof(unsigned char));
	if(pBits == NULL)
		return NULL;

	// Read in the bits
	// Check for read error. This should catch RLE or other 
	// weird formats that I don't want to recognize
	if(fread(pBits, lImageSize, 1, pFile) != 1)
	{
		free(pBits);
		return NULL;
	}

	// Set OpenGL format expected
	switch(sDepth)
	{
	case 3:     // Most likely case
		eFormat = GL_BGR;
		iComponents = GL_RGB;
		break;
	case 4:
		eFormat = GL_BGRA;
		iComponents = GL_RGBA;
		break;
	case 1:
		eFormat = GL_LUMINANCE;
		iComponents = GL_LUMINANCE;
		break;
	};


	// Done with File
	fclose(pFile);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return pointer to image data

	free(pBits);
	return textureID;
}


unsigned int loadImage(const char* filename)
{
	std::string fn(filename);
	std::string extname = fn.substr(fn.length()-4, 4);
	
	std::transform(extname.begin(), extname.end(), extname.begin(), ::tolower);

	if(fn.length() < 5)
		return 0;
	if(extname == ".bmp")
	{
		return loadBMP_custom(filename);
	}
	else if(extname == ".jpg")
	{
		return loadJPG_custom(filename);
	}
	else if(extname == ".tga")
	{
		return loadTGA_custom(filename);
	}
	return 0;
}