#ifdef GL_ES
precision highp float;
#endif

in vec4 v_position;
in vec4 v_color;
in vec2 v_tex_position;
layout(location = 0) out vec4 fragColor;

uniform sampler2D s_texture;

void main()
{
     fragColor =  texture(s_texture, v_tex_position) * v_color;
}
