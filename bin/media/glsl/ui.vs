#version 420 core

uniform vec4 pos;
uniform bool bleftpressed = false;
uniform bool brightpressed = false;

out VS_OUT
{
	vec4 col;
} vs_out;

void main(void)
{
	vec4 pressedcolor = vec4(0.0, 0.0, 1.0, 1.0);
	vec4 releasedcolor = vec4(0.0, 0.8, 0.2, 1.0);
	vec4 verts[] = {
		vec4(0.2, -0.9, 0.5, 1.0),
		vec4(0.45, -0.9, 0.5, 1.0),
		vec4(0.45, -0.75, 0.5, 1.0),
		vec4(0.2, -0.9, 0.5, 1.0),
		vec4(0.45, -0.75, 0.5, 1.0),
		vec4(0.2, -0.75, 0.5, 1.0),

		vec4(0.25, -0.85, 0.5, 1.0),
		vec4(0.35, -0.85, 0.5, 1.0),
		vec4(0.35, -0.80, 0.5, 1.0),
		vec4(0.25, -0.85, 0.5, 1.0),
		vec4(0.35, -0.80, 0.5, 1.0),
		vec4(0.25, -0.80, 0.5, 1.0),
		vec4(0.35, -0.88, 0.5, 1.0),
		vec4(0.40, -0.82, 0.5, 1.0),
		vec4(0.35, -0.76, 0.5, 1.0),

		vec4(-0.2, -0.9, 0.5, 1.0),
		vec4(-0.45, -0.75, 0.5, 1.0),
		vec4(-0.45, -0.9, 0.5, 1.0),
		vec4(-0.2, -0.9, 0.5, 1.0),
		vec4(-0.2, -0.75, 0.5, 1.0),
		vec4(-0.45, -0.75, 0.5, 1.0),

		vec4(-0.25, -0.85, 0.5, 1.0),
		vec4(-0.35, -0.80, 0.5, 1.0),
		vec4(-0.35, -0.85, 0.5, 1.0),
		vec4(-0.25, -0.85, 0.5, 1.0),
		vec4(-0.25, -0.80, 0.5, 1.0),
		vec4(-0.35, -0.80, 0.5, 1.0),

		vec4(-0.35, -0.88, 0.5, 1.0),
		vec4(-0.35, -0.76, 0.5, 1.0),
		vec4(-0.40, -0.82, 0.5, 1.0),
	};
   gl_Position = verts[gl_VertexID];
   vec4 outcol = releasedcolor;
	if(bleftpressed)
		outcol = pressedcolor;

   if(gl_VertexID < 6)
   {
		if(brightpressed)
			outcol = pressedcolor;
	   vs_out.col = outcol;
   }
   else if(gl_VertexID < 15)
   {
	   vs_out.col = vec4(1.0, 0.0, 0.0, 1.0);
   }
   else if(gl_VertexID < 21)
   {
	   if(bleftpressed)
		   outcol = pressedcolor;
	   else
		   outcol = releasedcolor;
	   vs_out.col = outcol;
   }
   else
   {
	   vs_out.col = vec4(1.0, 0.0, 0.0, 1.0);
   }
}
