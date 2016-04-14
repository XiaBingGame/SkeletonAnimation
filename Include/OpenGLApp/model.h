#ifndef __MODEL_H__
#define __MODEL_H__

#include "preinclude.h"

// Include GLEW
#include <GL/glew.h>

#define SB6M_FOURCC(a,b,c,d)            ( ((unsigned int)(a) << 0) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24) )
#define SB6M_VERTEX_ATTRIB_FLAG_NORMALIZED      0x00000001
#define SB6M_VERTEX_ATTRIB_FLAG_INTEGER         0x00000002

namespace sb6{

typedef struct SB6M_SUB_OBJECT_DECL_t
{
	unsigned int                first;
	unsigned int                count;
} SB6M_SUB_OBJECT_DECL;

typedef struct SB6M_HEADER_t
{
	union
	{
		unsigned int    magic;
		char            magic_name[4];
	};
	unsigned int        size;
	unsigned int        num_chunks;
	unsigned int        flags;
} SB6M_HEADER;

typedef struct SB6M_CHUNK_HEADER_t
{
	union
	{
		unsigned int    chunk_type;
		char            chunk_name[4];
	};
	unsigned int        size;
} SB6M_CHUNK_HEADER;

typedef struct SB6M_VERTEX_ATTRIB_DECL_t
{
	char                name[64];
	unsigned int        size;
	unsigned int        type;
	unsigned int        stride;
	unsigned int        flags;
	unsigned int        data_offset;
} SB6M_VERTEX_ATTRIB_DECL;


typedef struct SB6M_VERTEX_ATTRIB_CHUNK_t
{
	SB6M_CHUNK_HEADER           header;
	unsigned int                attrib_count;
	SB6M_VERTEX_ATTRIB_DECL     attrib_data[1];
} SB6M_VERTEX_ATTRIB_CHUNK;

typedef struct SB6M_CHUNK_VERTEX_DATA_t
{
	SB6M_CHUNK_HEADER   header;
	unsigned int        data_size;
	unsigned int        data_offset;
	unsigned int        total_vertices;
} SB6M_CHUNK_VERTEX_DATA;

typedef struct SB6M_CHUNK_INDEX_DATA_t
{
	SB6M_CHUNK_HEADER   header;
	unsigned int        index_type;
	unsigned int        index_count;
	unsigned int        index_data_offset;
} SB6M_CHUNK_INDEX_DATA;

typedef struct SB6M_CHUNK_SUB_OBJECT_LIST_t
{
	SB6M_CHUNK_HEADER           header;
	unsigned int                count;
	SB6M_SUB_OBJECT_DECL        sub_object[1];
} SB6M_CHUNK_SUB_OBJECT_LIST;

typedef enum SB6M_CHUNK_TYPE_t
{
	SB6M_CHUNK_TYPE_INDEX_DATA      = SB6M_FOURCC('I','N','D','X'),
	SB6M_CHUNK_TYPE_VERTEX_DATA     = SB6M_FOURCC('V','R','T','X'),
	SB6M_CHUNK_TYPE_VERTEX_ATTRIBS  = SB6M_FOURCC('A','T','R','B'),
	SB6M_CHUNK_TYPE_SUB_OBJECT_LIST = SB6M_FOURCC('O','L','S','T'),
	SB6M_CHUNK_TYPE_COMMENT         = SB6M_FOURCC('C','M','N','T')
} SB6M_CHUNK_TYPE;

class OPENGLAPPDLL_API object
{
public:
	object();
	~object();

	inline void render(unsigned int instance_count = 1,
		unsigned int base_instance = 0)
	{
		render_sub_object(0, instance_count, base_instance);
	}

	void render_sub_object(unsigned int object_index,
		unsigned int instance_count = 1,
		unsigned int base_instance = 0);

	void get_sub_object_info(unsigned int index, GLuint &first, GLuint &count)
	{
		if (index >= num_sub_objects)
		{
			first = 0;
			count = 0;
		}
		else
		{
			first = sub_object[index].first;
			count = sub_object[index].count;
		}
	}

	unsigned int get_sub_object_count() const           { return num_sub_objects; }
	GLuint       get_vao() const                        { return vao; }
	void load(const char * filename);
	void free();

private:
	GLuint                  vertex_buffer;
	GLuint                  index_buffer;
	GLuint                  vao;
	GLuint                  num_indices;
	GLuint                  index_type;

	enum { MAX_SUB_OBJECTS = 256 };

	unsigned int            num_sub_objects;
	SB6M_SUB_OBJECT_DECL    sub_object[MAX_SUB_OBJECTS];
};

}
#endif
