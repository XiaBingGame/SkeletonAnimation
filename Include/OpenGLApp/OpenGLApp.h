#ifndef __OPENGL_APP_H__
#define __OPENGL_APP_H__

#include "preinclude.h"

#include <stdio.h>
#include <string.h>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef float mmat4[16];
typedef float mvec3[3];
typedef float mvec4[4];


OPENGLAPPDLL_API GLuint LoadShadersV(const char * vertex_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVF(const char * vertex_file_path,const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVTF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVGF(const char * vertex_file_path, const char* geometry_file_path, const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersVTGF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char* geometry_file_path, const char * fragment_file_path);
OPENGLAPPDLL_API GLuint LoadShadersCS(const char * computer_file_path);

class OPENGLAPPDLL_API OpenGLApp
{
public:
	OpenGLApp() {}
	virtual ~OpenGLApp() {}

	virtual void run()
	{
		bool running = true;
		//if(the_app == 0)
		app = this;
		//else
		//	app = the_app;

		if (!glfwInit())
		{
			fprintf(stderr, "Failed to initialize GLFW\n");
			return;
		}
		
		init();
		
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.minorVersion);
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif /* _DEBUG */
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES, info.samples);
		glfwWindowHint(GLFW_STEREO, info.flags.stereo ? GL_TRUE : GL_FALSE);
		
		if (info.flags.fullscreen)
		{
			/*
			if (info.windowWidth == 0 || info.windowHeight == 0)
			{
				GLFWvidmode mode;
				glfwGetDesktopMode(&mode);
				info.windowWidth = mode.Width;
				info.windowHeight = mode.Height;
			}
			*/

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode *mode = glfwGetVideoMode(monitor);
			info.windowWidth = mode->width;
			info.windowHeight = mode->height;
			window = glfwCreateWindow(info.windowWidth, info.windowHeight, info.title, monitor, NULL);	
			glfwSwapInterval((int)info.flags.vsync);
		}
		else
		{
			window = glfwCreateWindow(info.windowWidth, info.windowHeight, info.title, NULL, NULL);
		}	

		if(!window)
			return;

		glfwMakeContextCurrent(window);
		
#ifdef _DEBUG
		fprintf(stderr, "VENDOR: %s\n", glGetString(GL_VENDOR));
		fprintf(stderr, "VERSION: %s\n", (char *)glGetString(GL_VERSION));
		fprintf(stderr, "RENDERER: %s\n", (char *)glGetString(GL_RENDERER));
#endif
		glewExperimental = true;
		if (glewInit() != GLEW_OK)
		{
			fprintf(stderr, "Failed to initialize GLEW\n");
			glfwDestroyWindow(window);
			return;
		}
		onResize(info.windowWidth, info.windowHeight);

		glfwSetWindowSizeCallback(window, glfw_onResize);
		glfwSetKeyCallback(window, glfw_onKey);
		glfwSetMouseButtonCallback(window, glfw_onMouseButton);
		glfwSetCursorPosCallback(window, glfw_onMouseMove);
		info.flags.stereo = (glfwGetWindowAttrib(window, GLFW_STEREO) ? 1 : 0);

		startup();
		do
		{
			render(glfwGetTime());

			glfwSwapBuffers(window);
			glfwPollEvents();

			running &= (glfwWindowShouldClose(window) == GL_FALSE);
		} while( running );

		shutdown();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	GLFWwindow* getWindow() { return window; } 

	virtual void init()
	{
		strcpy_s(info.title, 128, "OpenGL Application");
		info.windowWidth = 800;
		info.windowHeight = 600;
		info.majorVersion = 4;
		info.minorVersion = 3;
		info.samples = 0;
		info.flags.all = 0;
		info.flags.cursor = 1;
#ifdef _DEBUG
		info.flags.debug = 1;
#endif
	}

	virtual void startup()
	{
		
	}

	virtual void render(double currentTime)
	{
		static const GLfloat clearcolor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearBufferfv(GL_COLOR, 0, clearcolor);
		static float cleardepth = 1.0;
		glClearBufferfv(GL_DEPTH, 0, &cleardepth);
	}

	virtual void shutdown()
	{
	}

	virtual void onResize(int w, int h)
	{
		info.windowWidth = w;
		info.windowHeight = h;
		glViewport(0, 0, w, h);
	}

	virtual void onKey(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
	}

	virtual void onMouseButton(int button, int action, int mods)
	{
		glfwGetCursorPos(window, &mCurPosx, &mCurPosy);
		int i = 0;
	}

	virtual void onMouseMove(int x, int y)
	{
	}

	double getCursorX() { return mCurPosx; }
	double getCursorY() { return mCurPosy; }

public:
	struct APPINFO
	{
		char title[128];
		int windowWidth;
		int windowHeight;
		int majorVersion;
		int minorVersion;
		int samples;
		union
		{
			struct 
			{
				unsigned int    fullscreen  : 1;
				unsigned int    vsync       : 1;
				unsigned int    cursor      : 1;
				unsigned int    stereo      : 1;
				unsigned int    debug       : 1;
			};
			unsigned int	 all;
		} flags;
	};

protected:
	APPINFO info;
	static OpenGLApp* app;
	GLFWwindow* window;

	static void glfw_onResize(GLFWwindow* window, int w, int h)
	{
		app->onResize(w, h);
	}

	static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		app->onKey(key, scancode, action, mods);
	}

	static void glfw_onMouseButton(GLFWwindow* window, int button, int action, int mods)
	{
		app->onMouseButton(button, action, mods);
	}

	static void glfw_onMouseMove(GLFWwindow* window, double x, double y)
	{
		app->onMouseMove((int)x, (int)y);
	}

	void setVsync(bool enable)
	{
		info.flags.vsync = enable ? 1 : 0;
		glfwSwapInterval((int)info.flags.vsync);
	}

private:
	double mCurPosx;
	double mCurPosy;

	OpenGLApp(const OpenGLApp&);
	OpenGLApp& operator=(const OpenGLApp&);
};

#endif