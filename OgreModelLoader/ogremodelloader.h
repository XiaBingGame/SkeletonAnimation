#ifndef __TEMPLATE_APP_HEADER_H__
#define __TEMPLATE_APP_HEADER_H__

#include "OpenGLApp.h"
#include <vector>
class Mesh;

/** Class */
class model_loader_app : public OpenGLApp
{
public:
	/** 初始化函数.
	@remarks
		最初的初始化.
	*/
	void init();

	/// OpenGL 环境初始化
	void startup();

	/// OpenGL 环境销毁
	void shutdown();

	virtual void onKey(int key, int scancode, int action, int mods);

	/// 渲染
	void render(double t);

	virtual void onMouseButton(int button, int action, int mods);

private:
	void reloadShader();
	GLuint      program;		/// 程序对象
	GLuint		skeletonProgram;
	GLuint      vao;			/// 顶点数组对象
	GLuint		proj_location;
	GLuint		mv_location;
	GLuint		light_location;
	GLuint		tex1_location;
	GLuint		tex2_location;
	GLuint		tex3_location;
	GLuint		skeleton_mv_location;
	GLuint		skeleton_proj_location;
	GLuint		skeleton_bonemat_location;
	GLuint		uiProgram;

	bool		bLeftPressed;
	bool		bRightPressed;
	bool		bPaused;
	Mesh*		mMesh;
	int			mModelIndex;
	float		m_yRot;
	std::vector<Mesh*> mMeshes;
};


#endif