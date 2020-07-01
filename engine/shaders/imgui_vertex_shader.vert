in vec2 Position;
in vec4 Color;
in vec2 UV;

out vec2 Frag_UV;
out vec4 Frag_Color;

uniform mat3 u_move_matrix;

void main()
{
        Frag_UV = UV;
        Frag_Color = Color;
        gl_Position = vec4(u_move_matrix * vec3(Position,1.0), 1.0);
}
