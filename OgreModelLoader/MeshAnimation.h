#ifndef __MESH_ANIMATION_H__
#define __MESH_ANIMATION_H__

#include <glm/glm.hpp>
#include "xml/tinyxml.h"
#include <stdio.h>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
//##################################################################//
// Ogre XML-Animation File Reader
//##################################################################//
//#define clamp(a_,b_,c_) glm::min(glm::max(a_,b_),c_)

class MeshAnimation
{
	public:

	enum {  NAME_LEN = 30 };
	
	typedef struct
	{	
		float	time;
		float	rot[4]; // angle,x,y,z
		float	pos[3];
	} TKey;

	typedef struct
	{	
		std::vector<TKey> keys;
	} TTrack;

	typedef struct
	{
		char				nameLength;
		char				name[NAME_LEN];
		float				timeLength;
		std::vector<TTrack>	tracks;
		int					frameCount;
	} TAnimation;
	
	typedef struct
	{	
		char			nameLength;
		char			name[NAME_LEN];
		float			rot[4]; // angle,x,y,z
		float			pos[3];
		int				parent;
		glm::mat4		matrix; // animated result
		glm::mat4		invbindmatrix;  // inverse bindmatrix
		std::vector<int> childs;
	} TBone;

	std::vector<TBone>		bones;		
	std::vector<TAnimation>	animations;
	
	MeshAnimation(char* skeletonfilename)
	{
		LoadSkeletonXML (skeletonfilename);
	}
	MeshAnimation(){};

	void  LoadSkeletonXML (const char* ogreXMLfileName );
	int   GetBoneIndexOf ( char* name );
	int   GetAnimationIndexOf  (char* name);
	void  SetPose(int animation,double time);
	void  SetBindPose();
	void  EvalSubtree(int boneid,TAnimation &ani,int frame,float weight);
	TKey& GetInterpolatedKey(TTrack &t,int frame,float weight,bool normalize=false);
	void  ResampleAnimationTracks(double frames_per_second);
};

#endif
