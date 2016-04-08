#include <glm/glm.hpp>
#include "mesh.h"
#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include "OpenGLApp.h"
#include <glm/gtc/type_ptr.hpp>
#include "texture.h"
#include "stdio.h"
#include <limits>
#include "TextureHelper.h"

Mesh::Mesh()
{
	init();
}

Mesh::Mesh(char* ogrematerialfilename, char* ogremeshfilenamexml, unsigned int program, char* ogreskeletonfilenamexml)
	: mProgram(program)
	, mbSkin(false)
	, mCurrentLod(0)
{
	init();
	mProgram = program;
	mSkinLoc = glGetUniformLocation(mProgram, "use_skin");
	mBoneMatLoc = glGetUniformLocation(mProgram, "bone_mat");
	
	if(ogreskeletonfilenamexml)
	{
		mAnimation.LoadSkeletonXML(ogreskeletonfilenamexml);
		if(mAnimation.animations.size() > 0)
		{
			mbSkin = true;
			mAnimIndex = mAnimation.animations.size()-1;
			changeAnim();
		}	
	}
	
	LoadOgreXML(ogrematerialfilename, ogremeshfilenamexml);
	buildGeometry();
}

Mesh::Mesh(char* tlbbobjectfilename, unsigned int program)
	: mProgram(program)
	, mbSkin(false)
	, mCurrentLod(0)
{
	mProgram = program;
	mSkinLoc = glGetUniformLocation(mProgram, "use_skin");
	mBoneMatLoc = glGetUniformLocation(mProgram, "bone_mat");

	init();

	LoadObjXML(tlbbobjectfilename);
	buildGeometry();
}

void Mesh::LoadObjXML(char* name_object)
{
	std::string ogreskeletonfilenamexml = "../media/role/role.Skeleton.xml";
	mAnimation.LoadSkeletonXML(ogreskeletonfilenamexml.c_str());
	if(mAnimation.animations.size() > 0)
	{
		mbSkin = true;
		mAnimIndex = mAnimation.animations.size()-1;
		changeAnim();
	}

	std::vector<std::pair<std::string, std::string> > string_list;
	string_list.push_back(std::make_pair("../media/role/桃花男_02_头发.material", "../media/role/桃花男_02_头发.mesh.xml"));
	string_list.push_back(std::make_pair("../media/role/男主角_鸡蛋脸_01.material", "../media/role/男主角_鸡蛋脸_01.mesh.xml"));
	string_list.push_back(std::make_pair("../media/role/桃花男_02_双手.material", "../media/role/桃花男_02_双手.mesh.xml"));
	string_list.push_back(std::make_pair("../media/role/桃花男_02_身体.material", "../media/role/桃花男_02_身体.mesh.xml"));
	string_list.push_back(std::make_pair("../media/role/桃花男_02_双脚.material", "../media/role/桃花男_02_双脚.mesh.xml"));
	
	std::vector<ObjectSubEntity > objContainer;

	std::vector<std::pair<std::string, std::string> >::const_iterator it = string_list.begin();
	
	// 要加载 material 才行
	while(it != string_list.end())
	{
		LoadOgreXML((*it).first.c_str(), (*it).second.c_str());
		//LoadMesh((*it).c_str());
		ObjectSubEntity objSubEntity;
		objSubEntity.lod_meshes = lod_meshes;
		objSubEntity.materials = materials;
		objSubEntity.vertices = vertices;
		objContainer.push_back(objSubEntity);
		lod_meshes.clear();
		vertices.clear();
		materials.clear();
		it++;
	}

	std::vector<ObjectSubEntity >::const_iterator lodIt = objContainer.begin();
	int cur_vertex_start = 0;
	int cur_material_start = 0;
	lod_meshes.clear();
	vertices.clear();
	materials.clear();

	while(lodIt != objContainer.end())
	{
		ObjectSubEntity obj = *lodIt;
		vertices.insert(vertices.end(), obj.vertices.begin(), obj.vertices.end());
		materials.insert(materials.end(), obj.materials.begin(), obj.materials.end());

		std::vector<LOD>::iterator sublodit = obj.lod_meshes.begin();
		for(int i = 0; i < obj.lod_meshes.size(); i++)
		{
			while(lod_meshes.size() <= i)
			{
				LOD l;
				lod_meshes.push_back(l);
			}
			std::vector<Triangle>::iterator sublodtriit = obj.lod_meshes[i].triangles.begin();
			while(sublodtriit != obj.lod_meshes[i].triangles.end())
			{
				(*sublodtriit).index[0] += cur_vertex_start;
				(*sublodtriit).index[1] += cur_vertex_start;
				(*sublodtriit).index[2] += cur_vertex_start;
				(*sublodtriit).material += cur_material_start;
				sublodtriit++;
			}
			lod_meshes[i].triangles.insert(lod_meshes[i].triangles.end(), obj.lod_meshes[i].triangles.begin(), obj.lod_meshes[i].triangles.end());
		}
		//lod_meshes.insert(lod_meshes.end(), obj.lod_meshes.begin(), obj.lod_meshes.end());
		cur_vertex_start += obj.vertices.size();
		cur_material_start += obj.materials.size();
		lodIt++;
	}
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &mBoneBuffer);
	glDeleteBuffers(1, &mBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVAO);
}

void Mesh::init()
{
	mAnimTime = 0;
	mbSkin = false;
	mAnimIndex = 0;
	mAnimLength = 0.0;
	mMinPos = glm::vec3(INT_MAX);
	mMaxPos = glm::vec3(INT_MIN);
	mViewPos = glm::vec3(0.0f);
	mbPause = false;

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
	glGenBuffers(1, &mBuffer);
	glGenBuffers(1, &mIndexBuffer);
	glGenBuffers(1, &mSkeletonBuffer);
	glGenBuffers(1, &mSkeletonIndexBuffer);
	glGenVertexArrays(1, &mCubeVAO);
}

void Mesh::changeAnim()
{
	if(mAnimation.animations.size() > 0)
	{
		mAnimIndex = (mAnimIndex + 1) % mAnimation.animations.size();
		mAnimation.SetPose(mAnimIndex, 0.0);
		mAnimTime = 0.0;
		mAnimLength = mAnimation.animations[mAnimIndex].timeLength;
	}
}

void Mesh::buildGeometry()
{
	glBindVertexArray(mCubeVAO);
	glm::vec3 cubeVertices[8] = {glm::vec3(-1.0, -1.0, 1.0), glm::vec3(1.0, -1.0, 1.0),
								glm::vec3(1.0, 1.0, 1.0), glm::vec3(-1.0, 1.0, 1.0),
								glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0),
								glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0),};
	unsigned short cubeIndex[36] = {
		0, 1, 2, 0, 2, 3,
		1, 5, 6, 1, 6, 2,
		5, 4, 7, 5, 7, 6,
		4, 0, 3, 4, 3, 7,
		3, 2, 6, 3, 6, 7,
		4, 5, 1, 4, 1, 0
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, mSkeletonBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*8, glm::value_ptr(cubeVertices[0]), GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSkeletonIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 36, &cubeIndex[0], GL_STATIC_DRAW);

	glBindVertexArray(mVAO);	
	VertexInfo* local_array = new VertexInfo[vertices.size()];
	for(int i = 0; i < vertices.size(); i++)
	{
		local_array[i].pos = vertices[i].position;
		local_array[i].normal = vertices[i].normal;
		local_array[i].texcoord = glm::vec2(vertices[i].texcoord.x, 1.0-vertices[i].texcoord.y);
		local_array[i].weights = glm::vec3(vertices[i].weight[0], vertices[i].weight[1], vertices[i].weight[2]);

		updateMin(local_array[i].pos);
		updateMax(local_array[i].pos);
	}
	glm::vec3 center = -(mMinPos + mMaxPos)/2.0f;
	
	/*
	for(int i = 0; i < vertices.size(); i++)
	{
		local_array[i].pos += center;
	}
	*/
	
	mViewPos = -(mMinPos + mMaxPos)/2.0f;
	mViewPos.z = -glm::max<float>((mMaxPos.y - mMinPos.y), ((mMaxPos.z-mMinPos.z)/2.0));

	glm::vec3 VertexInfo::* ptr = &VertexInfo::normal;
	int offset = reinterpret_cast<int>(*(void**)(&ptr));
	std::cout << offset << std::endl;
	std::cout << sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2);
	
	int pervertex_size = sizeof(VertexInfo);

	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * pervertex_size, glm::value_ptr(local_array[0].pos), GL_STATIC_DRAW);
	
	delete [] local_array;
	
	glVertexAttribPointer(0, 3, GL_FLOAT, false, pervertex_size, 0);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, false, pervertex_size, (void*)(offsetof(struct VertexInfo,normal)));

	glVertexAttribPointer(1, 3, GL_FLOAT, false, pervertex_size, (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, pervertex_size, (void*)(offsetof(struct VertexInfo, texcoord)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, false, pervertex_size, (void*)(offsetof(struct VertexInfo, weights)));
	glEnableVertexAttribArray(3);
	
	
	glm::ivec3* id_array = new glm::ivec3[vertices.size()];
	for(int i = 0; i < vertices.size(); i++)
	{
		id_array[i] = glm::ivec3(vertices[i].boneindex[0], vertices[i].boneindex[1], vertices[i].boneindex[2]);
	}

	glGenBuffers(1, &mBoneBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBoneBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(int) * 3, glm::value_ptr(id_array[0]), GL_STATIC_DRAW);
	glVertexAttribIPointer(4, 3, GL_INT, 0, 0);
	glEnableVertexAttribArray(4);
	delete [] id_array;

	unsigned int total_tri_number = 0;
	unsigned int start = 0;
	for (int i = 0; i < lod_meshes.size(); i++)
	{
		lod_meshes[i].start = start;
		start += lod_meshes[i].triangles.size()*3;
		total_tri_number += lod_meshes[i].triangles.size();
	}

	unsigned int* indexdata = new unsigned int[total_tri_number*3];
	unsigned int index = 0;
	for (int i = 0; i < lod_meshes.size(); i++)
	{	
		for(int j = 0; j < lod_meshes[i].triangles.size(); j++)
		{
			indexdata[index++] = lod_meshes[i].triangles[j].index[0];
			indexdata[index++] = lod_meshes[i].triangles[j].index[1];
			indexdata[index++] = lod_meshes[i].triangles[j].index[2];
		}
	}

	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * total_tri_number, &indexdata[0], GL_STATIC_DRAW);
	delete [] indexdata;

	SubEntityInfo sei;
	for(int i = 0; i < lod_meshes.size(); i++)
	{
		for(int j = 0; j < lod_meshes[i].triangles.size(); j++)
		{
			if(j == 0)
			{
				sei.material = lod_meshes[i].triangles[j].material;
				sei.start = 0;
			}
			else
			{
				if(sei.material != lod_meshes[i].triangles[j].material)
				{
					sei.count = j*3 - sei.start;
					lod_meshes[i].m_subEntity.push_back(sei);
					sei.material = lod_meshes[i].triangles[j].material;
					sei.start = j*3;
				}
			}
		}

		sei.count = lod_meshes[i].triangles.size()*3 - sei.start;
		lod_meshes[i].m_subEntity.push_back(sei);
	}
	
}

void Mesh::draw(double t, unsigned int program, bool bSkeleton)
{
	//return;
	static double current_t = t;
	static glm::mat4 bonemat[100];

	double elapsedtime = t - current_t;
	current_t = t;

	glBindVertexArray(mVAO);

	glUniform1i(mSkinLoc, mbSkin ? 1 : 0);

	//mbSkin = false;
	
	if(mbSkin)
	{	
		if(!mbPause)
		{
			mAnimTime += elapsedtime;
			if(mAnimTime > mAnimLength)
			{
				mAnimTime -= mAnimLength;
			}
			mAnimation.SetPose(mAnimIndex, mAnimTime);
		}
		
		if(!bSkeleton)
		{
			for (int i = 0; i < mAnimation.bones.size(); i++)
			{
				bonemat[i] =  mAnimation.bones[i].matrix * mAnimation.bones[i].invbindmatrix;
			}
			glUniformMatrix4fv(mBoneMatLoc, mAnimation.bones.size(), false, glm::value_ptr(bonemat[0]));
		}
	}
	
	if(bSkeleton)
	{
		glBindVertexArray(mCubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mSkeletonBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSkeletonIndexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		if(program != 0)
		{
			GLuint boneMatLoc = glGetUniformLocation(program, "bonemat");
			for (int i = 0; i < mAnimation.bones.size(); i++)
			{
				glUniformMatrix4fv(boneMatLoc, 1, false, glm::value_ptr(mAnimation.bones[i].matrix));
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
			}
			
		}	
		//glDrawArrays(GL_POINTS, 0, 1);
		return;
	}
	
	glBindVertexArray(mVAO);
	glBindBuffer(GL_VERTEX_ARRAY, mBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

	if(lod_meshes.size() > 0)
	{
		for (int i = 0; i < lod_meshes[mCurrentLod].m_subEntity.size(); i++)
		{
			Material m = materials[lod_meshes[mCurrentLod].m_subEntity[i].material];
			if(m.diffuse_map.filename.length() > 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m.diffuse_map.gl_handle);
				glEnable(GL_TEXTURE_2D);
			}
			if(m.ambient_map.filename.length() > 0)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m.ambient_map.gl_handle);
				glEnable(GL_TEXTURE_2D);
			}
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			//glPointSize(2.0);
			glDrawElements(GL_TRIANGLES, lod_meshes[mCurrentLod].m_subEntity[i].count, GL_UNSIGNED_INT, (void*)((lod_meshes[mCurrentLod].start + lod_meshes[mCurrentLod].m_subEntity[i].start)*sizeof(unsigned int)));
		}
	}
}

void Mesh::changeLod()
{
	if(lod_meshes.size() > 0)
	{
		mCurrentLod = (mCurrentLod + 1) % lod_meshes.size();
	}
}

void Mesh::LoadOgreXML(const char* name_material, const char* name_mesh)
{
	std::fstream file(name_material, std::ios::in);
	std::string line;
	char name[1000];
	if(file.is_open())
	{
		Material mat,empty_mat;

		while(std::getline(file, line))
		{
			if(sscanf(line.c_str(),"material %s",name )==1){materials.push_back(mat);mat=empty_mat;mat.name=name;}
			sscanf(line.c_str()," ambient %f %f %f",&mat.ambient.x,&mat.ambient.y,&mat.ambient.z );
			sscanf(line.c_str()," diffuse %f %f %f",&mat.diffuse.x,&mat.diffuse.y,&mat.diffuse.z );
			sscanf(line.c_str()," specular %f %f %f",&mat.specular.x,&mat.specular.y,&mat.specular.z );
			sscanf(line.c_str()," emissive %f %f %f",&mat.emissive.x,&mat.emissive.y,&mat.emissive.z );
			if(sscanf(line.c_str()," texture %s",name )==1)
			{
				Texture *texture=&mat.diffuse_map; bool bump=0;
				if(strcmp(name,"_unit")==0)continue;
				if(strcmp(name,"_bump")==0)
				{
					bump=1;
					sscanf(line.c_str()," texture_bump %s",name );
					texture=&mat.bump_map;
				}
				if(strcmp(name,"_ambient")==0)
				{
					sscanf(line.c_str()," texture_ambient %s",name );
					texture=&mat.ambient_map;
				}
				std::string prename;
				std::string tempname(name_material);
				std::size_t pos = tempname.rfind("/");
				if(pos == std::string::npos)
					pos = tempname.rfind("\\");

				if(pos != std::string::npos)
					prename = tempname.substr(0, pos+1);
				texture->filename = prename + std::string(name);
				// texture->gl_handle = loadBMP_custom(texture->filename.c_str());
				texture->gl_handle = loadImage(texture->filename.c_str());
			}
			if(sscanf(line.c_str()," env_map %s",name )==1)
			{
				// To do
			}
		}
		materials.push_back(mat);
		//PrintMaterials();
	}
	else
	{
		std::cerr << "Can't read material" << name_material << std::endl;
		return;
	}

	file.close();
	LoadMesh(name_mesh);
}

void Mesh::LoadMesh(const char* name_mesh)
{
	std::fstream file(name_mesh, std::ios::in);
	char name[1000];
	std::string line;

	Triangle tri;
	glm::vec3 v;
	int a1,a2;
	float w;
	tri.material=0;
	int offset=0;
	

	std::vector<Triangle> triangles;
	std::vector<int>  lodmeshindex_offset;	// for LOD
	std::vector<int>  lodmeshindex_material;// for LOD		

	int normal_id=0,texcoord_id=0,tangent_id=0;
	char buf[50];

	if (!file.is_open())
	{
		sprintf (buf, "File %s not found!\n" ,name_mesh );
		std::cout << buf;
		return;
	}

	while(std::getline(file, line))
	{
		if(sscanf(line.c_str()," <face v1=\"%d\" v2=\"%d\" v3=\"%d\" />",&tri.index[0],&tri.index[1],&tri.index[2] )==3)
		{
			for (int i = 0; i < 3; i++)
			{
				tri.index[i]+=offset;
			}
			triangles.push_back(tri);
		}
		if(sscanf(line.c_str()," <position x=\"%f\" y=\"%f\" z=\"%f\" />",&v.x,&v.y,&v.z )==3)
		{
			Vertex a;
			a.position=v;
			vertices.push_back(a);
		}
		if(sscanf(line.c_str()," <normal x=\"%f\" y=\"%f\" z=\"%f\" />",&v.x,&v.y,&v.z )==3)
			vertices[normal_id++].normal=v;

		if(sscanf(line.c_str()," <texcoord u=\"%f\" v=\"%f\" />",&v.x,&v.y )==2)  
			vertices[texcoord_id++].texcoord=v;

		if(sscanf(line.c_str()," <tangent x=\"%f\" y=\"%f\" z=\"%f\" w=\"%f\"",&v.x,&v.y,&v.z,&w )==4)
			vertices[tangent_id++].tangent=glm::vec4(v.x,v.y,v.z,w);

		if(sscanf(line.c_str()," <vertexboneassignment vertexindex=\"%d\" boneindex=\"%d\" weight=\"%f\" />",&a1,&a2,&w )==3)
		{
			a1+=offset;
			if(a1>=vertices.size())
			{
				std::cerr << "Error!" << std::endl;
				return;
			}

			int i=vertices[a1].count;
			if(i<3)
			{
				vertices[a1].weight[i]=w;
				vertices[a1].boneindex[i]=a2;
				vertices[a1].count++;
			}
		}
		if(sscanf(line.c_str(),"        <submesh material=\"%s\" usesharedvertices",name )==1)
		{
			//if(lodmeshindex_offset.size() != 0)
			// 处理 TLBB 数据的时候, 其并不将顶点数据放在 submesh 内, 因此偏移出错
			if(lodmeshindex_offset.size() != 0 || triangles.size() > 0)
				offset=vertices.size();

			name[strlen(name)-1]=0;
			tri.material=GetMaterialIndex ( name );
			lodmeshindex_offset.push_back(offset);
			lodmeshindex_material.push_back(tri.material);
		}	
		if(sscanf(line.c_str()," <lodfacelist submeshindex=\"%d\" numfaces=",&a1)==1)
		{
			if(a1<0 || a1>=lodmeshindex_offset.size()) 
			{
				std::cerr << "Error!" << std::endl;
				return;
			}
			offset=lodmeshindex_offset[a1];
			tri.material=lodmeshindex_material[a1];
		}
		if(sscanf(line.c_str()," <lodgenerated value=\"%f\">",&w)==1)
		{
			LOD l;l.triangles=triangles;
			lod_meshes.push_back(l);
			triangles.clear();
		} 
	}

	if(lod_meshes.size() == 0)
	{
		LOD l;
		l.triangles = triangles;
		lod_meshes.push_back(l);
		triangles.clear();
	}
}

int Mesh::GetMaterialIndex (std::string name)
{
	for(int i = 0; i < materials.size(); i++)
		if ( name.compare( materials[i].name ) == 0 ) return i;

	std::cout << "couldnt find material %s" << std::endl;
	return -1;
}

void Mesh::PrintMaterials()
{
	std::cout << std::endl;
	char buf[200];
	for(unsigned int i=0; i<materials.size();i++)
	{	
	
		sprintf(buf, "Material %i : %s\n",i,materials[i].name.c_str());
		std::cout << buf;
		sprintf(buf, "  Ambient RGB %2.2f %2.2f %2.2f\n",	materials[i].ambient.x,materials[i].ambient.y,materials[i].ambient.z);
		std::cout << buf;
		sprintf(buf, "  Specular RGB %2.2f %2.2f %2.2f\n",materials[i].specular.x,materials[i].specular.y,materials[i].specular.z);
		std::cout << buf;
		sprintf(buf, "  Emissive RGB %2.2f %2.2f %2.2f\n",materials[i].emissive.x,materials[i].emissive.y,materials[i].emissive.z);
		std::cout << buf;
		sprintf(buf, "  Diffuse RGB %2.2f %2.2f %2.2f\n",	materials[i].diffuse.x,	materials[i].diffuse.y,	materials[i].diffuse.z);
		std::cout << buf;
		sprintf(buf, "  Diff. Tex : %s [Env %d]\n",materials[i].diffuse_map.filename.c_str(),materials[i].diffuse_map.envmap);
		std::cout << buf;
		//sprintf(buf, "  Amb . Tex : %s \n",materials[i].ambient_map.filename.c_str());
		std::cout << buf;
		//sprintf(buf, "  Bump  Tex : %s \n",materials[i].bump_map.filename.c_str());
		std::cout << buf;
		sprintf(buf, "  Alpha %2.2f\n", materials[i].alpha);
		std::cout << buf;

	}
	std::cout << "\n";
}