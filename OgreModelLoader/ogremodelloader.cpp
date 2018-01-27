#include "ogremodelloader.h"
#include "shader.h"
#include "texture.h"
#include "model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Windows.h>
#include "mesh.h"

void model_loader_app::init()
{
	static const char title[] = "OpenGL Application";
	OpenGLApp::init();
	memcpy(info.title, title, sizeof(title));

	mMesh = 0;
	bLeftPressed = false;
	bRightPressed = false;
}

void model_loader_app::startup(void)
{
	program = LoadShadersVF("../media/glsl/glsl.vs", "../media/glsl/glsl.fs");
	skeletonProgram = LoadShadersVF("../media/glsl/skeleton.vs", "../media/glsl/skeleton.fs");
	uiProgram = LoadShadersVF("../media/glsl/ui.vs", "../media/glsl/ui.fs");
	Mesh* m = new Mesh("../media/halo/halo.material", "../media/halo/halo.mesh.xml", program, "../media/halo/halo.skeleton.xml");
	mMeshes.push_back(m);
	m = new Mesh("../media/jaiqua/jaiqua.material", "../media/jaiqua/jaiqua.mesh.xml", program, "../media/jaiqua/jaiqua.skeleton.xml");
	mMeshes.push_back(m);
	m = new Mesh("../media/sinbad/sinbad.material", "../media/sinbad/sinbad.mesh.xml", program, "../media/sinbad/sinbad.skeleton.xml");
	mMeshes.push_back(m);
	//m = new Mesh("../media/role/桃花男_02_身体.material", "../media/role/body.mesh.xml", program, "../media/role/role.Skeleton.xml");
	//mMeshes.push_back(m);
	//m = new Mesh("../media/role/桃花男_01_身体.material", "../media/role/body.mesh.xml", program);
	//mMeshes.push_back(m);
	m = new Mesh("test load tlbb model", program);
	mMeshes.push_back(m);
	mMesh = m;
	mModelIndex = mMeshes.size() - 1;
	
	mv_location = glGetUniformLocation(program, "mv_matrix");
	proj_location = glGetUniformLocation(program, "proj_matrix");
	tex1_location = glGetUniformLocation(program, "tex1");
	tex2_location = glGetUniformLocation(program, "tex2");
	tex3_location = glGetUniformLocation(program, "tex3");

	skeleton_mv_location = glGetUniformLocation(skeletonProgram, "mv_matrix");
	skeleton_proj_location = glGetUniformLocation(skeletonProgram, "proj_matrix");
	skeleton_bonemat_location = glGetUniformLocation(skeletonProgram, "bonemat");

	glUseProgram(program);

	glUniform1i(tex1_location, 0);
	glUniform1i(tex2_location, 1);
	glUniform1i(tex3_location, 2);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void model_loader_app::shutdown(void)
{
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);

	for (int i = 0; i < mMeshes.size(); i++)
	{
		delete mMeshes[i];
	}
	mMeshes.clear();
}

void model_loader_app::onKey(int key, int scancode, int action, int mods)
{
	static bool bPressed = false;
	if(action == GLFW_PRESS && bPressed == false)
	{
		bPressed = true;
		if(key == GLFW_KEY_SPACE)
		{
			mModelIndex = (mModelIndex+1) % mMeshes.size();
			mMesh = mMeshes[mModelIndex];
		}
		else if(key == GLFW_KEY_L)
		{
			mMesh->changeLod();
		}
		else if(key == GLFW_KEY_ENTER)
		{
			mMesh->changePaused();
		}
	}
	if(action == GLFW_RELEASE)
	{
		bPressed = false;
	}

	OpenGLApp::onKey(key, scancode, action, mods);
}

void model_loader_app::render(double t)
{
	static const GLfloat background[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_COLOR, 0, background);
	glClearBufferfv(GL_DEPTH, 0, &one);

	glUseProgram(program);
	glViewport(0, 0, info.windowWidth, info.windowHeight);

	// camera orientation
	POINT cursor;
	GetCursorPos(&cursor); 
	float viewangle_x = float(cursor.x-info.windowWidth/2)/4.0;
	float viewangle_y = float(cursor.y-info.windowWidth/2)/4.0;

	if(mMesh)
	{
		glm::mat4 tmpMat = glm::rotate(glm::mat4(1.0), glm::radians(viewangle_y), glm::vec3(1.0, 0.0, 0.0));
		tmpMat = glm::rotate(tmpMat, glm::radians(viewangle_x), glm::vec3(0.0, 1.0, 0.0));
		tmpMat = glm::mat4(1.0f);
		
		glm::mat4 proj_matrix = glm::perspective(glm::radians(90.0f), (float)info.windowWidth / (float)info.windowHeight, 1.0f, 1000.0f);
		glm::mat4 mv_matrix = tmpMat * glm::translate(glm::mat4(1.0), glm::vec3(-mMesh->getSize()[0], 0.0, mMesh->getViewPos().z)+mMesh->getViewPos());
		//mv_matrix = mv_matrix*glm::rotate<float>(glm::mat4(1.0f), glm::radians(t*30), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 skeleton_matrix = glm::translate(mv_matrix, glm::vec3(mMesh->getSize()[0]*2, 0, 0));
		glUseProgram(skeletonProgram);
		glUniformMatrix4fv(skeleton_proj_location, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(skeleton_mv_location, 1, GL_FALSE, &skeleton_matrix[0][0]);
		mMesh->draw(t, skeletonProgram, true);

		glUseProgram(program);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);
		mMesh->draw(t);
		bool bSkeleton = true;
	}

	glUseProgram(uiProgram);
	glUniform1i(glGetUniformLocation(uiProgram, "bleftpressed"), bLeftPressed ? 1:0);
	glUniform1i(glGetUniformLocation(uiProgram, "brightpressed"), bRightPressed?1:0);
	glDrawArrays(GL_TRIANGLES, 0, 30);
}

void model_loader_app::onMouseButton(int button, int action, int mods)
{
	OpenGLApp::onMouseButton(button, action, mods);
	float x = getCursorX();
	float y = info.windowHeight - getCursorY();

	x = (x/(float)info.windowWidth - 0.5) * 2.0;
	y = (y/(float)info.windowHeight - 0.5) * 2.0;
	if(action == GLFW_PRESS)
	{
		if(x>=0.2 && x <=0.45 && y >= -0.9 && y < -0.75)
		{
			bRightPressed = true;
		}
		if(x<=-0.2 && x >=-0.45 && y >= -0.9 && y < -0.75)
		{
			bLeftPressed = true;
		}
	}
	else
	{
		if(bLeftPressed)
		{
			mMesh->changeAnim(false);
		}
		if(bRightPressed)
			mMesh->changeAnim();
		bLeftPressed = false;
		bRightPressed = false;
	}
}