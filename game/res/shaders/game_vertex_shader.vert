in vec2 a_position;
in vec4 a_color;
in vec2 a_tex_position;

out vec4 v_position;
out vec4 v_color;
out vec2 v_tex_position;

// move matrix
uniform mat3 u_move_matrix;

void main()
{
    vec3 moved_position =   u_move_matrix * vec3(a_position.x, a_position.y, 1.0);
    v_position = vec4(moved_position.x,moved_position.y, 0.0, 1.0);
    v_tex_position = a_tex_position;
    v_color = a_color;
    gl_Position = v_position;
}
