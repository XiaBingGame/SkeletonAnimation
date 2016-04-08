#version 420 core

out vec4 color;

in VS_OUT
{
    vec3 N;
    vec3 L;
    vec3 V;
	vec2 TexCoord;
	vec3 col;
} fs_in;

// Material properties
uniform vec3 diffuse_albedo = vec3(1.0, 1.0, 1.0);
uniform vec3 specular_albedo = vec3(0.2);
uniform float specular_power = 4.0;
uniform vec3 ambient = vec3(0.2, 0.2, 0.2);

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
                                                                  
void main(void)
{
	//if(fs_in.col != vec3(1.0))
	//	discard;

// Normalize the incoming N, L and V vectors
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(fs_in.L);
    vec3 V = normalize(fs_in.V);

    // Calculate R locally
    vec3 R = reflect(-L, N);

    // Compute the diffuse and specular components for each fragment
    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
    vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_albedo;
	
	color = vec4(ambient * texture(tex2, fs_in.TexCoord).xyz + diffuse * texture(tex1, fs_in.TexCoord).xyz + specular, 1.0);
	//color = vec4(fs_in.col, 1.0);
}
