#ifndef __MESH_H__
#define __MESH_H__

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "MeshAnimation.h"

class Mesh
{
public:
	// ---------------------------------------- //
	// Structures
	// ---------------------------------------- //
	struct Triangle
	{
		int index[3];	// vertex    ID
		int material;	// material  ID
	};

	struct SubEntityInfo
	{
		int start;
		int count;
		int material;
	};
	// ---------------------------------------- //
	class Vertex
	{
	public: 

		// general
		glm::vec3 position;
		glm::vec3 texcoord;
		glm::vec3 normal;
		glm::vec4 tangent;

		// skinning information
		float weight[3];
		int boneindex[3],count;

		// initialization
		Vertex()
		{
			count=0;
			weight[0]=weight[1]=weight[2]=0;
			boneindex[0]=boneindex[1]=boneindex[2]=0;
		};
	};
	// ---------------------------------------- //
	class Texture
	{
	public:
		Texture(){envmap=0;gl_handle=-1;}

		std::string filename;
		int gl_handle;
		bool envmap;
	};
	// ---------------------------------------- //
	class Material
	{
	public:

		Material()
		{
			ambient = glm::vec3( 0.6 , 0.3 ,0 );
			diffuse = glm::vec3( 0.3 , 0.3 ,0.3 );
			specular= glm::vec3( 0,0,0 );
			emissive= glm::vec3( 0,0,0 );
			diffuse_map.gl_handle=-1;
			emissive_map.gl_handle=-1;
			ambient_map.gl_handle=-1;
			alpha = 1;
			reflect = 0;
			name = "";
		}
		std::string name;
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 ambient;
		glm::vec3 emissive;
		float alpha,reflect;
		Texture diffuse_map;
		Texture emissive_map;
		Texture ambient_map;
		Texture bump_map;
	};
	
	// ---------------------------------------- //
	struct LOD // level of detail mesh
	{
		std::vector<Triangle>		triangles;
		std::vector<SubEntityInfo> m_subEntity;
		unsigned int start;	// 索引的起始位置
	};

	struct VertexInfo
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texcoord;
		glm::vec3 weights;
	};

	// ---------------------------------------- //
	// Main OBJ Part
	// ---------------------------------------- //
	

	/** 构造函数. */
	Mesh();
	/** 构造函数. */
	~Mesh();

	/** 改变当前动画.. */
	void changeAnim();
	void changeLod();
	void changePaused() { mbPause = !mbPause; }

	/** 构造函数. */
	Mesh(char* ogrematerialfilename, char* ogremeshfilenamexml, unsigned int program, char* ogreskeletonfilenamexml=0);
	Mesh(char* tlbbobjectfilename, unsigned int program);

	void draw(double t, unsigned int program = 0, bool bSkeleton = false);	/// 绘制.
	glm::vec3 getViewPos() { return mViewPos; }
	glm::vec3 getSize() { return mMaxPos - mMinPos; }
private:
	void init();		/// 初始化
	void LoadOgreXML(const char* name_material, const char* name_mesh);	/// 加载 材质文件和mesh文件
	void LoadMesh(const char* name_mesh);
	void LoadObjXML(char* name_object);
	int GetMaterialIndex ( std::string name );		/// 得到材质索引
	void PrintMaterials ();			/// 打印材质
	void buildGeometry();			/// 构建集合体
	void updateMin(glm::vec3 v) 
	{ mMinPos.x = glm::min<float>(mMinPos.x, v.x); mMinPos.y = glm::min<float>(mMinPos.y, v.y); mMinPos.z = glm::min<float>(mMinPos.z, v.z); }
	void updateMax(glm::vec3 v) 
	{ mMaxPos.x = glm::max<float>(mMaxPos.x, v.x); mMaxPos.y = glm::max<float>(mMaxPos.y, v.y); mMaxPos.z = glm::max<float>(mMaxPos.z, v.z); }

	unsigned int mVAO;				/// 本模型的 VAO
	unsigned int mBuffer;			/// 顶点 buffer
	unsigned int mSkeletonBuffer;
	unsigned int mSkeletonIndexBuffer;
	unsigned int mBoneBuffer;		/// 骨骼索引 buffer
	unsigned int mIndexBuffer;		/// 索引 buffer
	unsigned int mProgram;			/// 当前程序对象
	unsigned int mCubeVAO;

	unsigned int mSkinLoc;
	unsigned int mBoneMatLoc;


	int mAnimIndex;			/// 当前动画的索引
	double mAnimTime;		/// 当前动画的播放时间
	bool mbSkin;			/// 是否使用动画
	double mAnimLength;		/// 当前动画的长度
	double mbPause;

	bool					skinning;	// skinning on ?
	std::vector<LOD>		lod_meshes;	// triangles & displaylists for all LODs
	std::vector<Vertex>		vertices;	// vertex list
	std::vector<Material>	materials;	// material list
	int mCurrentLod;

	//GLuint mBoneBuffer;

	struct ObjectSubEntity
	{
		std::vector<LOD> lod_meshes;
		std::vector<Vertex> vertices;
		std::vector<Material> materials;
	};
	
	glm::vec3 mMinPos;
	glm::vec3 mMaxPos;
	glm::vec3 mViewPos;

	MeshAnimation mAnimation;		/// 保存动画对象.
};

#endif