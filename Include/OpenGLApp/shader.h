#ifndef SHADER_HPP
#define SHADER_HPP
#include "preinclude.h"

OPENGLAPPDLL_API GLuint LoadShadersV(const char * vertex_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVF(const char * vertex_file_path,const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVTF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVGF(const char * vertex_file_path, const char* geometry_file_path, const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVTGF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char* geometry_file_path, const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersCS(const char * computer_file_path);

#endif
