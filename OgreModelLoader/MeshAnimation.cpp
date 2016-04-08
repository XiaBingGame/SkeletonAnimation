#include "MeshAnimation.h"


MeshAnimation::TKey& MeshAnimation::GetInterpolatedKey(TTrack &t,int frame,float weight,bool normalize)
{
	TKey &k0=t.keys[(frame  ) % t.keys.size() ];
	TKey &k1=t.keys[(frame+1) % t.keys.size() ];

	static TKey k;float weight1=1.0-weight;
	for(int i = 0; i < 3; i++)
		k.pos[i]=k0.pos[i]*weight1+k1.pos[i]*weight;
	for(int i = 0; i < 4; i++)
		k.rot[i]=k0.rot[i]*weight1+k1.rot[i]*weight;

	if(normalize)
	{
		glm::vec3 axis(k.rot[1],k.rot[2],k.rot[3]);
		axis = glm::normalize(axis);
		k.rot[1]=axis.x;k.rot[2]=axis.y;k.rot[3]=axis.z;
	}
	return k;
}

void MeshAnimation::EvalSubtree(int id,TAnimation &ani,int frame, float weight=0)
{
	TBone &b=bones[id];
	glm::mat4 a,m,MINv;
	m = glm::mat4(1.0f);

	// bind pose : default
	glm::vec3 pos(b.pos[0],b.pos[1],b.pos[2]);
	// m.set(b.rot[0],b.rot[1],b.rot[2],b.rot[3]);	
	m = glm::rotate(glm::mat4(1.0), b.rot[0], glm::normalize(glm::vec3(b.rot[1], b.rot[2], b.rot[3])));

	if(ani.tracks[id].keys.size()>frame) // add animated pose if track available
		if(frame>=0)
		{	
			TKey &k=GetInterpolatedKey(ani.tracks[id],frame,weight);
			// a.set(k.rot[0],k.rot[1],k.rot[2],k.rot[3]);
			a = glm::rotate(glm::mat4(1.0), k.rot[0], glm::vec3(k.rot[1], k.rot[2], k.rot[3]));
			pos=pos+glm::vec3(k.pos[0],k.pos[1],k.pos[2]);
			m=glm::transpose(glm::transpose(a)*glm::transpose(m));
		}
		// m.set_translation(pos);
		// m = glm::translate(m, pos);
		m = glm::translate(glm::mat4(1.0), pos) * m;

		// store bone matrix
		//m = glm::mat4(1.0f);
		if(b.parent>=0) b.matrix=bones[b.parent].matrix*m; else b.matrix=m; 

		// progress through tree
		for(int i = 0; i <b.childs.size(); i++)
			EvalSubtree(b.childs[i],ani,frame,weight);
}

void MeshAnimation::SetPose(int animation_index,double time)
{
	if(animation_index>=animations.size()) 
	{
		exit(0);
		//("animation index %d out of range",animation_index);
	}

	TAnimation &ani=animations[animation_index];
	double time01=time/double(ani.timeLength);
	time01=time01-floor(time01);
	float frame=(ani.frameCount-2)*time01+1;

	for (int i = 0; i < bones.size(); i++) bones[i].matrix = glm::mat4(1.0);
	for (int i = 0; i < bones.size(); i++) if (bones[i].parent==-1) EvalSubtree(i,ani,int(frame),(frame-floor(frame)));
}

void MeshAnimation::SetBindPose()
{
	TAnimation &ani=animations[0];	
	for(int i = 0;i < bones.size();i++) bones[i].matrix = glm::mat4(1.0);
	for(int i = 0;i < bones.size();i++) if (bones[i].parent==-1) EvalSubtree(i,ani,-1,0);
	for(int i = 0;i < bones.size();i++) bones[i].invbindmatrix=bones[i].matrix;
	for(int i = 0;i < bones.size();i++) bones[i].invbindmatrix = glm::inverse(bones[i].invbindmatrix);
}

int MeshAnimation::GetAnimationIndexOf ( char* name )
{
	for(int i = 0; i < animations.size(); i++) if (strcmp(name, animations[i].name)==0) return i;
	//error_stop("animation %s not found!",name);
	std::cout << "animation %s not found!";
}

int MeshAnimation::GetBoneIndexOf ( char* name )
{
	for(int i = 0; i < bones.size(); i++) if (strcmp(name,bones[i].name)==0) return i;
	// error_stop( "Error! Bone [%s] does not exist!" ,name );
	std::cout <<  "Error! Bone [%s] does not exist!" ;
}
void MeshAnimation::ResampleAnimationTracks(double frames_per_second)
{
	for(int i = 0; i <animations.size(); i++)
		for(int j = 0; j < animations[i].tracks.size(); j++)
			if(animations[i].tracks[j].keys.size()>0)
			{
				TTrack dst; 
				TTrack &src=animations[i].tracks[j];
				double length=animations[i].timeLength;
				int newframecount=length*frames_per_second;
				int src_frame=0;
				//printf("src[%d]=%d frames\n",j,src.keys.size());

				for(int k = 0; k < newframecount; k++)
				{
					double time=k*length/double(newframecount-1);
					while(src_frame<src.keys.size() && time>src.keys[src_frame].time ) src_frame++;

					int src_frame_1 = glm::clamp<int>( src_frame-1 ,0,src.keys.size()-1);
					int src_frame_2 = glm::clamp<int>( src_frame   ,0,src.keys.size()-1);

					float t1=src.keys[src_frame_1].time;
					float t2=src.keys[src_frame_2].time;
					float w= (time-t1)/(t2-t1);

					TKey key=GetInterpolatedKey(src,src_frame_1,w,true);
					dst.keys.push_back(key);
				}
				animations[i].tracks[j]=dst;
				animations[i].frameCount=newframecount;
				//printf("dst[%d]=%d frames\n\n",j,dst.keys.size());
			}
}
void MeshAnimation::LoadSkeletonXML (const char* ogreXMLfileName)
{
	animations.clear();

	TiXmlDocument doc( ogreXMLfileName );
	if ( !doc.LoadFile() ) 
	{
		std::cout <<  "File %s load error %s\n";
		return;
	}


	TiXmlNode* node = 0;
	TiXmlElement* skeletonNode = 0;
	TiXmlElement* animationsElement = 0;
	TiXmlElement* animationElement = 0;
	TiXmlElement* bonesElement = 0;
	TiXmlElement* boneElement = 0;
	TiXmlElement* boneHierarchyElement = 0;
	TiXmlElement* boneParentElement = 0;

	node = doc.FirstChild( "skeleton" ); if(!node) 
	{
		std::cout << "node ptr 0";
		return;
	}
	skeletonNode = node->ToElement(); if(!skeletonNode) 
	{
		std::cout << "skeletonNode ptr 0";
		return;
	}

	bonesElement = skeletonNode->FirstChildElement( "bones" );assert( bonesElement );

	for(boneElement = bonesElement->FirstChildElement( "bone" );boneElement;
		boneElement = boneElement->NextSiblingElement( "bone" ) )
	{
		int result=0;
		const char* cBoneName;
		double dBoneID = 0,dPosX = 0,dPosY = 0,dPosZ = 0;
		double dAxisAngle = 0,dAxisX = 0,dAxisY = 0,dAxisZ = 0;

		result+= boneElement->QueryDoubleAttribute( "id", &dBoneID );
		cBoneName = boneElement->Attribute( "name" );

		TiXmlElement *positionElement = 0,*rotateElement = 0,*axisElement = 0;
		positionElement = boneElement->FirstChildElement( "position" );assert( positionElement );
		rotateElement = boneElement->FirstChildElement( "rotation" );assert( rotateElement );
		axisElement = rotateElement->FirstChildElement( "axis" );assert( axisElement );
		result+= positionElement->QueryDoubleAttribute( "x", &dPosX );
		result+= positionElement->QueryDoubleAttribute( "y", &dPosY );
		result+= positionElement->QueryDoubleAttribute( "z", &dPosZ );
		result+= rotateElement->QueryDoubleAttribute( "angle", &dAxisAngle );
		result+= axisElement->QueryDoubleAttribute( "x", &dAxisX );
		result+= axisElement->QueryDoubleAttribute( "y", &dAxisY );
		result+= axisElement->QueryDoubleAttribute( "z", &dAxisZ );
		assert( result == 0 );

		TBone bone;
		sprintf( bone.name ,"%s", cBoneName);
		bone.nameLength = strnlen(bone.name,NAME_LEN);
		bone.rot[0]    = dAxisAngle;
		bone.rot[1]    = dAxisX;
		bone.rot[2]    = dAxisY;
		bone.rot[3]    = dAxisZ;
		bone.pos[0]    = dPosX;
		bone.pos[1]    = dPosY;
		bone.pos[2]    = dPosZ;
		bone.parent			= -1;		
		bones.push_back(bone);		
		//printf ( "Bone %03d %s\n" , (int)dBoneID , cBoneName ) ;
	}	
	//printf ("\nBone Hierarchy\n" );

	boneHierarchyElement = skeletonNode->FirstChildElement( "bonehierarchy" );
	assert( boneHierarchyElement );

	for(boneParentElement = boneHierarchyElement->FirstChildElement( "boneparent" );boneParentElement;
		boneParentElement = boneParentElement->NextSiblingElement( "boneparent" ) )
	{
		const char* cBoneName;
		const char* cBoneParentName;
		cBoneName = boneParentElement->Attribute( "bone" );
		cBoneParentName = boneParentElement->Attribute( "parent" );
		int cBoneIndex		= GetBoneIndexOf((char*)cBoneName);
		int cBoneParentIndex	= GetBoneIndexOf((char*)cBoneParentName);		//printf ( "Bone[%s,%d] -> Parent[%s,%d]\n" , cBoneName , cBoneIndex, cBoneParentName , cBoneParentIndex ) ;
		bones[ cBoneIndex ].parent = cBoneParentIndex;
	}

	// build hierarchy in
	for (int i = 0; i < bones.size(); i++)
	{
		int p=bones[i].parent;
		if(p>=0) bones[p].childs.push_back(i);	
	}

	// build hierarchy out
	animationsElement = skeletonNode->FirstChildElement( "animations" );assert( animationsElement );

	for( animationElement = animationsElement->FirstChildElement( "animation" ); animationElement;
		animationElement = animationElement->NextSiblingElement( "animation" ) )
	{
		int result;
		double dAnimationLength=0;
		const char* cAnimationName;
		cAnimationName = animationElement->Attribute( "name" );
		result = animationElement->QueryDoubleAttribute( "length", &dAnimationLength );
		printf ( "Animation[%d] Name:[%s] , Length: %3.03f sec \n" ,animations.size(), cAnimationName , (float) dAnimationLength ) ;

		// --- Fill Memory Begin ---//

		TAnimation animation;
		animation.frameCount=0;
		sprintf( animation.name ,"%s", cAnimationName);
		animation.nameLength = strnlen(animation.name,NAME_LEN);
		animation.timeLength = dAnimationLength;
		std::vector<TTrack> &tracks = animation.tracks;
		tracks.resize(bones.size());

		// --- Fill Memory End ---//

		TiXmlElement *tracksElement = 0,*trackElement = 0;
		tracksElement = animationElement->FirstChildElement( "tracks" );
		assert( tracksElement );

		for( trackElement = tracksElement->FirstChildElement( "track" ); trackElement;
			trackElement = trackElement->NextSiblingElement( "track" ) )
		{
			const char* cBoneName;
			cBoneName = trackElement->Attribute( "bone" );
			//printf ( "\n   Bone Name:[%s]\n\n" , cBoneName ) ;

			// --- Fill Memory Begin ---//
			int trackIndex = GetBoneIndexOf((char*)cBoneName);
			TTrack &track = tracks[ trackIndex ];
			//sprintf( track.name ,"%s", cBoneName);
			// --- Fill Memory End ---//

			TiXmlElement* keyframesElement = 0, *keyframeElement = 0;
			keyframesElement = trackElement->FirstChildElement( "keyframes" );
			assert( keyframesElement );

			for( keyframeElement = keyframesElement->FirstChildElement( "keyframe" ); keyframeElement;
				keyframeElement = keyframeElement->NextSiblingElement( "keyframe" ) )
			{
				int result;
				double dKeyTime;
				result = keyframeElement->QueryDoubleAttribute( "time", &dKeyTime );
				//printf ( "     Keyframe Time: %3.3f  < Anination: %s , Bone: %s >\n" , 
				//	(float)dKeyTime , cAnimationName, cBoneName ) ;

				double dTranslateX = 0, dTranslateY = 0,dTranslateZ = 0;
				double dAxisAngle = 0,dAxisX = 0,dAxisY = 0,dAxisZ = 0;
				TiXmlElement *translateElement = 0,*rotateElement = 0,*axisElement = 0;
				translateElement = keyframeElement->FirstChildElement( "translate" );assert( translateElement );
				rotateElement = keyframeElement->FirstChildElement( "rotate" );assert( rotateElement );
				axisElement = rotateElement->FirstChildElement( "axis" );assert( axisElement );
				result = translateElement->QueryDoubleAttribute( "x", &dTranslateX );
				result+= translateElement->QueryDoubleAttribute( "y", &dTranslateY );
				result+= translateElement->QueryDoubleAttribute( "z", &dTranslateZ );
				result+= rotateElement->QueryDoubleAttribute( "angle", &dAxisAngle );
				result+= axisElement->QueryDoubleAttribute( "x", &dAxisX );
				result+= axisElement->QueryDoubleAttribute( "y", &dAxisY );
				result+= axisElement->QueryDoubleAttribute( "z", &dAxisZ );
				assert( result == 0 );

				// --- Fill Memory Begin ---//
				TKey key;
				key.time	  = dKeyTime;
				key.rot[0]    = dAxisAngle;
				key.rot[1]    = dAxisX;
				key.rot[2]    = dAxisY;
				key.rot[3]    = dAxisZ;
				key.pos[0] = dTranslateX;
				key.pos[1] = dTranslateY;
				key.pos[2] = dTranslateZ;
				track.keys.push_back(key);
			}
			animation.frameCount=glm::max<int>(animation.frameCount,track.keys.size());
		}
		animations.push_back(animation);
	}
	ResampleAnimationTracks(20);// 20 keyframes per second
	SetBindPose();				// store bind pose
	int s = bones.size();
	char buf[100];
	for(int i = 0; i < 100; i++)
		buf[i] = 0;
	/*
	for(int i = 0; i < s; i ++)
	{
		glm::mat4 m = bones[i].invbindmatrix*bones[i].matrix;

		sprintf(buf, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
		std::cout << buf;
	}
	*/
	printf ( "Skeleton: %d bones\n\n" , bones.size() ) ;

	if(bones.size()>=100)
	{
		std::cout << "too many bones in skeleton (%d>100)\n";
		return;
	}
}
