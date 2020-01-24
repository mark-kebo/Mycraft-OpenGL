#version 420

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_tex_uv;
layout(location = 2) in vec3 vertex_normal;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 tex_uv;

struct Matrix {
    mat4x4 mvp_matrix;
    mat4x4 model_matrix;
};

layout(std140, binding = 1) uniform Matrices {
	Matrix array[500];
} matrices;

void main() {
    gl_Position = matrices.array[gl_InstanceID].mvp_matrix * vec4(vertex_position, 1.f);

    vec4 tmp_position_ws = matrices.array[gl_InstanceID].model_matrix * vec4(vertex_position, 1.f);
    position_ws = tmp_position_ws.xyz / tmp_position_ws.w;

    normal_ws = transpose(inverse(mat3x3(matrices.array[gl_InstanceID].model_matrix))) * vertex_normal;

    tex_uv = vertex_tex_uv;
}
