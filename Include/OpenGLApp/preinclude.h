#ifndef __HXB_PRE_INCLUDED__
#define __HXB_PRE_INCLUDED__

#ifdef OPENGLAPPDLL_EXPORTS
#define OPENGLAPPDLL_API __declspec(dllexport) 
#else
#define OPENGLAPPDLL_API __declspec(dllimport) 
#endif

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

#endif



