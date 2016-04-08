#version 420 core

layout (location = 0) in vec4 position;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 bonemat = mat4(1.0);


void main(void)
{           
   gl_Position = proj_matrix * mv_matrix * bonemat * position;
}
