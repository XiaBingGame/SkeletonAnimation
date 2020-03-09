#version 420 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 weights;
layout (location = 4) in ivec3 ids;
            
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);
uniform mat4 bone_mat[100];
uniform bool use_skin = false;

// Inputs from vertex shader
out VS_OUT
{
    vec3 N;
    vec3 L;
    vec3 V;
	vec2 TexCoord;
	vec3 col;
} vs_out;

void main(void)
{           
// Calculate view-space coordioate
	int testid = 1;
	if(weights[0] > 0.99 && (weights[0] + weights[1] + weights[2]) < 1.01)
	{
		vs_out.col = vec3(1.0);
	}
	else
	{
		vs_out.col = vec3(0.0);
	}
	vs_out.col = vec3(float(ids[0])/17.0 * weights[0], float(ids[1])/17.0 * weights[1], float(ids[2])/17.0 * weights[2]);
	mat4 animMat = mat4(1.0);
	if(use_skin)
	{
		animMat = bone_mat[ids[0]] * weights[0] + bone_mat[ids[1]] * weights[1] + bone_mat[ids[2]] * weights[2];
	}
	
	mat4 mat = mv_matrix * animMat;
	vec4 pos = position;

    vec4 P = mat * pos;
    vs_out.N = mat3(mat) * normal;

    // Calculate light vector
    vs_out.L = light_pos - P.xyz;

    vs_out.V = -P.xyz;
	vs_out.TexCoord = texcoord;
    gl_Position = proj_matrix * P;
}
