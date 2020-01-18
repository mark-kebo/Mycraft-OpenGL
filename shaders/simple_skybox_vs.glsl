#version 420

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout(std140, binding = 6) uniform Matrices {
    mat4x4 projection;
	mat4x4 view;
} matrices;

void main() {
    TexCoords = aPos;
    vec4 pos = matrices.projection * matrices.view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  