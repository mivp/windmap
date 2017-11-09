#version 120

attribute vec3 a_pos;
attribute vec2 a_tex_pos;

uniform mat4 u_mv;
uniform mat4 u_p;

varying vec2 v_tex_pos;

void main(void)
{
	v_tex_pos = vec2(a_tex_pos[0], 1-a_tex_pos[1]);

	gl_Position = u_p * u_mv * vec4(a_pos, 1);
}
