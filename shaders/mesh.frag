#version 120

uniform sampler2D u_tex;
uniform float u_opacity;

varying vec2 v_tex_pos;

void main()
{
    vec4 color = texture2D(u_tex, v_tex_pos);
    color[3] = u_opacity * color[3];
    gl_FragColor = color;
}