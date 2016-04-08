#include <iostream>


#include "ogremodelloader.h"

#ifdef _DEBUG
#pragma comment(lib, "OpenGLAppDll_d.lib")
#else
#pragma comment(lib, "OpenGLAppDll.lib")
#endif

/** @} @} */

int main(int argc, char** argv)
{
	model_loader_app a;
	a.run();
	return 0;
}