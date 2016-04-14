#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "preinclude.h"

struct header
{
	unsigned char       identifier[12];
	unsigned int        endianness;
	unsigned int        gltype;
	unsigned int        gltypesize;
	unsigned int        glformat;
	unsigned int        glinternalformat;
	unsigned int        glbaseinternalformat;
	unsigned int        pixelwidth;
	unsigned int        pixelheight;
	unsigned int        pixeldepth;
	unsigned int        arrayelements;
	unsigned int        faces;
	unsigned int        miplevels;
	unsigned int        keypairbytes;
};

// Load a .BMP file using our custom loader
OPENGLAPPDLL_API GLuint loadBMP_custom(const char * imagepath);

//// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another texture loading library, 
//// or do it yourself (just like loadBMP_custom and loadDDS)
//// Load a .TGA file using GLFW's own loader
//GLuint loadTGA_glfw(const char * imagepath);

// Load a .DDS file using GLFW's own loader
OPENGLAPPDLL_API GLuint loadDDS(const char * imagepath);

OPENGLAPPDLL_API unsigned int loadKTX(const char * filename, unsigned int tex = 0);
#endif