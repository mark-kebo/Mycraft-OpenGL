#version 420

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout(std140, binding = 1) uniform Projection
{
    mat4x4 projection;
} projection;

layout(std140, binding = 2) uniform View
{
    mat4x4 view;
} view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection.projection * view.view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  